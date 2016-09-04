#include <stdio.h>

// start svg dependencies
#include <string.h>
#include <math.h>
#define NANOSVG_IMPLEMENTATION  // Expands implementation
#include "nanosvg.h"
// end svg dependencies

// start nanovg dependencies
#ifdef NANOVG_GLEW
	#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
#include "nanovg.h"
#define NANOVG_GL2_IMPLEMENTATION
#include "nanovg_gl.h"
#include "perf.h"
// end nanovg dependencies

// start VectorOSD specific dependencies
#include "VectorOSD_widgets.hpp"
// end VectorOSD specific dependencies


// glyphs used in ui
struct NSVGimage* glyph_power = NULL;
struct NSVGimage* glyph_gps = NULL;
struct NSVGimage* glyph_armed = NULL;

// initialise widgets
logBox log_box(300,0);


void errorcb(int error, const char* desc) {
	printf("GLFW error %d: %s\n", error, desc);
}

int blowup = 0;
int premult = 0;


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


void drawGlyph(NVGcontext* vg, NSVGimage* image) {
	NSVGshape* shape;
	NSVGpath* path;

	// <<<< start experimental stuff
	int width = 200, height = 200; // the desired width and height for the final image

	//glViewport(0, 0, width, height);

	float view[4], cx, cy, hw, hh, aspect, px;
	cx = image->width*0.5f;
	cy = image->height*0.5f;
	hw = image->width*0.5f;
	hh = image->height*0.5f;


	if (width/hw < height/hh) {
		aspect = (float)height / (float)width;
		view[0] = cx - hw * 1.2f;
		view[2] = cx + hw * 1.2f;
		view[1] = cy - hw * 1.2f * aspect;
		view[3] = cy + hw * 1.2f * aspect;
	} else {
		aspect = (float)width / (float)height;
		view[0] = cx - hh * 1.2f * aspect;
		view[2] = cx + hh * 1.2f * aspect;
		view[1] = cy - hh * 1.2f;
		view[3] = cy + hh * 1.2f;
	}
	// Size of one pixel.
	px = (view[2] - view[1]) / (float)width;

	glOrtho(view[0], view[2], view[3], view[1], -1, 1);

	// draw bounding box
	glColor4ub(0,0,0,64);
	glBegin(GL_LINE_LOOP);
	glVertex2f(0, 0);
	glVertex2f(image->width, 0);
	glVertex2f(image->width, image->height);
	glVertex2f(0, image->height);
	glEnd();

	// <<<< end experimental stuff

	printf("Rendering glyph\n");
	for (shape = image->shapes; shape != NULL; shape = shape->next) {
			nvgFillColor(vg, nvgRGBA(255,255,255,255));
			nvgStrokeColor(vg, nvgRGBA(255,255,255,255));
			nvgStrokeWidth(vg, shape->strokeWidth);
			printf("  Rendering shape\n");
	    for (path = shape->paths; path != NULL; path = path->next) {
					printf("   Rendering path\n");
	        for (int i = 0; i < path->npts-1; i += 3) {
							nvgBeginPath(vg);
			        nvgMoveTo(vg, path->pts[0], path->pts[1]);
			        for (i = 0; i < path->npts-1; i += 3) {
			            float* p = &path->pts[i*2];
			            nvgBeginPath(vg);
			            nvgBezierTo(vg, p[2], p[3], p[4], p[5], p[6], p[7]);
			        }
			        if (path->closed)
			        	nvgLineTo(vg, path->pts[0], path->pts[1]);
			        nvgStroke(vg);
	        }
	    }
	}
}

NSVGimage* loadGlyph(const char* location) {
	// good ideas here: https://github.com/memononen/nanosvg/issues/58
	// just load some mono color glyphs for use in ui
	NSVGimage* g_image = nsvgParseFromFile(location, "px", 96.0f);
	if (g_image == NULL) {
		printf("Could not open glyph: "); printf(location); printf("\n");
	}
	return g_image;
}

void loadAssets(NVGcontext* vg) {
	printf("Loading Assets...\n");

	// load font
	nvgCreateFont(vg, "sans", "../example/Roboto-Regular.ttf");
	// load glyphs
	glyph_power = loadGlyph("../glyphs/nano.svg");
}

void unloadAssets() {
	// unload glyphs
	nsvgDelete(glyph_power);
}

// actually render the objects
void render(NVGcontext* vg) {
	nvgBeginPath(vg);
	nvgRect(vg, 100,100, 120,30);
	nvgFillColor(vg, nvgRGBA(255,192,0,255));
	nvgFill(vg);

	log_box.render(vg);

	drawGlyph(vg,glyph_power);
}

void init() {
	log_box.log("Initialising");
	log_box.log("Adding Assets");
}

int main() {
	GLFWwindow* window;
	NVGcontext* vg = NULL;
	PerfGraph fps;
	double previous_time = 0;

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
	previous_time = glfwGetTime();

	loadAssets(vg);
	init();

	while (!glfwWindowShouldClose(window)) {
		double mx, my, current_time, delta_time;
		int winWidth, winHeight;
		int fbWidth, fbHeight;
		float pxRatio;

		current_time = glfwGetTime();
		delta_time = current_time - previous_time;
		previous_time = current_time;
		updateGraph(&fps, delta_time);

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
			renderGraph(vg, 5,5, &fps);
			// finish rendering
		nvgEndFrame(vg);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	unloadAssets();
	nvgDeleteGL2(vg);

	glfwTerminate();
	return 0;
}
