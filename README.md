# bee.lua

Lua runtime and toolset

| MSVC 15.8 (Windows) | MinGW 8.2 (Windows) | Clang 10.0 (macOS) | GCC 8.2 (Linux) |
| ------ | ------ | ------ | ------ |
| [![Build Status](https://dev.azure.com/actboy168/bee.lua/_apis/build/status/bee.lua?branchName=master&jobName=windows_msvc)](https://dev.azure.com/actboy168/bee.lua/_build/latest?definitionId=5?branchName=master) | [![Build Status](https://dev.azure.com/actboy168/bee.lua/_apis/build/status/bee.lua?branchName=master&jobName=Windows_MinGW)](https://dev.azure.com/actboy168/bee.lua/_build/latest?definitionId=5?branchName=master) | [![Build Status](https://dev.azure.com/actboy168/bee.lua/_apis/build/status/bee.lua?branchName=master&jobName=macos)](https://dev.azure.com/actboy168/bee.lua/_build/latest?definitionId=5?branchName=master) | [![Build status](https://ci.appveyor.com/api/projects/status/qfp4flrsoi1aat41?svg=true)](https://ci.appveyor.com/project/actboy168/bee-lua) |

## Build

* install ninja
* ninja -f ninja/msvc.ninja
* ninja -f ninja/mingw.ninja
* ninja -f ninja/linux.ninja
* ninja -f ninja/macos.ninja

## Lua patch

* Enable ansi escape code on windows
* String encoding on windows using utf8
* Remove randomness when traversing the table
* Disable load binary chunk (for security)
* Add error hook (for debugger)
* Add resume hook (for debugger)
* Add lua_getprotohash (for debugger)

## Feature

|            | Windows | Linux | macOS |
| ---------- | ------- | ----- |------ |
| filesystem |   Yes   |  Yes  |  Yes  |
| thread     |   Yes   |  Yes  |  Yes  |
| socket     |   Yes   |  Yes  |  Yes  |
| subprocess |   Yes   |  Yes  |  Yes  |
| filewatch  |   Yes   |  Yes  |  Yes  |
| registry   |   Yes   |  N/A  |  N/A  |

## TODO

* High-level network library
