#!/usr/bin/env python3
#
# Copyright (c) 2018 Yousong Zhou <yszhou4tech@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

import argparse
import calendar
import datetime
import errno
import fcntl
import hashlib
import json
import os
import os.path
import re
import shutil
import ssl
import subprocess
import sys
import time
import urllib.request

TMPDIR = os.environ.get('TMP_DIR') or '/tmp'
TMPDIR_DL = os.path.join(TMPDIR, 'dl')


class PathException(Exception): pass
class DownloadGitHubError(Exception): pass


class Path(object):
    """Context class for preparing and cleaning up directories.

    If ```preclean` is ``False``, ``path`` will NOT be removed on context enter

    If ``path`` ``isdir``, then it will be created on context enter.

    If ``keep`` is True, then ``path`` will NOT be removed on context exit
    """

    def __init__(self, path, isdir=True, preclean=False, keep=False):
        self.path = path
        self.isdir = isdir
        self.preclean = preclean
        self.keep = keep

    def __enter__(self):
        if self.preclean:
            self.rm_all(self.path)
        if self.isdir:
            self.mkdir_all(self.path)
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        if not self.keep:
            self.rm_all(self.path)

    @staticmethod
    def mkdir_all(path):
        """Same as mkdir -p."""
        names = os.path.split(path)
        p = ''
        for name in names:
            p = os.path.join(p, name)
            Path._mkdir(p)

    @staticmethod
    def _rmdir_dir(dir_):
        names = Path._listdir(dir_)
        for name in names:
            p = os.path.join(dir_, name)
            Path.rm_all(p)
        Path._rmdir(dir_)

    @staticmethod
    def _mkdir(path):
        Path._os_func(os.mkdir, path, errno.EEXIST)

    @staticmethod
    def _rmdir(path):
        Path._os_func(os.rmdir, path, errno.ENOENT)

    @staticmethod
    def _remove(path):
        Path._os_func(os.remove, path, errno.ENOENT)

    @staticmethod
    def _listdir(path):
        return Path._os_func(os.listdir, path, errno.ENOENT, default=[])

    @staticmethod
    def _os_func(func, path, errno, default=None):
        """Call func(path) in an idempotent way.

        On exception ``ex``, if the type is OSError and ``ex.errno == errno``,
        return ``default``, otherwise, re-raise
        """
        try:
            return func(path)
        except OSError as e:
            if e.errno == errno:
                return default
            else:
                raise

    @staticmethod
    def rm_all(path):
        """Same as rm -r."""
        if os.path.islink(path):
            Path._remove(path)
        elif os.path.isdir(path):
            Path._rmdir_dir(path)
        else:
            Path._remove(path)

    @staticmethod
    def untar(path, into=None):
        """Extract tarball at ``path`` into subdir ``into``.

        return subdir name if and only if there exists one, otherwise raise PathException
        """
        args = ('tar', '-C', into, '-xzf', path, '--no-same-permissions')
        subprocess.check_call(args, preexec_fn=lambda: os.umask(0o22))
        dirs = os.listdir(into)
        if len(dirs) == 1:
            return dirs[0]
        else:
            raise PathException('untar %s: expecting a single subdir, got %s' % (path, dirs))

    @staticmethod
    def tar(path, subdir, into=None, ts=None):
        """Pack ``path`` into tarball ``into``."""
        # --sort=name requires a recent build of GNU tar
        args = ['tar', '--numeric-owner', '--owner=0', '--group=0', '--sort=name', '--mode=a-s']
        args += ['-C', path, '-cf', into, subdir]
        envs = os.environ.copy()
        if ts is not None:
            args.append('--mtime=@%d' % ts)
        if into.endswith('.xz'):
            envs['XZ_OPT'] = '-7e'
            args.append('-J')
        elif into.endswith('.bz2'):
            args.append('-j')
        elif into.endswith('.gz'):
            args.append('-z')
            envs['GZIP'] = '-n'
        else:
            raise PathException('unknown compression type %s' % into)
        subprocess.check_call(args, env=envs)


