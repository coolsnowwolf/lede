# OpenWrt inside a user mode linux

> Why would we even want this many ask?

There are potentially a lot of reasons, one obvious one to me, it allows folks
to 'kick the tires' without actually flashing up any hardware.  It's also a
great environment for porting over packages, you can get a package fully
functional in the uclibc root environment inside a uml without actually
disturbing your 'real router', and then rebuild for a specific target once it's
fully tested.

This is a first stab at a build that 'just works' and there will be more
cleanup to come.  The simple directions are:-

* Configure for uml target
* Configure with an ext4 or squashfs root file system
* Build it all

In your bin directory you will find a Kernel and an root file system when it's
finished. Just run it like this:-

```shell
./openwrt-uml-generic-vmlinux ubd0=openwrt-uml-generic-squashfs.img
```

The uml will start and eventually the serial console of the uml will be at your
console prompt. If you would like it in xterms, substitute `con=xterm` and
`con0=xterm`. **No networking is configured** but it's a starting point. The
resulting file system has just enough free space to start kicking the tires and
playing in the world of 'embedded routers' along with all the resource
restrictions that come with that world.

To configure networking and more refer to the *user mode linux* documentation
online:
https://docs.kernel.org/virt/uml/user_mode_linux_howto_v2.html#setting-up-uml-networking

Actually functional uml networking can be a challenge at times. Again, if you are
unfamiliar with uml and uml networking, please read the docs and how-to stuff available
on the net. It does take some fiddling to get it started and working right the first
time, but after that, it opens up a whole new world of virtual machines.
