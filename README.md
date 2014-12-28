BYB Spike Recorder
===================

A neural recording app for the PC and Macs.  Runs on Windows, Linux and OSX


Building instructions
------------------------

To build BYB Spike Recorder follow these steps.

### Installing dependencies

***on Linux***

```
$ sudo apt-get install build-essential libsdl1.2-dev libsdl-image1.2-dev
```

The last dependency is libbass which isn’t available as a package in most
distributions. Therefore you have to install it manually.
To do that, download it from

http://www.un4seen.com/download.php?bass24-linux

and unpack it into a temporary directory.

```
$ mkdir /tmp/bass
$ cd /tmp/bass
$ unzip ~/path/to/bass24-linux.zip
```

and copy the library to the system library directory

```
$ sudo cp x64/libbass.so /usr/lib/
```

(or on a 32-bit system 'cp libbass.so /usr/lib')

***on OS X***

Install the dependencies [libsdl](https://www.libsdl.org/download-1.2.php) and [sdl_image](https://www.libsdl.org/projects/SDL_image/release-1.2.html) by downloading the linked .dmg files and moving the .framework files to `/Library/Frameworks`.

### Compilation

```
$ cd BYB-Neural-Recorder
$ make
```

### Running

If everything worked, you can run it by

```
$ ./SpikeRecorder
```
