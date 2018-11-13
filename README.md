# Dragonet

## Multi-core pub/sub IPC framework for embedded systems

Publish-subscribe IPC framework for heterogenous multi-core embedded systems running Linux and FreeRTOS, using [LCM](https://github.com/lcm-proj/lcm), [RPMsg](https://github.com/OpenAMP/open-amp/tree/master/lib/rpmsg), and FreeRTOS queues. Currently supports the i.MX 6SoloX (tested on [Dragonflyte flight computer](https://github.com/raphaelchang/dragonflyte-hardware)).

## Usage
### Installing on Linux
For the MPU side running Linux, the package is built as a shared library. You will also need to modify the RPMsg-char driver  (available in Linux >= 4.11) with this [patch](https://github.com/OpenAMP/meta-openamp/blob/master/recipes-kernel/linux/openamp-kmeta/cfg/0001-rpmsg-virtio-rpmsg-Add-RPMsg-char-driver-support.patch).
```
cmake .
make
make install
```
To use the library in your own project, link the library and include `dragonet.h`.

### Running in Linux
Run the following commands before running the library. This assumes that the RPMsg-char driver is built as a kernel module, and that the MCU side is already running a Dragonet instance.
```
insmod /lib/modules/[kernel_version]kernel/drivers/rpmsg/rpmsg_char.ko
ifconfig lo multicast
route add -net 224.0.0.0 netmask 240.0.0.0 dev lo
```

### Building with FreeRTOS
For the MCU side, the library is built as a static library that is linked into a C++ project that has FreeRTOS and [RPMsg-Lite](https://github.com/NXPmicro/rpmsg-lite) (v2.0.0) included already. If you are using CMake, you can include `cmake/DragonetStatic.cmake` as well as `inc/platform/[platform]/dragonet_platform.h` from your `CMakeLists.txt`, and then link the library into your project.
