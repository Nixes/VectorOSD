#VectorOSD

A fast and smooth opengl(es) OSD for fpv.

Makes use of NanoVG for drawing.

Requires a c++11 compatible compiler for now, you can set this in gcc by adding the argument
```
-std=c++11
```
#Ubuntu / Debian Instructions

Install dependencies
```
sudo apt-get install premake4 make build-essential libglew-dev libglfw3-dev
```

Clone repo
```
git clone --recursive https://github.com/Nixes/VectorOSD.git
cd VectorOSD
```

Generate makefiles
```
premake4 gmake
```

Build
```
cd build
make
```

#Windows MSYS Instructions (x86-64)

Install Dependencies
```
pacman -S mingw-w64-x86_64-gcc make mingw-w64-x86_64-glew mingw-w64-x86_64-glfw
```

Install libserialport
```
pacman -S base-devel
git clone git://sigrok.org/libserialport
./autogen.sh
./configure
make install
```

Clone repo
```
git clone --recursive https://github.com/Nixes/VectorOSD.git
cd VectorOSD
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
