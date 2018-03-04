#-*- coding:utf-8 -*-

import httplib
from urlparse import urlparse

urlkey = 'HTTP_X_FORWARDED_URL'

def valid(key):
    return key.startswith('HTTP_') and not key.startswith('HTTP_BAE') and key not in [urlkey, 'HTTP_X_HOST']

class WSGIGofwRelayApplication(object):
    def handler(self, environ, start_response):
        """goflyway mitm relay proxy in python
        """

        try:
            url = urlparse(environ[urlkey])
            connection = httplib.HTTPConnection(url.netloc)
            path = url.geturl().replace('%s://%s' % (url.scheme, url.netloc), '')
        except Exception:
            start_response('400 Bad Request', [('Content-Type', 'text/html')])
            yield str(environ)
            return

        body = None
        try:
            length = int(environ['CONTENT_LENGTH'])
        except (KeyError, ValueError):
            pass
        else:
            body = environ['wsgi.input'].read(length)

        headers = dict((key[5:].lower().replace('_', '-'), value) for key, value in environ.items() if valid(key))
        headers['host'] = url.netloc
        if 'CONTENT_TYPE' in environ:
            headers['content-type'] = environ['CONTENT_TYPE']

        # start_response('200 OK', [('Content-Type', 'text/html')])
        # yield str(headers)
        # return

        try:
            connection.request(environ['REQUEST_METHOD'], path, body=body, headers=headers)
        except Exception as e:
            start_response('500 Internal Server Error', [('Content-Type', 'text/html')])
            yield str(e)
            return

        response = connection.getresponse()
        start_response('{0.status} {0.reason}'.format(response), response.getheaders())

        while True:
            chunk = response.read(4096)
            if chunk:
                yield chunk
            else:
                break

    def __call__(self, environ, start_response):
        return self.handler(environ, start_response)

application = WSGIGofwRelayApplication()
