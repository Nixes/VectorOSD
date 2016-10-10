#include <vector>
#include <string>
#include <math.h>

using std::string;
const long double PI = 3.141592653589793238L;

void renderArrow() {

}

// TODO OPTIMISATION: streamline branching
double animationAmount(double tmp_current_time,double tmp_total_time) {
  double animation_amount =  tmp_current_time / tmp_total_time ;
  if (tmp_current_time > tmp_total_time) {
    animation_amount = 1.0;
  } else if (animation_amount < 0.0) {
    animation_amount = 0.0;
  }
  return animation_amount;
};

unsigned char animateTransparency(double animation_amount) {
  const unsigned char start_transparency = 0;
  const unsigned char end_transparency = 255;

  unsigned char range = end_transparency - start_transparency;
  unsigned char final_transparency = range * animation_amount;

  return final_transparency;
};

void renderText(NVGcontext* vg, float x, float y,const char* text) {
  float font_size = 20.0;
	nvgFontSize(vg, font_size);
	nvgFontFace(vg, "sans");
	nvgFillColor(vg, nvgRGBA(255,255,255,255));
	nvgTextAlign(vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
	nvgText(vg, x + font_size / 6, y + font_size /2, text, NULL);
};

void renderNumber(NVGcontext* vg, float x, float y,int number) {
	char number_string[15];
	sprintf(number_string, "%d", number);

	renderText(vg, x, y, number_string);
};

float convertRadToDeg(float radians) {
  return radians * 180.0 / PI;
};

class logBox {
private:
  const unsigned int padding = 10;
  const unsigned int height = 200;
  const unsigned int width = 400;
  const unsigned int line_height = 20;
  unsigned int x;
  unsigned int y;

  const double anim_line_time = 0.50000; // how long the animation should take in seconds
  double anim_current_line_time; // how long it has been animating for

  const double anim_hide_time = 2.0; // time to wait between entrys before hiding the log box
  double anim_current_hide_time; // how long till last update

  std::vector<string> events;

  void renderLogText(NVGcontext* vg,unsigned int tmp_x,unsigned int tmp_y,const char* text,char fade) {
    nvgFontSize(vg, 20.0f);
    nvgFontFace(vg, "sans");
    nvgFillColor(vg, nvgRGBA(255,255,255,fade));
    nvgTextAlign(vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
    nvgText(vg, tmp_x, tmp_y, text, NULL);
  }

  void renderLogEvents(NVGcontext* vg) {
    int events_size = events.size();
    for(int i = 0 ; i < events_size; i++) {
      unsigned char fade = 255;
      if (i == events_size - 1) {
        fade = animateTransparency(animationAmount(anim_current_line_time,anim_line_time));
      }
      unsigned int real_line_no = events_size - i;
      unsigned int new_y = real_line_no * line_height;
      renderLogText(vg,x + padding, new_y, events[i].c_str(), fade);
      // animate last event
    }
  }

  void renderLogBox(NVGcontext* vg) {
  	nvgBeginPath(vg);

  	// left line
  	nvgMoveTo(vg,x,y);
  	nvgLineTo(vg,x,y+height);
  	// right line
  	nvgMoveTo(vg,x+width,y);
  	nvgLineTo(vg,x+width,y+height);

  	nvgFillColor(vg, nvgRGBA(255,255,255,100));
  	nvgFill(vg);
  }
public:
  // constructor
  logBox(unsigned int temp_x,unsigned int temp_y) {
    x = temp_x;
    y = temp_y;
    anim_current_line_time = 0;
  };

  // destructor
  ~logBox() {

  };

  void log(const char* text) {
    string tmp_string = text;
    events.push_back(tmp_string);
    anim_current_line_time = 0; // reset line animation
    anim_current_hide_time = 0;
  }

  void render(NVGcontext* vg, double delta_time) {
    if (anim_current_line_time < anim_line_time) {
      anim_current_line_time += delta_time;
    }
    if (anim_current_hide_time < anim_hide_time) {
      anim_current_hide_time += delta_time;
      renderLogBox(vg);
      renderLogEvents(vg);
    }
  }
};

class powerStats {
private:
  // battery states
  enum battery_states {normal, warning, severe};
  battery_states battery_state;

  // battery state colours
  const NVGcolor normal_colour = nvgRGBA(150,255,150,100);
  const NVGcolor warning_colour = nvgRGBA(255,255,150,100);
  const NVGcolor severe_colour = nvgRGBA(255,150,150,100);

  // animation variables
  const double max_anim_time = 0.5; // 500ms
  double current_anim_time;

  // battery voltages
  const float max_voltage = 12.6;
  const float min_voltage = 11.1;
  const float warning_voltage = 11.4;

  float previous_battery_amount = 0;
  float battery_amount = 0.5;

  // position
  unsigned int x;
  unsigned int y;

  // size
  const unsigned int width = 800;
  const unsigned int height = 10;


  void updateBatteryAmount(float new_battery_value) {
    // save the previous battery value
    previous_battery_amount = battery_amount;
    // update the current battery amount
    battery_amount = new_battery_value;
  }

  // battery value returned between 0.0 and 1.0
  void calculateBatteryAmount(float battery_voltage) {
    float range = max_voltage - min_voltage;
    float compensated_value =  battery_voltage - min_voltage;

    float new_battery_value = compensated_value / range;
    if (new_battery_value > 0) {
      updateBatteryAmount(new_battery_value);
    } else {
      updateBatteryAmount(0);
    }

    printf("\nDEBUG: Battery_amount %f previous_battery_amount %f\n", battery_amount, previous_battery_amount);

    // update battery state
    if (battery_voltage <= min_voltage) {
      battery_state = severe;
    } else if (battery_voltage < warning_voltage) {
      battery_state = warning;
    } else {
      battery_state = normal;
    }
  }

  void renderBatteryBar(NVGcontext* vg) {
    double difference_battery_amount = battery_amount - previous_battery_amount ;
    double animated_battery_amount = previous_battery_amount + (difference_battery_amount * animationAmount(current_anim_time,max_anim_time) );
    unsigned int calculated_width = animated_battery_amount * width;

    //printf("DEBUG: difference_battery_amount %f animated_difference %f animated_battery_amount %f calculated_width %i\n", difference_battery_amount,animated_difference, animated_battery_amount,calculated_width);

    nvgBeginPath(vg);

    nvgRect(vg, x,y, calculated_width, height);

    if (battery_state == normal) {
      nvgFillColor(vg, normal_colour);
    } else if (battery_state == warning) {
      nvgFillColor(vg, warning_colour);
    } else if (battery_state == severe) {
      nvgFillColor(vg, severe_colour);
    }
    nvgFill(vg);
  }

  void renderBatteryBox(NVGcontext* vg) {
    nvgBeginPath(vg);
    nvgRect(vg, x,y, width, height);
    nvgStrokeColor(vg, nvgRGBA(255,255,255,100));
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);
  }
public:
  powerStats(unsigned int temp_x,unsigned int temp_y) {
    x = temp_x;
    y = temp_y;
    battery_amount = 0.5;
  }
  ~powerStats() {

  }
  void render(NVGcontext* vg, double delta_time) {
    if (current_anim_time <max_anim_time ) {
      current_anim_time += delta_time;
    }
    renderBatteryBox(vg);
    renderBatteryBar(vg);
  }

  void update(float tmp_battery_voltage) {
    calculateBatteryAmount(tmp_battery_voltage);
    current_anim_time = 0; // reset animation on update
  }
};

// shows attitude
class attitudeIndicator {
private:
  // position
  unsigned int x;
  unsigned int y;

  // size
  const unsigned int height = 450;
  const unsigned int width = 400;

  // pre-translation positions
  int pre_trans_x;
  int pre_trans_y;

  // angles are all in radians
  struct str_attitude {
    float roll;
    float pitch;
    float yaw;
  };

  str_attitude previous_attitude;
  str_attitude attitude;

  // animation variables
  double max_anim_time; // in seconds
  double current_anim_time;

  // past update times, used for automatic calculation of max_anim_time
  double update_times[3];

  // settings for drawing pitch scale
  int pitch_markers_num; // how many pitch marker lines to show
  const int pitch_markers_interval = 5; // draw every 5 degrees
  const int pitch_markers_spacing = 50; // in pixels

  str_attitude interpolateAttitide() {
    float difference_roll = attitude.roll - previous_attitude.roll;
    float difference_pitch = attitude.pitch - previous_attitude.pitch;
    float difference_yaw = attitude.yaw - previous_attitude.yaw;

    str_attitude animated_attitude;
    animated_attitude.roll = previous_attitude.roll + (difference_roll * animationAmount(current_anim_time,max_anim_time) );
    animated_attitude.pitch = previous_attitude.pitch + (difference_pitch * animationAmount(current_anim_time,max_anim_time) );

    return animated_attitude;
  }

  // render current roll tag
  void renderRollText(NVGcontext* vg,float roll) {
    const int box_width = 33;
    const int angle_text_x = (pre_trans_x + width/2) - box_width;
    const int angle_text_y = pre_trans_y + height;

    const float deg_roll = convertRadToDeg(roll);
    nvgBeginPath(vg);
    nvgRect(vg, angle_text_x, angle_text_y, box_width, 20);
    nvgStrokeColor(vg, nvgRGBA(255,255,255,255));
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);

    renderNumber(vg, angle_text_x, angle_text_y, (int)deg_roll );
  }

  // render a marker line
  void renderAngleLine(NVGcontext* vg, int angle,float pitch) {
    const int line_length = 50; // in pixels

    const int middle_y = pre_trans_y + (height/2);
    const int calculated_y = middle_y + ( (convertRadToDeg(pitch)-angle) * (pitch_markers_spacing / pitch_markers_interval) );


    // first line
    nvgBeginPath(vg);
    nvgMoveTo(vg,pre_trans_x , calculated_y);
    nvgLineTo(vg,pre_trans_x + line_length, calculated_y);
    nvgFillColor(vg, nvgRGBA(255,255,255,255));
    nvgFill(vg);

    // second line
    nvgBeginPath(vg);
    nvgMoveTo(vg,pre_trans_x + width - line_length ,calculated_y);
    nvgLineTo(vg,pre_trans_x + width, calculated_y);
    nvgFillColor(vg, nvgRGBA(255,255,255,255));
    nvgFill(vg);

    switch(angle) {
      case 0:
        // render horizon line
        nvgBeginPath(vg);
        nvgMoveTo(vg,pre_trans_x , calculated_y);
        nvgLineTo(vg,pre_trans_x + width, calculated_y);
        nvgFillColor(vg, nvgRGBA(255,255,255,255));
        nvgFill(vg);
        break;
    }

    renderNumber(vg, pre_trans_x + width, calculated_y - 10, angle);
  }

  // render center line
  void renderMiddleLine(NVGcontext* vg) {
    unsigned int line_length = width;

    nvgBeginPath(vg);
    nvgMoveTo(vg,pre_trans_x,pre_trans_y + height/2);
    nvgLineTo(vg,pre_trans_x + line_length, pre_trans_y + height/2);
    nvgFillColor(vg, nvgRGBA(255,255,255,255));
    nvgFill(vg);
  }

  // round down pitch based on pitch marker interval
  int roundPitch(float pitch) {
    int pitch_deg = convertRadToDeg(pitch);
    return pitch_deg - pitch_deg % pitch_markers_interval;
  }

  // renders the marker lines for pitch
  void renderPitchLines(NVGcontext* vg,float pitch) {
    renderMiddleLine(vg);

    for (int i =0; i < pitch_markers_num; i++) {
      int tmp_deg = ( ((pitch_markers_num/2) - i ) * pitch_markers_interval) + roundPitch(pitch);
      renderAngleLine(vg,tmp_deg,pitch);
    }
  }

  void renderBorder(NVGcontext* vg) {
    nvgBeginPath(vg);
    nvgRect(vg, pre_trans_x, pre_trans_y, width, height);
    nvgStrokeColor(vg, nvgRGBA(255,255,255,100));
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);
  }

