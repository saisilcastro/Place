#ifndef WAVE_OF_H_
#define WAVE_OF_H_

#include <place/system/gear.h>

typedef struct {
	float w, h;
	float x, y;
	float angle, amplitude;
	float cursor, time;
	PixelSet color;
} WaveSet;

extern void wave_loop(GearSet *gear);
extern void wave_std(WaveSet *sine, GearSet *gear, PixelSet color);
extern void wave_sin(WaveSet *sine);
extern void wave_input(GearSet * gear, WaveSet *wave, unsigned char *select);
extern void wave_start_channel(WaveSet *channel, unsigned char max, GearSet *gear);

#endif /* WAVE_OF_H_ */
