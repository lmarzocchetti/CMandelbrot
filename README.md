# CMandelbrot
Mandelbrot Viewer in C using CSFML (SFML wrapper) for the graphic part. The application is multithread.
![Screenshot_20211113_183420](https://user-images.githubusercontent.com/61746163/141653472-3668a91c-0d54-4320-b6e8-2c2f1959e4e8.png)

### Prerequisite for install
Install from your GNU/Linux package manager the libraries for SFML

Debian/Ubuntu:
```
$ sudo apt install libsfml-dev libcsfml-dev
```
Arch Linux:
```
$ sudo pacman -S sfml csfml
```

### Install
Simply launch the build.sh file which is the gcc command to compile with all flag and optimizations
```
$ ./build.sh
```
If the script is not executable launch this command and relaunch the build.sh:
```
$ chmod 755 build.sh
```

### Launch
Launch the program with 1 command line argument which is the maximum iteration to compute
```
$ ./cmandelbrot <NUM_ITERATION> (ex: $ ./cmandelbrot 1000)
```

### Commands
- [Arrow keys]: for moving the image in the same iteration
- [Space]: for the next iteration

## TO DO:
- [x] Introduce a Color palette system for better color;
- [ ] Introduce a new moving system with mouse and pointer
- [ ] Change the SFML library for a lower level lib (like OpenGL);
- [ ] Don't recreate the threads but use the same 10;
- [ ] Introduce the Julia mode for explore the Julia set too
