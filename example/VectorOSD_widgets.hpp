#include <vector>
#include <string>
#include <math.h>

using std::string;


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
  }

  void render(NVGcontext* vg, double delta_time) {
    if (anim_line_current_time < anim_line_time) {
      anim_line_current_time += delta_time;
    }
    renderLogBox(vg);
    renderLogEvents(vg);
  }
};

class powerStats {
private:
  // battery states
  enum battery_states {normal, warning, severe};
  battery_states battery_state;

  // battery state colours
  const NVGcolor normal_colour = nvgRGBA(255,255,255,255);
  const NVGcolor warning_colour = nvgRGBA(255,255,150,255);
  const NVGcolor severe_colour = nvgRGBA(255,150,150,255);

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
    nvgStrokeColor(vg, nvgRGBA(255,255,255,255));
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
    printf("DEBUG: current_anim_time %f anim_time %f\n", current_anim_time, anim_time);
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
  const unsigned int height = 300;
  const unsigned int width = 400;

  float roll,pitch,yaw;

  void renderAngleLine(NVGcontext* vg) {
    unsigned int start_x = x + x / 2;
    unsigned int start_y = y + y / 2;

    unsigned int line_length = 100;

    unsigned int end_x = start_x + line_length * sin(roll);
    unsigned int end_y = start_y + line_length * sin(roll);

    printf("end_x %d end_y %d\n",end_x,end_y );

    nvgBeginPath(vg);
    nvgMoveTo(vg,start_x,start_y);
    nvgLineTo(vg,end_x,end_y);
    nvgFillColor(vg, nvgRGBA(255,255,255,100));
    nvgFill(vg);
  }
  void renderBorder(NVGcontext* vg) {
    nvgBeginPath(vg);
    nvgRect(vg, x,y, width, height);
    nvgStrokeColor(vg, nvgRGBA(255,255,255,255));
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);
  }

public:
  attitudeIndicator(unsigned int temp_x,unsigned int temp_y) {
    x = temp_x;
    y = temp_y;
    roll = 1;
  }
  ~attitudeIndicator() {

  }

  void render(NVGcontext* vg) {
    renderBorder(vg);
    renderAngleLine(vg);
  }

  void update(float delta_roll) {
    roll+= delta_roll;
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