class GitHubCommitTsCache(object):
    __cachef = 'github.commit.ts.cache'
    __cachen = 2048

    def __init__(self):
        Path.mkdir_all(TMPDIR_DL)
        self.cachef = os.path.join(TMPDIR_DL, self.__cachef)
        self.cache = {}

    def get(self, k):
        """Get timestamp with key ``k``."""
        fileno = os.open(self.cachef, os.O_RDONLY | os.O_CREAT)
        with os.fdopen(fileno) as fin:
            try:
                fcntl.lockf(fileno, fcntl.LOCK_SH)
                self._cache_init(fin)
                if k in self.cache:
                    ts = self.cache[k][0]
                    return ts
            finally:
                fcntl.lockf(fileno, fcntl.LOCK_UN)
        return None

    def set(self, k, v):
        """Update timestamp with ``k``."""
        fileno = os.open(self.cachef, os.O_RDWR | os.O_CREAT)
        with os.fdopen(fileno, 'w+') as f:
            try:
                fcntl.lockf(fileno, fcntl.LOCK_EX)
                self._cache_init(f)
                self.cache[k] = (v, int(time.time()))
                self._cache_flush(f)
            finally:
                fcntl.lockf(fileno, fcntl.LOCK_UN)

    def _cache_init(self, fin):
        for line in fin:
            k, ts, updated = line.split()
            ts = int(ts)
            updated = int(updated)
            self.cache[k] = (ts, updated)

    def _cache_flush(self, fout):
        cache = sorted(self.cache.items(), key=lambda a: a[1][1])
        cache = cache[:self.__cachen]
        self.cache = {}
        os.ftruncate(fout.fileno(), 0)
        fout.seek(0, os.SEEK_SET)
        for k, ent in cache:
            ts = ent[0]
            updated = ent[1]
            line = '{0} {1} {2}\n'.format(k, ts, updated)
            fout.write(line)


