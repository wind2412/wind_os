# wind_os

a simple os, no gui

## ATTENTIONS

The bootloader is from XV6. OS designing thinking is from mostly ucore, little from linux 0.11 kernel. And hurley_os kernel helped a lot, on my os learing road.

Thank for all of U !
    
This kernel doesn't has a file system. It is really simple for os learning.

## Completions
- [x] interrupt vectors
- [x] global descriptor table
- [x] page management
- [x] malloc
- [x] virtual memory
- [x] process
- [x] atom / synchronize
- [x] P/V semaphore
- [x] monitor     

## Install and Play

To make it, please:
   
    `git clone https://github.com/wind2412/wind_os.git`


1. if you are on i386 linux, please delete the `Makefile`'s all prefix `i386-elf-`, and run `make && make qemu` is okay.
2. if you are on mac OS, please install i386 tool chains first.
    
```no language
brew tap wind2412/homebrew-gcc_cross_compilers
brew install i386-elf-binutils
brew install i386-elf-gcc
brew install i386-elf-gdb
make all && make qemu
```

is okay.

Enjoy~

