# wcc

Low level language that compiles directly to WebAssembly. The goal of the project is to be as close to WebAssembly as possible yet providing nice abstraction mechanisms to speed up the development process. The target audience of the language is people who do [raw WebAssembly](https://surma.dev/things/raw-wasm/) development for recreational purposes.

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
