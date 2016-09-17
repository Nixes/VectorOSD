
local action = _ACTION or ""

solution "nanovg"
	location ( "build" )
	configurations { "Debug", "Release" }
	platforms {"native", "x64", "x32"}

   	project "nanovg"
		language "C"
		kind "StaticLib"
		includedirs { "nanovg" }
		files { "nanovg/*.c" }
		targetdir("build")
		defines { "_CRT_SECURE_NO_WARNINGS" } --,"FONS_USE_FREETYPE" } Uncomment to compile with FreeType support

		configuration "Debug"
			defines { "DEBUG" }
			flags { "Symbols", "ExtraWarnings"}

		configuration "Release"
			defines { "NDEBUG" }
			flags { "Optimize", "ExtraWarnings"}


    project "VectorOSD"
		kind "ConsoleApp"
			language "C++"
			files { "src/VectorOSD.cpp", "src/perf.c" }
			includedirs { "nanovg", "src", "telemetry" }
			targetdir("build")
			links { "nanovg" }

            configuration { "linux" }
                linkoptions { "`pkg-config --libs glfw3`" }
                links { "GL", "GLU", "m", "GLEW" }
                defines { "NANOVG_GLEW" }

            configuration { "windows" }
                links { "glfw3", "gdi32", "winmm", "user32", "glew32", "glu32","opengl32", "kernel32" }
                defines { "NANOVG_GLEW", "_CRT_SECURE_NO_WARNINGS" }

            configuration { "macosx" }
                links { "glfw3" }
                linkoptions { "-framework OpenGL", "-framework Cocoa", "-framework IOKit", "-framework CoreVideo" }

            configuration "Debug"
                defines { "DEBUG" }
                flags { "Symbols", "ExtraWarnings"}

            configuration "Release"
                defines { "NDEBUG" }
                flags { "Optimize", "ExtraWarnings"}
