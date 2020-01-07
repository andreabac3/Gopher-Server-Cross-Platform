# GOPHER-PROJECT

This is a Gopher server written purely in C. It was presented as a project for the System Programming course under the supervision of professor Massimo Bernaschi.
As required for the exam, in addition to the implementation of the Gopher server, it is necessary to develop a process for writing Logs, using the 'Select' and other features to increase the total complexity of the project.
The software is compatible with both Windows 10 and Linux.

## The Gopher Protocol

Wikipedia: The Gopher protocol is a communications protocol designed for distributing, searching, and retrieving documents in Internet Protocol networks. The design of the Gopher protocol and user interface is menu-driven, and presented an alternative to the World Wide Web in its early stages, but ultimately fell into disfavor, yielding to the Hypertext Transfer Protocol (HTTP). The Gopher ecosystem is often regarded as the effective predecessor of the World Wide Web.

## Getting Started

### Compilers

#### Linux:

* GCC 9.x

#### Windows 10:

* Mingw

## How to install

Compile with CMakeLists.txt, this software is compatible with Linux and Windows 10.
Execute the following command in the cmake-build-debug folder.

```sh
$ cmake
$ make
$ ./gopherLinux
```

For use with the ubuntu distribution you need to run the following command

```sh
$ sudo apt-get install build-essential zlib1g-dev
```

The structure of this project is 100% compatible with Clion and other JetBrains products.
## CommandLine Option

For each option having {param}, replace param with an appropriate argument.<br>
*   **-d "{param}"**   (as Directory) followed with the path to use as root for sending files. Use of the spaces in the path are allowed.
*   **-m {param}**   (as Mode_Concurrency) followed with the string 'Thread' or 'Process' for the selection of the parallel mode for sending requests.
*   **-p {param}**   (as Port) replace param with the integer for specify the port to use
*   **-i {param}**   replace param with a string for specify the hostname.
*   **-e**   to enable daemon mode.
*   **-s**  (as Security) to enable protection for the DOS attack.

## Configuration File

All params are required.
<br> Examples of Configuration File:

```plain
port 7080
root_dir "./rootDirectory/"
mode_concurrency Thread
external_ip cloud.myservergopher.com
```

```plain
port 7000
root_dir "~/gopher_root/"
mode_concurrency Process
external_ip gopher.theprotocol.it
```

## Tests

The Gopher server supports more than 50.000 requests simultaneously. Has limited use of memory, tests with Valgrind report zero errors or warnings. <br>
You can find the stress script in the root of the project, It's a file bash called stressCurl.sh

### Operating Systems tested

#### Windows 10

#### Linux:

*   **Arch Linux**
*   **Manjaro**
*   **Debian 9.x**

### Known Issues

Lost the compatibility with macOS due to an unresolved Apple bug. 90% of the software is compatible with macOS X Mojave. <br>

### Future Works

Clean the code and fix the known issues.

## Authors

*   **Andrea Bacciu**  - [github](https://github.com/andreabac3)
*   **Valerio Neri**  - [github](https://github.com/ValerioNeriGit)