public:
  attitudeIndicator(unsigned int temp_x,unsigned int temp_y) {
    x = temp_x;
    y = temp_y;

    pre_trans_x = 0 - (width / 2);
    pre_trans_y = 0 - (height / 2);
    attitude.roll = 0;
    attitude.pitch = 0;
    attitude.yaw = 0;
    update_times[0] = 0; update_times[1] = 0; update_times[2] = 0;

    current_anim_time = 0.0;
    max_anim_time = 0.10;

    // calulate number of bearing markers required to fill the width provided
    pitch_markers_num = width / pitch_markers_spacing;
  }
  ~attitudeIndicator() {

  }

  void render(NVGcontext* vg, double delta_time) {
    if (current_anim_time <max_anim_time ) {
      current_anim_time += delta_time;
    }
    str_attitude animated_attitude = interpolateAttitide();

    // render compass before translation

    // TODO: merge translation and rotation into one operation
    nvgTranslate(vg,x + width/2, y + height/2);
    nvgRotate(vg, animated_attitude.roll);

    renderBorder(vg);
    renderRollText(vg,animated_attitude.roll);
    renderPitchLines(vg,animated_attitude.pitch);
  }

  void updateDelta(float delta_roll ,float delta_pitch) {
    attitude.roll+= delta_roll;
    attitude.pitch+= delta_pitch;
  }
  void update(float tmp_roll, float tmp_pitch) {
    previous_attitude = attitude;

    attitude.roll = tmp_roll;
    attitude.pitch = tmp_pitch;

    //max_anim_time = current_anim_time;
    current_anim_time = 0;
  }
};


