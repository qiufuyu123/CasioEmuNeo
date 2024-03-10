# Emulator

Assuming your current directory is this one, you have
[meson](https://mesonbuild.com/) and [ninja](https://ninja-build.org/) installed
and you have the proper ROM (name `rom.bin`) in `../../models/<model>`:

```
$ meson build && cd build
$ meson configure -Dwarning_level=3 -Dcpp_std=c++11
$ ninja
$ ./emulator ../../models/<model>
```

Unlike the previous, terrible excuse of a build system, this one is likely to
work well on platforms other than unices with coreutils and other shady stuff
installed.
