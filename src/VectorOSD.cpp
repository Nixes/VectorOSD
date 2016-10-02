//#define USING_GLES
#define SERIAL_ENABLED

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
// some building conditionals based on if we are using opengles or opengl
#ifdef USING_GLES
	#define NANOVG_GLES2_IMPLEMENTATION
	#define GLFW_INCLUDE_ES2
#else
	#define NANOVG_GL2_IMPLEMENTATION
#endif

#include <GLFW/glfw3.h>
#include "nanovg.h"
#include "nanovg_gl.h"
#include "perf.h"
// end nanovg dependencies

// start VectorOSD specific dependencies
#include "VectorOSD_widgets.hpp"
#include <libserialport.h> // cross platform serial port lib
#include "common/mavlink.h" // mavlink headers
// end VectorOSD specific dependencies


// glyphs used in ui
struct NSVGimage* glyph_power = NULL;
struct NSVGimage* glyph_gps = NULL;
struct NSVGimage* glyph_armed = NULL;

// initialise widgets
logBox log_box(300,0);
powerStats power_stats(50,550);
attitudeIndicator attitude_indicator(300,50);
bearingIndicator bearing_indicator(300,10);

// provide global serial object
struct sp_port *port;

void errorcb(int error, const char* desc) {
	printf("GLFW error %d: %s\n", error, desc);
}

int blowup = 0;
int premult = 0;


void readMav() {
  mavlink_message_t msg;
  mavlink_status_t status;
  while ( sp_input_waiting(port) > 0) {
    //printf("Bytes waiting %i\n", bytes_waiting);
    char byte_buff[1];
    if ( sp_nonblocking_read(port,byte_buff,1) ) {
      if(mavlink_parse_char(MAVLINK_COMM_1, byte_buff[0], &msg, &status)) {
        printf("  Found mavlink packet\n");
        // determine type of message
        switch(msg.msgid) {
          case MAVLINK_MSG_ID_HEARTBEAT: {
            printf("   Got Heartbeat\n");
          }
          break;

          case MAVLINK_MSG_ID_VFR_HUD:
            printf("   Got VFR HUD\n");
            mavlink_vfr_hud_t vfr_packet;
            mavlink_msg_vfr_hud_decode(&msg, &vfr_packet);
            printf("Current heading %i\n",vfr_packet.heading);
						bearing_indicator.update(vfr_packet.heading);
          break;

          case MAVLINK_MSG_ID_ATTITUDE:
            printf("   Got Attitude\n");
            mavlink_attitude_t attitude_packet;
            mavlink_msg_attitude_decode(&msg, &attitude_packet);
            printf("Pitch %f, Yaw %f, Roll %f\n", attitude_packet.pitch, attitude_packet.yaw, attitude_packet.roll);
						attitude_indicator.update(attitude_packet.roll,attitude_packet.pitch);
          break;

          default:
            printf("   Got unknown %d",msg.msgid);
          	// Do nothing
          break;
        }
      }
    }
  }
}

// request a faster update rate for attitude from the flight controller
void requestFastUpdate() {
	mavlink_message_t msg;
	mavlink_message_interval_t interval;

	interval.interval_us = 16666; // a ~60hz update rate in microseconds 16666
	interval.message_id = MAVLINK_MSG_ID_ATTITUDE;

	mavlink_msg_message_interval_encode(255, 200, &msg, &interval);

	uint8_t buf[256];
	int buf_size = mavlink_msg_to_send_buffer(buf, &msg);
	//p_sensorsPort->write_message(command);
	sp_blocking_write(port,buf,buf_size,1000);
}

bool openSerialPort(char* port_name) {
  printf("Opening port '%s' \n", port_name);
  sp_return error = sp_get_port_by_name(port_name,&port);
  if (error == SP_OK) {
    sp_set_baudrate(port,57600);
    error = sp_open(port,SP_MODE_READ_WRITE);
    if (error == SP_OK) {
      return true;
    } else {
      printf("Error opening serial device\n");
      return false;
    }
  } else {
    printf("Error finding serial device\n");
    return false;
  }
}

