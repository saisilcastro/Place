#include <place/system/gear.h>
#include <place/plugin/allegro-plugin.h>

GearSet local, *gear = &local;

static void gear_zero(GearSet *set);

GearSet gear_standard(SystemSet up) {
	gear_zero(gear);
	gear->up[0] = up;
	return *gear;
}

#include <stdio.h>

static void gear_itoa(long long number, unsigned char base);

static void gear_print(const char *str, ...) {
	unsigned short pos = 0;
	va_list symbol;
	va_start(symbol, str);
	while (*(str + pos)) {
		if (*(str + pos) == '%') {
			pos++;
			if (*(str + pos) == 'x')
				gear_itoa(va_arg(symbol, int), 16);
			if (*(str + pos) == 'd' || *(str + pos) == 'i') {
				gear_itoa(va_arg(symbol, int), 10);
			}
			if (*(str + pos) == 'l' && *(str + pos + 1) == 'l'
					&& *(str + pos + 2) == 'i') {
				pos += 3;
				gear_itoa(va_arg(symbol, long long), 10);
			}
			if (*(str + pos) == 'o')
				gear_itoa(va_arg(symbol, int), 8);
			if (*(str + pos) == 'b')
				gear_itoa(va_arg(symbol, int), 2);
		} else {
			char c = *(str + pos);
			write(1, &c, 1);
		}
		pos++;
	}
	va_end(symbol);
}

static void gear_event_on(MACHINE_EVENT ev) {
	BIT_ON(gear->event, ev);
}

static void gear_event_off(MACHINE_EVENT ev) {
	BIT_OFF(gear->event, ev);
}

static STATUS gear_is_event(MACHINE_EVENT ev) {
	return BIT_IS_ON(gear->event, ev);
}

static void gear_mouse_press(MACHINE_MOUSE_BUTTON bt) {
	BIT_ON(gear->mouse->button, bt);
}

static void gear_mouse_release(MACHINE_MOUSE_BUTTON bt) {
	BIT_OFF(gear->mouse->button, bt);
}

static STATUS gear_mouse_is_pressed(MACHINE_MOUSE_BUTTON bt) {
	return BIT_IS_ON(gear->mouse->button, bt);
}

static void gear_mouse_wheel_on(MACHINE_MOUSE_WHEEL wheel) {
	BIT_ON(gear->mouse->wheel, wheel);
}

static void gear_mouse_wheel_off(MACHINE_MOUSE_WHEEL wheel) {
	BIT_OFF(gear->mouse->wheel, wheel);
}

static STATUS gear_mouse_wheel_is_on(MACHINE_MOUSE_WHEEL wheel) {
	return BIT_IS_ON(gear->mouse->wheel, wheel);
}

static STATUS gear_start(void) {
	if (!gear)
		return On;
	switch (gear->up->system) {
	case SYSTEM_PRIMARY: {
		break;
	}
	case SYSTEM_ALLEGRO: {
		allegro_plugin_start();
		break;
	}
	case SYSTEM_SDL: {
		break;
	}
	case SYSTEM_CONSOLE:
	default: {
		break;
	}
	}
	return Off;
}

static STATUS gear_event(vf2d * scroll) {
	if (!gear)
		return Off;
	switch (gear->up->system) {
	case SYSTEM_ALLEGRO:
		return allegro_plugin_event(scroll);
		break;
	case SYSTEM_SDL:
		break;
	case SYSTEM_PRIMARY:
		break;
	case SYSTEM_CONSOLE:
	default:
		break;
	}
	return Off;
}

static void gear_clear(PixelSet p) {
	if (!gear)
		return;
	switch (gear->up->system) {
	case SYSTEM_MAIN_NONE:
	default:
		return;
	case SYSTEM_PRIMARY:
#if (defined(WIN32) || defined(WINDOWS_XP))
		//windows_clear(&p, bDepth);
#endif
		break;
	case SYSTEM_ALLEGRO:
		allegro_plugin_clear_up(&p);
		break;
	case SYSTEM_SDL: {
		//sdl_plugin_clear(&p);
	}
		break;
	}
}

static void gear_show(void) {
	if (!gear)
		return;
	switch (gear->up->system) {
	case SYSTEM_MAIN_NONE:
	default:
		break;
	case SYSTEM_PRIMARY:
#if (defined(WIN32) || defined(WINDOWS_XP))
		//windows_show();
#else
#endif
		break;
	case SYSTEM_ALLEGRO:
		allegro_plugin_show();
		break;
	case SYSTEM_SDL:
		//sdl_plugin_show();
		break;
	}
}

static STATUS gear_key_press(KEYBOARD_MAP map) {
	if (!gear)
		return Off;
	switch (gear->up->system) {
	case SYSTEM_PRIMARY:
		break;
	case SYSTEM_ALLEGRO:
		return allegro_plugin_key(map);
	case SYSTEM_SDL:
		break;
	case SYSTEM_CONSOLE:
	default:
		break;
	}
	return Off;
}

static void gear_put_pixel(int x, int y, PixelSet color) {
	if (!gear)
		return;
	switch (gear->up->system) {
	case SYSTEM_PRIMARY:
		break;
	case SYSTEM_ALLEGRO:
		al_put_pixel(x, y, al_map_rgba(color.r, color.g, color.b, color.a));
		break;
	case SYSTEM_SDL:
		break;
	case SYSTEM_CONSOLE:
	default:
		break;
	}
}

static void gear_put(GearSet *set){
	gear = set;
}

static void gear_function(GearSet *set) {
	if (!set)
		return;
	set->put = gear_put;
	set->start = gear_start;
	set->on_event = gear_event;
	set->key_press = gear_key_press;
	set->clear = gear_clear;
	set->print = gear_print;
	set->event_on = gear_event_on;
	set->event_off = gear_event_off;
	set->is_event = gear_is_event;
	set->mouse_press = gear_mouse_press;
	set->mouse_release = gear_mouse_release;
	set->mouse_is_pressed = gear_mouse_is_pressed;
	set->wheel_on = gear_mouse_wheel_on;
	set->wheel_off = gear_mouse_wheel_off;
	set->wheel_is_on = gear_mouse_wheel_is_on;
	set->put_pixel = gear_put_pixel;
	set->show = gear_show;
}

static void gear_zero(GearSet *set) {
	if (!set)
		return;
	gear_function(set);
	set->up->system = SYSTEM_CONSOLE;
	set->up->driver = GRAPHIC_CONSOLE;
	set->up->audio = SYSTEM_AUDIO_NONE;
	set->up->network = SYSTEM_NETWORK_NONE;
	set->title = "Gear";
	set->w = WINDOW_WIDTH;
	set->h = WINDOW_HEIGHT;
	set->x = WINDOW_LEFT_CENTER;
	set->y = WINDOW_TOP_CENTER;
	set->event = 0x001;
}

static void gear_itoa(long long number, unsigned char base) {
	char multiply = 1;
	if (number > -10 && number < 0)
		write(1, "-", 1);
	if (number <= -base || number >= base) {
		gear_itoa(number / base, base);
		number %= base;
	}
	if (number < 0)
		multiply = -1;
	char c =
			number * multiply < 10 ?
					(number * multiply) + 48 : (number * multiply) + 87;
	write(1, &c, 1);
}
