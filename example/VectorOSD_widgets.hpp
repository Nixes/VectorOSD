#include <vector>
#include <string>
#include <math.h>

using std::string;
const long double PI = 3.141592653589793238L;

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

class logBox {
private:
  const unsigned int padding = 10;
  const unsigned int height = 200;
  const unsigned int width = 400;
  const unsigned int line_height = 20;
  unsigned int x;
  unsigned int y;

  const double anim_line_time = 0.50000; // how long the animation should take in seconds
  double anim_line_current_time; // how long it has been animating for

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
        fade = animateTransparency(animationAmount(anim_line_current_time,anim_line_time));
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
    anim_line_current_time = 0;
  };

  // destructor
  ~logBox() {

  };

  void log(const char* text) {
    string tmp_string = text;
    events.push_back(tmp_string);
    anim_line_current_time = 0; // reset line animation
    anim_current_hide_time = 0;
  }

  void render(NVGcontext* vg, double delta_time) {
    if (anim_line_current_time < anim_line_time) {
      anim_line_current_time += delta_time;
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
  const NVGcolor normal_colour = nvgRGBA(255,255,255,100);
  const NVGcolor warning_colour = nvgRGBA(255,255,150,100);
  const NVGcolor severe_colour = nvgRGBA(255,150,150,100);

  // animation variables
  const double anim_time = 0.5; // 500ms
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
    if (battery_voltage < min_voltage) {
      battery_state = severe;
    } else if (battery_voltage < warning_voltage) {
      battery_state = warning;
    } else {
      battery_state = normal;
    }
  }

  void renderBatteryBar(NVGcontext* vg) {
    double difference_battery_amount = battery_amount - previous_battery_amount ;
    double animated_battery_amount = previous_battery_amount + (difference_battery_amount * animationAmount(current_anim_time,anim_time) );
    unsigned int calculated_width = animated_battery_amount * width;

    //printf("DEBUG: difference_battery_amount %f animated_difference %f animated_battery_amount %f calculated_width %i\n", difference_battery_amount,animated_difference, animated_battery_amount,calculated_width);

    nvgBeginPath(vg);

    nvgRect(vg, x,y, calculated_width, height);

    if (battery_state == normal) {
      nvgFillColor(vg, normal_colour);
    } else if (battery_state == warning) {
      nvgFillColor(vg, warning_colour);
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
    if (current_anim_time < anim_time) {
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
  float roll,pitch,yaw;

  // settings for drawing pitch scale
  const int pitch_markers_num = 10; // how many pitch marker lines to show
  const int pitch_markers_interval = 5; // draw every 5 degrees

  float convertRadToDeg(float radians) {
    return radians * 180.0 / PI;
  };

  // render current roll tag
  void renderAngleText(NVGcontext* vg) {
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
  void renderAngleLine(NVGcontext* vg, int angle) {
    const int line_length = 50; // in pixels
    const int marker_spacing = 10; // in pixels


    const int middle_y = pre_trans_y + (height/2);
    const int calculated_y = middle_y + ( (angle-convertRadToDeg(pitch) ) * marker_spacing);


    // first line
    nvgBeginPath(vg);
    nvgMoveTo(vg,pre_trans_x , calculated_y);
    nvgLineTo(vg,pre_trans_x + line_length, calculated_y);
    nvgFillColor(vg, nvgRGBA(255,255,255,100));
    nvgFill(vg);

    // second line
    nvgBeginPath(vg);
    nvgMoveTo(vg,pre_trans_x + width - line_length ,calculated_y);
    nvgLineTo(vg,pre_trans_x + width, calculated_y);
    nvgFillColor(vg, nvgRGBA(255,255,255,100));
    nvgFill(vg);

    renderNumber(vg, pre_trans_x + width, calculated_y, angle);
  }

  // render center line
  void renderMiddleLine(NVGcontext* vg) {
    unsigned int line_length = width;

    nvgBeginPath(vg);
    nvgMoveTo(vg,pre_trans_x,pre_trans_y + height/2);
    nvgLineTo(vg,pre_trans_x + line_length, pre_trans_y + height/2);
    nvgFillColor(vg, nvgRGBA(255,255,255,100));
    nvgFill(vg);
  }

  // renders the marker lines for pitch
  void renderAngleLines(NVGcontext* vg) {
    renderMiddleLine(vg);

    for (int i =0; i < pitch_markers_num; i++) {
      int tmp_deg = ( ((pitch_markers_num/2) - i ) * pitch_markers_interval) + convertRadToDeg(pitch);
      renderAngleLine(vg,tmp_deg);
    }
    // should get 25 to -25
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
    roll = 0;
    pitch = 0;
  }
  ~attitudeIndicator() {

  }

  void render(NVGcontext* vg) {
    // render compass before translation

    // TODO: merge translation and rotation into one operation
    nvgTranslate(vg,x + width/2, y + height/2);
    nvgRotate(vg, roll);

    renderBorder(vg);
    renderAngleText(vg);
    renderAngleLines(vg);
  }

  void update(float delta_roll ,float delta_pitch) {
    roll+= delta_roll;
    pitch+= delta_pitch;
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
