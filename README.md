BYB Spike Recorder
===================

A neural recording app for the PC and Macs.  Runs on Windows, Linux and OSX


Building instructions
------------------------

To build BYB Spike Recorder follow these steps.

### Installing dependencies

***on Linux***

Open Terminal and install git by typing:
```
$ sudo apt-get install git-core git-gui git-doc
$ git config –global user.name “your-username”
$ git config –global user.email “your@email.com”
```
Make a directory where you will keep git repositories
Go to directory that you made and type:
```
$ git init
```
after that clone repository:
```
$ git clone https://github.com/BackyardBrains/Spike-Recorder.git
```
When we cloned repository we have to install tools that will enable us to compile the code:
```
$ sudo apt install make
$ sudo apt-get install build-essential g++
```
After that we have to install libraries:
```
$ sudo apt-get install -y libhidapi-dev
$ sudo apt-get install libsdl2-dev
```
(this works for Ubuntu for different systems it might differ: https://wiki.libsdl.org/Installation)
```
$ sudo apt-get install libsdl2-image-dev
```

The last dependency is libbass which isn’t available as a package in most distributions. Therefore you have to install it manually. To do that, download it from
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

go to directory Spike-Recorder and type:
```
$ make
```


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
