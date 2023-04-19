#include <place/system/graphic/pixel-of.h>

PIXEL_OF_CALL PixelSet PIXEL_OF_TYPE pixel_start(void) {
	PixelSet pixel = { 255, 255, 255, 255 };
	return pixel;
}

PIXEL_OF_CALL PixelSet PIXEL_OF_TYPE pixel_rgb_start(B8U r, B8U g, B8U b) {
	PixelSet pixel = { r, g, b, 255};
	return pixel;
}

PIXEL_OF_CALL PixelSet PIXEL_OF_TYPE pixel_rgba_start(B8U r, B8U g, B8U b, B8U a) {
	PixelSet pixel = { r, g, b, a};
	return pixel;
}

PIXEL_OF_CALL void PIXEL_OF_TYPE pixel_rgb_set(PixelSet * p, B8U r, B8U g, B8U b){
	if(!p)return;
	p->r = r;
	p->g = g;
	p->b = b;
}

PIXEL_OF_CALL void PIXEL_OF_TYPE pixel_rgba_set(PixelSet * p, B8U r, B8U g, B8U b, B8U a){
	if(!p)return;
	p->r = r;
	p->g = g;
	p->b = b;
	p->a = a;
}
