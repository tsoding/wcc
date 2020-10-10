# wcc

Low level language that compiles directly to WebAssembly. The goal of the project is to be as close to WebAssembly as possible yet providing nice abstraction mechanisms to speed up the development process. The target audience of the language is people who do [raw WebAssembly](https://surma.dev/things/raw-wasm/) development for recreational purposes.

## Why not [AssemblyScript](https://github.com/AssemblyScript/assemblyscript)

- wcc is written in C/C++ which makes the compilation times way shorter, which is important for maintaining a healthy development cycle.
- wcc is not bound by TypeScript/JavaScript compatibility. Which enables us to innovate as much as we want. (which will bring more points to this list in the future)
- wcc will be a single executable that does not depend on binaryen/wabt/etc. (work in progress)
- ...

## Dependancies

- [GNU make 4.2.1+](https://www.gnu.org/software/make/)
- [gcc 8.3.0+](https://gcc.gnu.org/) or [clang 7.0.1+](https://clang.llvm.org/)
- [node v13.5.0+](https://nodejs.org/)
- [wabt](https://github.com/WebAssembly/wabt)

## Quick Start

``` console
$ make
$ cat ./samples/fib.wc
$ ./wcc -t wat ./samples/fib.wc
```

For more info on how to use the build do

```console
$ make help
```
