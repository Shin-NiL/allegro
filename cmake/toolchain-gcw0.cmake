# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
SET(CMAKE_C_COMPILER   /opt/gcw0-toolchain/usr/bin/mipsel-gcw0-linux-uclibc-gcc.exe)
SET(CMAKE_CXX_COMPILER /opt/gcw0-toolchain/usr/bin/mipsel-gcw0-linux-uclibc-g++.exe)

# where is the target environment 
SET(CMAKE_FIND_ROOT_PATH  /opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc /opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot/usr)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# multiple settings
SET(ALLEGRO_USE_CONSTRUCTOR_EXITCODE FAILED_TO_RUN)