class bearingIndicator {
private:
  // position
  unsigned int x;
  unsigned int y;

  // size
  const unsigned int height = 20;
  const unsigned int width = 400;

  // number of bearing markers to render
  int bearing_markers_num;
  const int bearing_marker_spacing = 15; // in pixels
  const int bearing_markers_interval = 5; // draw every 5 degrees

  // bearing
  float bearing; // in degrees, 0-359

  // converts negative values to positive bearings in the other direction
  int rollBearing(int input_bearing) {
    // -10 = 350
    // -20 = 340
    if (bearing < 0) {
      const int max_deg = 360;
      return input_bearing + max_deg;
    } else {
      return input_bearing;
    }
  }

  // marker_angle is in degrees, bearing is in radians
  void renderBearingMarker(NVGcontext* vg, int marker_angle,float bearing) {
    const int line_height = 5; // in pixels

    const float bearing_deg = bearing;
    const int middle_x = x + (width/2);
    const int calculated_x = middle_x + ( (marker_angle-bearing_deg) * ( bearing_marker_spacing / bearing_markers_interval ) );

    nvgBeginPath(vg);
    nvgMoveTo(vg,calculated_x , y);
    nvgLineTo(vg,calculated_x, y + line_height);
    nvgFillColor(vg, nvgRGBA(255,255,255,255));
    nvgFill(vg);

    /*
    Only render text if marker angle matches one of these values, just do a giant case switch
      large icons
          N = 0/360
          E = 90
          S = 180
          W = 270
      smaller icons
          NW, NE , SW, SE
    */
    const int wrapped_marker_angle = rollBearing(marker_angle);
    switch(wrapped_marker_angle) {
      case 0:
        renderText(vg, calculated_x - 10, y + line_height, "N");
        break;
      case 360:
        renderText(vg, calculated_x - 10, y + line_height, "N");
        break;

      case 90:
        renderText(vg, calculated_x - 10, y + line_height, "E");
        break;

      case 180:
        renderText(vg, calculated_x - 10, y + line_height, "S");
        break;

      case 270:
        renderText(vg, calculated_x - 10, y + line_height, "W");
        break;
    }
  }

