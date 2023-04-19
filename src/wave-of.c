#include <math.h>
#include <time.h>
#include <stdio.h>
#include "../include/wave-of.h"
#include <place/system/gear.h>

#define PI 3.141592654

void wave_loop(GearSet *gear) {
	PixelSet bg = { 64, 64, 64, 255 };
	unsigned char select = 0, max = 8;
	WaveSet ch[max];
	wave_start_channel(ch, max, gear);
	while (gear->on_event(NULL)) {
		gear->clear(bg);
		for (ch[0].x = 0; ch[0].x <= ch[0].w; ch[0].x++) {
			wave_input(gear, &ch[select], &select);
			unsigned char i;
			for (i = 0; i < max; i++) {
				ch[i].x = ch[0].x;
				wave_sin(&ch[i]);
				gear->put_pixel(ch[i].x, ch[i].y, ch[i].color);
				gear->show();
			}
		}
	}
}

void wave_start_channel(WaveSet *channel, unsigned char max, GearSet *gear) {
	unsigned char i, r, g, b;
	float amplitude = 1.0;
	srand(time(NULL));
	for (i = 0; i < max; i++) {
		r = 128 + (rand() % 128);
		g = 128 + (rand() % 128);
		b = 128 + (rand() % 128);
		wave_std(&channel[i], gear, pixel_rgb_start(r, g, b));
		channel[i].amplitude -= amplitude * ((i + 1) * 0.05);
	}
}

void wave_input(GearSet * gear, WaveSet *wave, unsigned char *select) {
	if (!gear)
		return;
	if (gear->key_press(KEY_UP))
		wave->cursor--;
	if (gear->key_press(KEY_DOWN))
		wave->cursor++;
	if (gear->key_press(KEY_LEFT))
		wave->time -= 0.001;
	if (gear->key_press(KEY_RIGHT))
		wave->time += 0.001;
	if (gear->key_press(KEY_LCTRL)) {
		if (gear->key_press(KEY_Z)) {
			if (gear->key_press(KEY_MINUS))
				wave->amplitude -= 0.005;
			if (gear->key_press(KEY_EQUALS))
				wave->amplitude += 0.005;
		}
		if (gear->key_press(KEY_S)) {
			static STATUS minus = Off;
			static STATUS plus = Off;
			if (gear->key_press(KEY_MINUS))
				minus = On;
			if (gear->key_press(KEY_EQUALS))
				plus = On;
			if (!gear->key_press(KEY_MINUS) && minus) {
				*select -= 1;
				minus = Off;
			}
			if (!gear->key_press(KEY_EQUALS) && plus) {
				*select += 1;
				plus = Off;
			}

		}
	}
	gear->on_event(NULL);
}

void wave_sin(WaveSet * wave) {
	if (!wave)
		return;
	wave->angle = (wave->x / (wave->w * wave->time)) * 360;
	wave->y = wave->cursor
			+ (sin(wave->angle * PI / 180) * wave->h * wave->amplitude);
}

void wave_std(WaveSet *sine, GearSet *gear, PixelSet color) {
	if (!sine)
		return;
	sine->w = gear->w;
	sine->h = gear->h;
	sine->x = 0;
	sine->y = 0;
	sine->cursor = sine->h * 0.5;
	sine->amplitude = 0.5;
	sine->angle = 0;
	sine->time = 0.125;
	sine->color = color;
}
