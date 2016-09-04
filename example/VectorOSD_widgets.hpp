#include <vector>
#include <string>

using std::string;

class logBox {

private:
  const unsigned int padding = 10;
  const unsigned int height = 400;
  const unsigned int width = 400;
  const unsigned int line_height = 20;
  unsigned int x;
  unsigned int y;

  std::vector<string> events;

  void renderLogText(NVGcontext* vg,unsigned int tmp_x,unsigned int tmp_y,const char* text) {
    nvgFontSize(vg, 20.0f);
    nvgFontFace(vg, "sans");
    nvgFillColor(vg, nvgRGBA(255,255,255,255));
    nvgTextAlign(vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
    nvgText(vg, tmp_x, tmp_y, text, NULL);
  }

  void renderLogEvents(NVGcontext* vg) {
    int events_size = events.size();
    for(int i = 0 ; i < events_size; i++) {
      unsigned int new_y = i * line_height;
      renderLogText(vg,x + padding, new_y, events[i].c_str() );
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
  };

  // destructor
  ~logBox() {

  };

  void log(const char* text) {
    string tmp_string = text;
    events.push_back(tmp_string);
  }

  void render(NVGcontext* vg) {
    renderLogBox(vg);
    renderLogEvents(vg);
  }
};
