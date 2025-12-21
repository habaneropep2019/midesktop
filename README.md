# MiDesktop
MiDesktop is classical computing at its best - no frills, plain but useable.

## About MiDesktop

MiDesktop (formerly MiDE) is a fork of KDE version 1.1.2, the last release of the 1.x series. Since then, it's been ported to the Osiris toolkit (which is a fork of Qt 2.3.2).

The goal is to modernize MiDesktop without filling it with a bunch of features and bloat, retaining the essence of 90's and early 2000's computing without the pain points.

**Important:** MiDesktop is still in the early stages and as such should be considered **experimental software**, in that it could have serious bugs or security flaws that have not been discovered yet. Use at your own risk!

## Building MiDesktop

### System Requirements

A Linux system with glibc and an X11R7+ compliant display server such as XWayland or XOrg. Wayback has been tested. XLibre has not been tested. XNest is permitted for some testing but behaves erraneously and bug reports for XNest-specific issues will generally not be accepted.

You will need the following libraries and their development files/packages:

- libjpeg
- libtiff
- libpng
- Osiris Toolkit 2.4.4 or newer
- CMake
- libxcb/libxkb
- GCC 12 or newer
- gettext

I'm sure I'm missing some stuff there - this list isn't exhaustive and will likely be updated as I have time to test and double check it (as well as make distribution required package lists for building).

**Note:** CMake may complain about not finding libxcb/xkb and/or libtiff. As long as these are installed and compilation is successful, you can safely ignore these warnings (this is a bug in the CMake build system).

### Compiling and installing MiDesktop

MiDesktop is built in two steps. First, the libraries are built, then the base.

MiDesktop should **always** be installed in the `/opt` prefix. It cannot be installed alongside any other KDE version unless it is installed in `/opt` - this will be fixed in a future release.

libs:
```
cd libs
mkdir build && cd build
cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX=/opt/kde1 -DCMAKE_BUILD_TYPE=Debug ..
ninja
ninja install # May need to be run with sudo or as root
cd ../..
```

base:
```
cd base
mkdir build && cd build
cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX=/opt/kde1 -DCMAKE_BUILD_TYPE=Debug ..
ninja
ninja install # May need to be run with sudo or as root
cd ../..
```

### Post-install configuration

A few things need to be done to ensure that MiDesktop is usable.

First, you need to update the `PREFIX` environment variable.

Easiest way is to create a script in `/etc/profile.d/` as root:

```
touch /etc/profile.d/midesktop.sh
echo '#!/bin/bash' > /etc/profile.d/midesktop.sh
echo 'export PATH=$PATH:/opt/kde1/bin'
chmod +x /etc/profile.d/midesktop.sh
```


Finally, in order for you to be able to login with a MiDesktop session from your display manager, create the file `/usr/share/xsessions/midesktop.desktop` with the following contents:

```
[Desktop Entry]
Name=MiDesktop
Comment=MiDesktop
Exec=/opt/kde1/bin/startkde
TryExec=/opt/kde1/bin/startkde
Type=Application
X-LightDM-DesktopName=MiDesktop
DesktopNames=MiDesktop
```

**Most importantly** logout *and* reboot. This will ensure that all environment variables are updated next time you log in. Users have reported issues when they have not rebooted and only logged out and back in again. This is not a MiDesktop bug, rather, how some distributions/software work.

Congratulations, you should now have a working MiDesktop install!

## Testing and bugs

Well, it's known that MiDesktop has a lot of bugs and quirks. Bug reports are very welcome at https://centre.libranext.com/libranext/mide/-/issues

### Known issues

- Some menus on the lower taskbar do not remain open when clicked, i.e. opens and closes immediately
- KMenuEdit does not display any menu contents and is functionally unusable at the moment
- MiDesktop is unaware of multi-monitor setups.
- Scrolling in applications using the mousewheel does not work currently
- Firefox will maximize its size infinitely - it appears to not know where the actual edge of the screen is. This appears to only be an issue on multi-monitor setups. On single monitor setups, Firefox works normally
- Google Chrome is unable to maximize its window size. This is probably related to the Firefox bug.
- No wallet on startup
- No sound support
- I know I've forgotten quite a few. This software is old, the fact that it works at all on modern Linux is fantastic.
- Please submit bug reports if you run across issues.