  // render center line
  void renderMiddleLine(NVGcontext* vg) {
    unsigned int center_x = x + width/2;

    nvgBeginPath(vg);
    nvgMoveTo(vg,center_x,y);
    nvgLineTo(vg,center_x,y + height);
    nvgFillColor(vg, nvgRGBA(255,255,255,255));
    nvgFill(vg);

    renderNumber(vg,center_x-10,y + height, rollBearing( (int)bearing ) );
  }

  // round down bearing based on bearing marker interval
  int roundBearing(float bearing) {
    int bearing_deg = bearing;
    return bearing_deg - bearing_deg % bearing_markers_interval;
  }


  void renderBearingMarkers(NVGcontext* vg,float bearing) {
    renderMiddleLine(vg);

    for (int i =0; i < bearing_markers_num; i++) {
      int tmp_deg = ( ((bearing_markers_num/2) - i ) * bearing_markers_interval) + roundBearing(bearing);
      renderBearingMarker(vg,tmp_deg,bearing);
    }
  }

  void renderBorder(NVGcontext* vg) {
    nvgBeginPath(vg);
    nvgRect(vg, x, y, width, height);
    nvgStrokeColor(vg, nvgRGBA(255,255,255,100));
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);
  }
public:
  bearingIndicator(unsigned int temp_x,unsigned int temp_y) {
      x = temp_x;
      y = temp_y;
      bearing = 0;

      // calulate number of bearing markers required to fill the width provided
      bearing_markers_num = width / bearing_marker_spacing ;
  }
  void render(NVGcontext* vg) {
    renderBorder(vg);
    renderBearingMarkers(vg, bearing);
  }
  void updateDelta(float tmp_bearing) {
    float tmp_modified_bearing = bearing + tmp_bearing;
    if (tmp_modified_bearing < 360) {
      bearing = tmp_modified_bearing;
    }
  }
  // accepts bearing in degrees
  void update(float tmp_bearing) {
    if (tmp_bearing > 0 && tmp_bearing < 360) {
      bearing = tmp_bearing;
    }
  }
};

// includes mini map and waypoints
class locationSystem {
private:

public:
};


// shows overall system icons and manages boot process (including animations)
class systemStatus {
private:
  enum system_status { ok, warning, error };
  struct system {
    string name;
    system_status status;

  };
  std::vector<system> systems;

public:
  systemStatus() {

  }

  ~systemStatus() {

  }

  void render() {

  }

  void update() {

  }
};
