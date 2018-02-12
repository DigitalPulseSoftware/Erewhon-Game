# Pre-requisites

Compiling Erewhon requires a C++17 compiler, such as MSVC 15.5 (coming with Visual Studio 2017), Clang 4 or GCC 7.

## Windows

First, you have to download the required libraries to compile the project:

### Nazara Engine

You will need to download a precompiled package of [Nazara](https://github.com/DigitalPulseSoftware/NazaraEngine], or compile it yourself (by following the instructions on the repository).
To download a precompiled package of the last commit, go [here](https://ci.appveyor.com/project/DrLynix/nazaraengine/branch/master) and select a DebugDynamic/ReleaseDynamic x64 configuration according to what you intend to do with this project (Debugging is recommended for development), then click on the Artifacts tab and finally download the package\NazaraEngine.7z file.

It is also possible to compile it yourself, just don't forget to package the engine before using it.

### libpq (Server-only)

You can download a precompiled libpq package made by myself [here](https://utopia.digitalpulsesoftware.net/files/pgsql.7z)  
You can also find it in the precompiled Windows binaries of PostgreSQL, along with PostgreSQL itself and PGadmin4 software, you can download it from the [PostgreSQL website](https://www.postgresql.org/download/windows/) (download a zip archive, **not the installer**), you will have to put the includes in a `postgresql` directory (making `<pgsql directory>/include/postgresql/libpq-fe.h` a valid path), in order to make the include path compatible with the Linux package.
It is also possible, but should not be required, to download and compile libpq yourself ([repository](https://github.com/postgres/postgres)) but this is beyond the scope of this document. 

### Generating the project

Go to the `build` folder.

Replace the two configuration entries values (`NazaraPath` and `PostgresClientPath`) by the paths where you installed Nazara Engine package and libpq in config.lua.

Example configuration:
```lua
NazaraPath = [[C:\NazaraEngine\package]]
PostgresClientPath = [[C:\pgsql]]
```

Open cmd.exe (or bash.exe if you're using WSL) and run `premake.exe <action>` with the action you're targeting (`vs2017` if you're using Visual Studio, use `premake.exe --help` to see which actions are availables`).

The project files, used to build both the client and server (as you would do with any regular project), will be placed in a new folder, named after the action you've chosen

You can now start the client/server (just don't forget to copy the assets, config and scripts file at the project root next to your .exe)

## Linux

<todo>

```sh
```

## macOS

Currently, macOS is **not** supported, due to the lacking support from [Nazara Engine](https://github.com/DigitalPulseSoftware/NazaraEngine).  
If you're a macOS developper, please consider helping us by improving support for your OS in [our engine](https://github.com/DigitalPulseSoftware/NazaraEngine).

# FAQ

## **1. Q: Why is 64bits the only supported architecture?**

A: Supporting only 64bits processors makes the whole project easier to maintain, plus according to [Steam Survey](http://store.steampowered.com/hwsurvey/) only about ~2% of users are still using 32bits OS (with virtually all processors supporting 64bits instructions).  
Thus, it didn't make any point to support 32bits softwares.

**Still**, it is possible to build Erewhon (client or server) as a 32bits software, since our dependencies are also available for 32bits OS and the project's source code doesn't assume it will be compiled as a 64bits software.
