#include <vector>
#include <string>

using std::string;

class logBox {

private:
  const unsigned int padding = 10;
  unsigned int x;
  unsigned int y;

  std::vector<string>;


  void renderLogBox(NVGcontext* vg) {
  	const unsigned int height = 400;
  	const unsigned int width = 400;
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

  void render(NVGcontext* vg) {
    renderLogBox(vg);

  }
};
