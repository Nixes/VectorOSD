#include <stdio.h>
#ifdef NANOVG_GLEW
	#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
#include "nanovg.h"
#define NANOVG_GL2_IMPLEMENTATION
#include "nanovg_gl.h"
#include "perf.h"


void errorcb(int error, const char* desc) {
	printf("GLFW error %d: %s\n", error, desc);
}

int blowup = 0;
int premult = 0;

void renderBootLogBox() {

}

void runBootCheck() {

}

void renderBatteryBar(NVGcontext* vg) {

}

void renderAttitudeIndicator(NVGcontext* vg) {

}

void renderNumber(NVGcontext* vg, float x, float y,int number) {
	char number_string[15];
	sprintf(number_string, "%d", number);

	nvgFontSize(vg, 20.0f);
	nvgFontFace(vg, "sans");
	nvgFillColor(vg, nvgRGBA(255,255,255,255));
	nvgTextAlign(vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
	nvgText(vg, x, y, number_string, NULL);
}

void renderText(NVGcontext* vg, float x, float y,const char* text) {
	nvgFontSize(vg, 20.0f);
	nvgFontFace(vg, "sans");
	nvgFillColor(vg, nvgRGBA(255,255,255,255));
	nvgTextAlign(vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
	nvgText(vg, x, y, text, NULL);
}

// actually render the objects
void render(NVGcontext* vg) {
	renderText(vg,0,10,"Fuck yeah we have text!");
	renderText(vg,500,550,"More Text");

	nvgBeginPath(vg);
	nvgRect(vg, 100,100, 120,30);
	nvgFillColor(vg, nvgRGBA(255,192,0,255));
	nvgFill(vg);
}

void loadShape() {
	// good ideas here: https://github.com/memononen/nanosvg/issues/58
	// just load some mono color glyphs for use in ui
}

void loadAssets(NVGcontext* vg) {
	// load font
	nvgCreateFont(vg, "sans", "../example/Roboto-Regular.ttf");
}

int main() {
	GLFWwindow* window;
	NVGcontext* vg = NULL;
	PerfGraph fps;
	double prevt = 0;

	if (!glfwInit()) {
		printf("Failed to init GLFW.");
		return -1;
	}

	initGraph(&fps, GRAPH_RENDER_FPS, "Frame Time");

	glfwSetErrorCallback(errorcb);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);


	window = glfwCreateWindow(1000, 600, "VectorOSD", NULL, NULL);
	//	window = glfwCreateWindow(1000, 600, "NanoVG", glfwGetPrimaryMonitor(), NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	#ifdef NANOVG_GLEW
	  if(glewInit() != GLEW_OK) {
			printf("Could not init glew.\n");
			return -1;
		}
	#endif

	#ifdef DEMO_MSAA
		vg = nvgCreateGL2(NVG_STENCIL_STROKES | NVG_DEBUG);
	#else
		vg = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
	#endif
		if (vg == NULL) {
			printf("Could not init nanovg.\n");
			return -1;
		}

	glfwSwapInterval(0);

	glfwSetTime(0);
	prevt = glfwGetTime();

	loadAssets(vg);

	while (!glfwWindowShouldClose(window)) {
		double mx, my, t, dt;
		int winWidth, winHeight;
		int fbWidth, fbHeight;
		float pxRatio;

		t = glfwGetTime();
		dt = t - prevt;
		prevt = t;
		updateGraph(&fps, dt);

		glfwGetCursorPos(window, &mx, &my);
		glfwGetWindowSize(window, &winWidth, &winHeight);
		glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

		// Calculate pixel ratio for hi-dpi devices.
		pxRatio = (float)fbWidth / (float)winWidth;

		// Update and render
		glViewport(0, 0, fbWidth, fbHeight);
		if (premult)
			glClearColor(0,0,0,0);
		else
			glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

		nvgBeginFrame(vg, winWidth, winHeight, pxRatio);

		// render some things here
		render(vg);
		// finish rendering

		nvgEndFrame(vg);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	nvgDeleteGL2(vg);

	glfwTerminate();
	return 0;
}
