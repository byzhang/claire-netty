# Installation Guide

claire-nettyis a collection of utility for build modern c++ network application. 

It is designed for used in x86-64 Linux. Beacause claire-nettyuse some new kernel api(timerfd, eventfd...), kernel version require at least more than 2.6.32.

Currently claire-netty use [CMake][1] build the library, require at least 2.8 version.

It depends claire-common(with all its dependent library) and serveral other library:
 
- [c-ares][2]: claire-netty use c-ares support dns resolver.

- [gperftools][3]: claire-netty implement remote server profiling through gperftools,  and it need install libunwind 0.99 first. 

- [ctemplate][4]: claire-netty use ctemplate implement html output

After install all before, run ./build.sh and ./build.sh install.

Done.


  [1]: http://www.cmake.org/
  [2]: http://c-ares.haxx.se/
  [3]: https://code.google.com/p/gflags/
  [4]: https://code.google.com/p/googletest/
  [5]: http://stackoverflow.com/questions/13513905/how-to-properly-setup-googletest-on-linux
  [6]: https://code.google.com/p/protobuf/
  [7]: https://code.google.com/p/rapidjson/