void drawGlyph(NVGcontext* vg, NSVGimage* image) {
	NSVGshape* shape;
	NSVGpath* path;

	printf("Rendering glyph\n");
	for (shape = image->shapes; shape != NULL; shape = shape->next) {
			//nvgStrokeWidth(vg, shape->strokeWidth);
			printf("  Rendering shape\n");
	    for (path = shape->paths; path != NULL; path = path->next) {
					printf("   Rendering path\n");
	        for (int i = 0; i < path->npts-1; i += 3) {
							//nvgBeginPath(vg);
			        //nvgMoveTo(vg, path->pts[0], path->pts[1]);
			        //for (i = 0; i < path->npts-1; i += 3) {
									printf("   	Rendering point\n");
			            float* p = &path->pts[i*2];
			            nvgBeginPath(vg);
			            nvgBezierTo(vg, p[2], p[3], p[4], p[5], p[6], p[7]);
			        //}
			        /*if (path->closed)
			        	nvgLineTo(vg, path->pts[0], path->pts[1]);*/
			        //nvgStroke(vg);
							nvgStrokeColor(vg, nvgRGBA(255,255,0,32));
							nvgStrokeWidth(vg, 3.0f);
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
	nvgCreateFont(vg, "sans", "../src/Roboto-Regular.ttf");
	// load glyphs
	glyph_power = loadGlyph("../glyphs/nano.svg");
}

void unloadAssets() {
	// unload glyphs
	nsvgDelete(glyph_power);
}

static void debugKeys(GLFWwindow* window, int key, int scancode, int action, int mods) {
	NVG_NOTUSED(scancode);
	NVG_NOTUSED(mods);
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {}
	if (key == GLFW_KEY_S && action == GLFW_PRESS) {}
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {}
	if (key == GLFW_KEY_L && action == GLFW_PRESS) {
		log_box.log("A new log element");
	}
	// battery bar debug keys
	if (key == GLFW_KEY_B && action == GLFW_PRESS) {
		power_stats.update(10.9);
	}
	if (key == GLFW_KEY_N && action == GLFW_PRESS) {
		power_stats.update(11.2);
	}
	if (key == GLFW_KEY_M && action == GLFW_PRESS) {
		power_stats.update(12.1);
	}

	// attitude debug keys
	if (key == GLFW_KEY_LEFT && action != GLFW_RELEASE) {
		attitude_indicator.updateDelta(-0.01,0);
	}
	if (key == GLFW_KEY_RIGHT && action != GLFW_RELEASE) {
		attitude_indicator.updateDelta(0.01,0);
	}
	if (key == GLFW_KEY_UP && action != GLFW_RELEASE) {
		attitude_indicator.updateDelta(0,0.01);
	}
	if (key == GLFW_KEY_DOWN && action != GLFW_RELEASE) {
		attitude_indicator.updateDelta(0,-0.01);
	}

	// compass debug keys
	if (key == GLFW_KEY_Q && action != GLFW_RELEASE) {
		bearing_indicator.updateDelta(-0.5);
	}
	if (key == GLFW_KEY_E && action != GLFW_RELEASE) {
		bearing_indicator.updateDelta(0.5);
	}
}

// actually render the objects
void render(NVGcontext* vg, double delta_time) {
	//printf("Delta_time: %f\n",delta_time );
	log_box.render(vg,delta_time);
	power_stats.render(vg,delta_time);
	bearing_indicator.render(vg);
	//drawGlyph(vg,glyph_power);

	attitude_indicator.render(vg,delta_time); // attitude indicator must be the last thing to render as it does some complex transformations
}

void init() {
	log_box.log("Initialising");
	log_box.log("Adding Assets");
}

int main(int argc, char* args[] ) {
	#ifdef SERIAL_ENABLED
		if (argc < 2) {
			printf("You must specify a serial port to obtain telemetry data\n");
			exit(1);
		}

		if (!openSerialPort(args[1]) ) {
			exit(1);
		} else {
			printf("Successfully connected to serial device\n");
		}
		requestFastUpdate();
	#endif

	GLFWwindow* window;
	NVGcontext* vg = NULL;
	PerfGraph fps;
	double previous_time = 0;

	int glfw_init_error = glfwInit();
	if (!glfw_init_error) {
		printf("Failed to init GLFW.");
		printf(" Init returned: %i.\n", glfw_init_error);
		return -1;
	}

	initGraph(&fps, GRAPH_RENDER_FPS, "Frame Time");

	glfwSetErrorCallback(errorcb);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);


	window = glfwCreateWindow(1000, 600, "VectorOSD", NULL, NULL);
	//	window = glfwCreateWindow(1000, 600, "NanoVG", glfwGetPrimaryMonitor(), NULL); // this creates a full screen window
	if (!window) {
		glfwTerminate();
		printf("Failed to create window.\n");
		return -1;
	}

	glfwSetKeyCallback(window, debugKeys);
	glfwMakeContextCurrent(window);

	#ifdef NANOVG_GLEW
	  if(glewInit() != GLEW_OK) {
			printf("Could not init glew.\n");
			return -1;
		}
	#endif

	#ifdef USING_GLES
		vg = nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
	#else
		vg = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
	#endif
	if (vg == NULL) {
		printf("Could not init nanovg.\n");
		return -1;
	}

	glfwSwapInterval(1); // this changes vsync settings, to turn vsync off set to 0

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
			renderGraph(vg, 5,5, &fps);
			render(vg,delta_time);
			// finish rendering
		nvgEndFrame(vg);

		glfwSwapBuffers(window);
		glfwPollEvents();

		#ifdef SERIAL_ENABLED
		readMav();
		#endif
	}

	sp_close(port); // close serial port

	unloadAssets();

	#ifdef USING_GLES
		nvgDeleteGLES2(vg);
	#else
		nvgDeleteGL2(vg);
	#endif

	glfwTerminate();
	return 0;
}
