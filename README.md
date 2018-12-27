# Dragonet

## Multi-core pub/sub IPC framework for embedded systems

Publish-subscribe IPC framework for heterogenous multi-core embedded systems running Linux and FreeRTOS, using [LCM](https://github.com/lcm-proj/lcm), [RPMsg](https://github.com/OpenAMP/open-amp/tree/master/lib/rpmsg), and FreeRTOS queues. Currently supports the i.MX 6SoloX (tested on [Dragonflyte flight computer](https://github.com/raphaelchang/dragonflyte-hardware)).

<img width="400" src="https://raphaelchang.com/wp-content/uploads/2018/12/dragonet.png">

## Setup
### Installing on Linux
For the MPU side running Linux, the package is built as a shared library. You will also need to modify the RPMsg-char driver  (available in Linux >= 4.11) with this [patch](https://github.com/OpenAMP/meta-openamp/blob/master/recipes-kernel/linux/openamp-kmeta/cfg/0001-rpmsg-virtio-rpmsg-Add-RPMsg-char-driver-support.patch).

Install LCM by following the instructions [here](https://lcm-proj.github.io/build_instructions.html). Then, run the following commands in the repository directory to install the Dragonet library.
```
cmake .
make
make install
```
To use the library in your own project, link the library and include `dragonet.h`.

### Running in Linux
Run the following commands before running the library. This assumes that the RPMsg-char driver is built as a kernel module, and that the MCU side is already running a Dragonet instance.
```
insmod /lib/modules/[kernel_version]/kernel/drivers/rpmsg/rpmsg_char.ko
ifconfig lo multicast
route add -net 224.0.0.0 netmask 240.0.0.0 dev lo
```

### Building with FreeRTOS
For the MCU side, the library is built as a static library that is linked into a C++ project that has FreeRTOS and [RPMsg-Lite](https://github.com/NXPmicro/rpmsg-lite) (v2.0.0) included already. If you are using CMake, you can include `cmake/DragonetStatic.cmake` as well as `inc/platform/[platform]/dragonet_platform.h` from your `CMakeLists.txt`, and then link the library into your project. Include `dragonet.h` in the rest of the project to use the library.

## Usage
The usage of the library is the same regardless of the platform. On Linux, only one instance of the Dragonet object should be created per process. On an MCU running FreeRTOS, there can only be one Dragonet instance in the entire program. This usually means the instance is created on startup and passed to several FreeRTOS tasks. `Init()` must be called before the other functions can be used. On FreeRTOS, `Init()` must be called after the scheduler starts, usually by calling `vTaskStartScheduler()`. This means `Init()` will have to be called inside a task. Because multiple tasks will attempt to call `Init()`, it is designed to allow this.

### Subscriber example
```
dragonet::Dragonet dragonet_;
int callback(const int *data)
{
  // Do something
}
int main()
{
  dragonet_.Init();
  dragonet_.Subscribe("channel-name", callback);
  dragonet_.Spin();
}
```

### Publisher example
```
dragonet::Dragonet dragonet_;
int main()
{
  dragonet_.Init();
  for (int data = 0; data < 10; data++)
  {
    dragonet_.Publish("channel-name", &data);
  }
}
```
