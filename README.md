#VectorOSD

A fast and smooth opengl(es) OSD for fpv.

Makes use of NanoVG for drawing.


#Windows MSYS Instructions (x86-64)

Install GCC
```
pacman -S mingw-w64-x86_64-gcc
```


Install Make
```
pacman -S make
```


Install GLEW
```
pacman -S mingw-w64-x86_64-glew
```


Install glfw
```
pacman -S mingw-w64-x86_64-glfw
```

If you want to update build scripts, Run
```
./premake4 gmake
```

Build
```
cd VectorOSD/build
make
```


Run VectorOSD from build folder!
