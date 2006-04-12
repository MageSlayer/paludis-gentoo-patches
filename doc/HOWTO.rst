=====================================
HOWTO Break your system using Paludis
=====================================
---------------------------------------------
and maybe get a usable chroot if you're lucky
---------------------------------------------

.. DANGER::
   READ EVERYTHING CAREFULLY, OR YOU WILL BREAK YOUR SYSTEM!

THIS WILL PROBABLY NOT ACTUALLY WORK! Paludis is at the very early alpha stage.
It can sometimes install things, if you're very lucky, but it can't uninstall
them, upgrade them sensibly or anything like that. There are zillions of things
that are not implemented. There are zillions of things that Paludis does
differently from Portage -- some of these are intentional, some are not.

Do not try to use Paludis and Portage to install things inside the same root.
The config and vdb formats are not compatible!

Install Paludis locally, being sure to configure sysconfdir as /etc to avoid
confusion later on. There's an ebuild at::

    http://svn.berlios.de/viewcvs/*checkout*/paludis/overlay/sys-apps/paludis/paludis-0.ebuild

You'll need libebt, eselect and subversion.

.. Important:: You should seriously consider grabbing an svn snapshot of
  eselect if 1.0.2 hasn't been released yet, or you will encounter weird "You
  are not root" errors even when you're root.

Make sure the test suite passes (either FEATURES="test" for Portage, or 'make
check'). If it fails, don't continue until you figure out why.

Set up a local bootstrap config directory. We're assuming you're using
/mychroot as the root path. The reason for doing things this way will become
apparent later on.

::

    mkdir ~/.paludis-bootstrap
    cat <<END > ~/.paludis-bootstrap/specpath
    root = /mychroot
    config-suffix =
    END

Make some skeleton directories and files::

    mkdir -p /mychroot/etc/{env.d,paludis/repositories}
    mkdir -p /mychroot/var/{db/pkg,paludis/repositories/gentoo/distfiles}
    mkdir -p /mychroot/tmp/
    touch /mychroot/etc/ld.so.conf

Set up your keywords.conf. At the very least, you'll need a "* keywords" line.
Further lines can be added in the form "atom keywords". Like with Portage,
accepting ``~keyword`` does *not* accept ``keyword``, so if you want a fully
``~arch`` system you should use ``* arch ~arch``.

::

    cat <<END > /mychroot/etc/paludis/keywords.conf
    * x86
    dev-cpp/libebt x86 ~x86
    sys-apps/paludis x86 ~x86
    dev-util/subversion x86 ~x86
    app-admin/eselect x86 ~x86
    app-editors/vim x86 ~x86
    app-editors/vim-core x86 ~x86
    END

Set up your use.conf. At the very least, you'll need a "* flags" line. Again,
additional per-atom lines can be specified. The -apache2 is important, if you
value your sanity, since you'll be installing subversion::

    cat <<END > /mychroot/etc/paludis/use.conf
    * -doc nls unicode -apache2
    app-editors/vim -nls
    END

Set up your package_unmask.conf and package_mask.conf, if necessary::

    cat <<END > /mychroot/etc/paludis/package_unmask.conf
    app-editors/vim
    app-editors/vim-core
    END

Set up your bashrc. This must NOT be used to change any values that affect
dependency resolution (e.g. USE, LINGUAS). It can be used to set CFLAGS, CHOST
and the like (on some archs you'll have to do this to avoid getting junk from
your profile)::

    cat <<END > /mychroot/etc/paludis/bashrc
    export CFLAGS="-O2 -march=pentium4 -fomit-frame-pointer"
    export CXXFLAGS="\${CFLAGS}"
    export CHOST="i686-pc-linux-gnu"
    END

Set up your repository files. Do not tinker with the VDB location! Here we'll
avoid using /usr/portage for the main tree because sticking data that gets
changed on /usr is silly. We use the ${ROOT} variable, which is set magically,
to make the config work both in and outside of a chroot (this is one of the
reasons we have the weird-looking specpath thing)::

    cat <<END > /mychroot/etc/paludis/repositories/gentoo.conf
    location = \${ROOT}/var/paludis/repositories/gentoo/
    sync = rsync://rsync.europe.gentoo.org/gentoo-portage/
    profile = \${ROOT}/var/paludis/repositories/gentoo/profiles/default-linux/x86/2006.0
    format = portage
    END

    cat <<END > /mychroot/etc/paludis/repositories/installed.conf
    location = \${ROOT}/var/db/pkg/
    format = vdb
    END

    cat <<END > /mychroot/etc/paludis/repositories/paludis-overlay.conf
    location = \${ROOT}/var/paludis/repositories/paludis-overlay/
    sync = svn://svn.berlios.de/paludis/overlay
    profile = \${ROOT}/var/paludis/repositories/gentoo/profiles/default-linux/x86/2006.0
    eclassdir = \${ROOT}/var/paludis/repositories/gentoo/eclass
    distdir = \${ROOT}/var/paludis/repositories/gentoo/distfiles
    cache = /var/empty
    format = portage
    importance = 10
    END

Now check that the config looks ok, and sync::

    paludis --config-suffix bootstrap --list-repositories
    sudo paludis --config-suffix bootstrap --sync

If you have problems, try adding "--log-level debug". This may or may not give
helpful information....

The initial sync will be slow. You can cheat and copy an existing Portage tree
checkout into /mychroot/var/paludis/repositories/gentoo/, but remember to
preserve mtimes and permissions if you do. Note that there's no hideously
painful 'Updating the Portage cache...' to go through. Paludis will use the
metadata cache, if available, but does not use the dep cache.

Now install baselayout and then system. We install baselayout manually first
because it's easier than creating a bunch of directories by hand.

Note that Paludis will use src_test regardless of FEATURES (FEATURES is a
Portage thing, and Paludis doesn't use it any more than it has to).

.. Important:: Unfortunately, various system packages have broken test suites,
  so the system install will probably bomb out midway unless you export
  SKIP_FUNCTIONS=test beforehand. You can do this in your environment or
  (better) in ``/mychroot/etc/paludis/bashrc``. If you're especially sneaky,
  you can do it conditional upon ``$PN``.

Also note that there're a whole load of circular dependencies in system
(ncurses <-> gpm, patch <-> patch, gcc <-> glibc for example), so you'll
almost certainly need --dl-drop-circular at this stage.

::

    paludis --config-suffix bootstrap --install --pretend --dl-drop-all sys-apps/baselayout
    sudo paludis --config-suffix bootstrap --install --dl-drop-all sys-apps/baselayout

    paludis --config-suffix bootstrap --install --pretend --dl-drop-circular system
    sudo paludis --config-suffix bootstrap --install --dl-drop-circular system

Note that system will pull in Portage. That's a profiles thing that's
unavoidable for now. It won't pull in Paludis, so we do that manually::

    paludis --config-suffix bootstrap --install --pretend sys-apps/paludis
    sudo paludis --config-suffix bootstrap --install sys-apps/paludis

And that should (but probably won't) give you a usable chroot::

    sudo cp /etc/resolv.conf /mychroot/etc/
    sudo chroot /mychroot
    reset
    export HOME=/root
    cd
    cp /etc/skel/.bashrc .
    . .bashrc
    mount -tproc none /proc
    mount -tsysfs none /sys
    udevstart
    eselect env update
    source /etc/profile
    ( . /etc/paludis/bashrc ; gcc-config -1 )
    eselect env update
    source /etc/profile
    paludis --install app-editors/vim
    paludis --uninstall app-editors/nano
    paludis --uninstall sys-apps/portage

.. vim: set et sw=4 spell spelllang=en ft=glep :