class DownloadGitHubTarball(object):
    """Download and repack archive tarball from GitHub.

    Compared with the method of packing after cloning the whole repo, this
    method is more friendly to users with fragile internet connection.

    However, there are limitations with this method

     - GitHub imposes a 60 reqs/hour limit for unauthenticated API access.
       This affects fetching commit date for reproducible tarballs.  Download
       through the archive link is not affected.

     - GitHub archives do not contain source codes for submodules.

     - GitHub archives seem to respect .gitattributes and ignore paths with
       export-ignore attributes.

    For the first two issues, the method will fail loudly to allow fallback to
    clone-then-pack method.

    As for the 3rd issue, to make sure that this method only produces identical
    tarballs as the fallback method, we require the expected hash value to be
    supplied.  That means the first tarball will need to be prepared by the
    clone-then-pack method
    """

    __repo_url_regex = re.compile(r'^(?:https|git)://github.com/(?P<owner>[^/]+)/(?P<repo>[^/]+)')

    def __init__(self, args):
        self.dl_dir = args.dl_dir
        self.version = args.version
        self.subdir = args.subdir
        self.source = args.source
        self.url = args.url
        self._init_owner_repo()
        self.xhash = args.hash
        self._init_hasher()
        self.commit_ts = None           # lazy load commit timestamp
        self.commit_ts_cache = GitHubCommitTsCache()
        self.name = 'github-tarball'

    def download(self):
        """Download and repack GitHub archive tarball."""
        self._init_commit_ts()
        with Path(TMPDIR_DL, keep=True) as dir_dl:
            # fetch tarball from GitHub
            tarball_path = os.path.join(dir_dl.path, self.subdir + '.tar.gz.dl')
            with Path(tarball_path, isdir=False):
                self._fetch(tarball_path)
                # unpack
                d = os.path.join(dir_dl.path, self.subdir + '.untar')
                with Path(d, preclean=True) as dir_untar:
                    tarball_prefix = Path.untar(tarball_path, into=dir_untar.path)
                    dir0 = os.path.join(dir_untar.path, tarball_prefix)
                    dir1 = os.path.join(dir_untar.path, self.subdir)
                    # submodules check
                    if self._has_submodule(dir0):
                        raise self._error('Fetching submodules is not yet supported')
                    # rename subdir
                    os.rename(dir0, dir1)
                    # repack
                    into=os.path.join(TMPDIR_DL, self.source)
                    Path.tar(dir_untar.path, self.subdir, into=into, ts=self.commit_ts)
                    try:
                        self._hash_check(into)
                    except Exception:
                        Path.rm_all(into)
                        raise
                    # move to target location
                    file1 = os.path.join(self.dl_dir, self.source)
                    if into != file1:
                        shutil.move(into, file1)

    def _has_submodule(self, dir_):
        m = os.path.join(dir_, '.gitmodules')
        try:
            st = os.stat(m)
            return st.st_size > 0
        except OSError as e:
            return e.errno != errno.ENOENT

    def _init_owner_repo(self):
        m = self.__repo_url_regex.search(self.url)
        if m is None:
            raise self._error('Invalid github url: {}'.format(self.url))
        owner = m.group('owner')
        repo = m.group('repo')
        if repo.endswith('.git'):
            repo = repo[:-4]
        self.owner = owner
        self.repo = repo

    def _init_hasher(self):
        xhash = self.xhash
        if len(xhash) == 64:
            self.hasher = hashlib.sha256()
        elif len(xhash) == 32:
            self.hasher = hashlib.md5()
        else:
            raise self._error('Requires sha256sum for verification')
        self.xhash = xhash

    def _hash_check(self, f):
        with open(f, 'rb') as fin:
            while True:
                d = fin.read(4096)
                if not d:
                    break
                self.hasher.update(d)
        xhash = self.hasher.hexdigest()
        if xhash != self.xhash:
            raise self._error('Wrong hash (probably caused by .gitattributes), expecting {}, got {}'.format(self.xhash, xhash))

    def _init_commit_ts(self):
        if self.commit_ts is not None:
            return
        # GitHub provides 2 APIs[1,2] for fetching commit data.  API[1] is more
        # terse while API[2] provides more verbose info such as commit diff
        # etc.  That's the main reason why API[1] is preferred: the response
        # size is predictable.
        #
        # However, API[1] only accepts complete commit sha1sum as the parameter
        # while API[2] is more liberal accepting also partial commit id and
        # tags, etc.
        #
        # [1] Get a single commit, Repositories, https://developer.github.com/v3/repos/commits/#get-a-single-commit
        # [2] Git Commits, Git Data, https://developer.github.com/v3/git/commits/#get-a-commit
        apis = [
            {
                'url': self._make_repo_url_path('git', 'commits', self.version),
                'attr_path': ('committer', 'date'),
            }, {
                'url': self._make_repo_url_path('commits', self.version),
                'attr_path': ('commit', 'committer', 'date'),
            },
        ]
        version_is_sha1sum = len(self.version) == 40
        if not version_is_sha1sum:
            apis.insert(0, apis.pop())
        reasons = ''
        for api in apis:
            url = api['url']
            attr_path = api['attr_path']
            try:
                ct = self.commit_ts_cache.get(url)
                if ct is not None:
                    self.commit_ts = ct
                    return
                ct = self._init_commit_ts_remote_get(url, attr_path)
                self.commit_ts = ct
                self.commit_ts_cache.set(url, ct)
                return
            except Exception as e:
                reasons += '\n' + ("  {}: {}".format(url, e))
        raise self._error('Cannot fetch commit ts:{}'.format(reasons))

    def _init_commit_ts_remote_get(self, url, attrpath):
        resp = self._make_request(url)
        data = resp.read()
        date = json.loads(data)
        for attr in attrpath:
            date = date[attr]
        date = datetime.datetime.strptime(date, '%Y-%m-%dT%H:%M:%SZ')
        date = date.timetuple()
        ct = calendar.timegm(date)
        return ct

    def _fetch(self, path):
        """Fetch tarball of the specified version ref."""
        ref = self.version
        url = self._make_repo_url_path('tarball', ref)
        resp = self._make_request(url)
        with open(path, 'wb') as fout:
            while True:
                d = resp.read(4096)
                if not d:
                    break
                fout.write(d)

    def _make_repo_url_path(self, *args):
        url = '/repos/{0}/{1}'.format(self.owner, self.repo)
        if args:
            url += '/' + '/'.join(args)
        return url

    def _make_request(self, path):
        """Request GitHub API endpoint on ``path``."""
        url = 'https://api.github.com' + path
        headers = {
            'Accept': 'application/vnd.github.v3+json',
            'User-Agent': 'OpenWrt',
        }
        req = urllib.request.Request(url, headers=headers)
        sslcontext = ssl._create_unverified_context()
        fileobj = urllib.request.urlopen(req, context=sslcontext)
        return fileobj

    def _error(self, msg):
        return DownloadGitHubError('{}: {}'.format(self.source, msg))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--dl-dir', default=os.getcwd(), help='Download dir')
    parser.add_argument('--url', help='Download URL')
    parser.add_argument('--subdir', help='Source code subdir name')
    parser.add_argument('--version', help='Source code version')
    parser.add_argument('--source', help='Source tarball filename')
    parser.add_argument('--hash', help='Source tarball\'s expected sha256sum')
    args = parser.parse_args()
    try:
        method = DownloadGitHubTarball(args)
        method.download()
    except Exception as ex:
        sys.stderr.write('{}: Download from {} failed\n'.format(args.source, args.url))
        sys.stderr.write('{}\n'.format(ex))
        sys.exit(1)

if __name__ == '__main__':
    main()
