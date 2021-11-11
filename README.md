# CMandelbrot
Mandelbrot Viewer in C using CSFML (SFML wrapper) for the graphic part. The application is multithread.

### Prerequisite for install
Install from your GNU/Linux package manager the libraries for SFML

Debian/Ubuntu:
```
$ sudo apt install libsfml-dev libcsfml-dev
```
Arch Linux:
```
$ pacman -S sfml csfml
```

### Install
Simply launch the build.sh file which is the gcc command to compile with all flag and optimizations
```
$ ./build.sh
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
- [ ] Introduce a Color palette system for better color;
- [ ] Introduce a new moving system with mouse and pointer
- [ ] Change the SFML library for a lower level lib (like OpenGL);
- [ ] Don't recreate the threads but use the same 12;
- [ ] Introduce the Julia mode for explore the Julia set too
