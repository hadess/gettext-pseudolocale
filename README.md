gettext-pseudolocale provides a shared library
to be used as an `LD_PRELOAD` to mangle English/ASCII
text into something recognisable, but that would make
internationalisation issues obvious.

Building
--------

```sh
$ mkdir build
$ cd build
$ meson ..
$ ninja
```

Usage
-----

```
LD_PRELOAD=gettext-pseudolocale.so myapplication
```

Technical details
-----------------

The shared library overrides libintl calls, which are usually part
of the glibc DSO on Linux systems. This software has not been tested
on anything but Linux.

License
-------

This code is under the same license as the glibc itself
(GNU Lesser General Public License v 2.1)

See the glibc license for details:
http://www.gnu.org/software/libc/manual/html_mono/libc.html#Copying

The current mangle table is based on the “Bent” effect from Lunicode.js,
see `bent.h` for details.

Related links
-------------

This feature was inspired by Android's _pseudolocales_. See this page
for details:
https://developer.android.com/guide/topics/resources/pseudolocales

Copyright
---------

Copyright Bastien Nocera <<hadess@hadess.net>> 2019
