#include <vector>
#include <string>

using std::string;

// TODO OPTIMISATION: streamline branching
double animationAmount(double tmp_current_time,double tmp_total_time) {
  double animation_amount =  tmp_current_time / tmp_total_time ;
  //printf("Anim amount: %f from: tmp_total_time %f / tmp_current_time %f \n",animation_amount,,tmp_current_time);
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
  const unsigned int height = 400;
  const unsigned int width = 400;
  const unsigned int line_height = 20;
  unsigned int x;
  unsigned int y;

  const double anim_line_time = 1.00000; // how long the animation should take in seconds
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
        printf("Anim amount: %f\n",animationAmount(anim_line_current_time,anim_line_time));
        printf("Fade was: %u\n",fade);
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

public:
};

// shows attitude
class attitudeIndicator {
private:
  const unsigned int height = 400;
  const unsigned int width = 600;

  float roll,pitch,yaw;

  renderAngleLine() {

  }
  renderBorder() {

  }

public:
  attitudeIndicator() {

  }
  ~attitudeIndicator() {

  }

  void render() {

  }

  void update() {

  }
};

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
