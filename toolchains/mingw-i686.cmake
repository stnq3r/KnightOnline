# Cross-compile toolchain for building the OpenKO Windows client on macOS
# using Homebrew's mingw-w64 (32-bit / Win32 target to match the original build).
#
# Usage:
#   cmake -S . -B <builddir> -DCMAKE_TOOLCHAIN_FILE=toolchains/mingw-i686.cmake ...

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR i686)

set(_MINGW_ROOT "/opt/homebrew/Cellar/mingw-w64/14.0.0_3/toolchain-i686")
set(_MINGW_TRIPLE "i686-w64-mingw32")

set(CMAKE_C_COMPILER   "/opt/homebrew/bin/${_MINGW_TRIPLE}-gcc")
set(CMAKE_CXX_COMPILER "/opt/homebrew/bin/${_MINGW_TRIPLE}-g++")
set(CMAKE_RC_COMPILER  "/opt/homebrew/bin/${_MINGW_TRIPLE}-windres")
set(CMAKE_AR           "/opt/homebrew/bin/${_MINGW_TRIPLE}-ar")
set(CMAKE_RANLIB       "/opt/homebrew/bin/${_MINGW_TRIPLE}-ranlib")

set(CMAKE_FIND_ROOT_PATH "${_MINGW_ROOT}/${_MINGW_TRIPLE}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
