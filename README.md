#VectorOSD

A fast and smooth opengl(es) OSD for fpv.

Makes use of NanoVG for drawing.

Requires a c++11 compatible compiler for now, you can set this in gcc by adding the argument
```
-std=c++11
```


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



Build (make sure you use the "MinGW-w64 Win64 Shell" since we installed the 64bit packages)
```
cd VectorOSD/build
make
```


Run VectorOSD from build folder!
