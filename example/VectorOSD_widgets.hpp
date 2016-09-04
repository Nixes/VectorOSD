class logBox {
  void renderBootLogBox(NVGcontext* vg, unsigned int x, unsigned int y) {
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
};
