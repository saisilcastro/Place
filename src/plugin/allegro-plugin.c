#include <place/plugin/allegro-plugin.h>
#include <stdio.h>

STATUS allegro_debug = Off;
extern GearSet *gear;

static AllegroSet * allegro_plugin_push(void);

ALLEGRO_PLUGIN_CALL STATUS ALLEGRO_PLUGIN_TYPE allegro_plugin_start(void) {
	gear->plugin = allegro_plugin_push();
	if (!gear || !al_init())
		return Off;
	switch (gear->up->driver) {
	case GRAPHIC_PRIMARY:
	case GRAPHIC_OPENGL:
		ALL_PLUGIN(gear)->init |= ALLEGRO_OPENGL_FORWARD_COMPATIBLE;
		break;
	case GRAPHIC_DIRECTX:
		ALL_PLUGIN(gear)->init |= ALLEGRO_OPENGL_FORWARD_COMPATIBLE;
		break;
	case GRAPHIC_CONSOLE:
		ALL_PLUGIN(gear)->init |= ALLEGRO_OPENGL_FORWARD_COMPATIBLE;
		break;
	}
	if (gear->is_event(MACHINE_FULLSCREEN))
		ALL_PLUGIN(gear)->init |= ALLEGRO_FULLSCREEN;
	else {
		ALL_PLUGIN(gear)->init |= ALLEGRO_WINDOWED;
		ALL_PLUGIN(gear)->init |= ALLEGRO_RESIZABLE;
		if (gear->x == WINDOW_LEFT_CENTER || gear->y == WINDOW_TOP_CENTER)
			allegro_window_center_get();
	}
	if (!al_install_keyboard()) {
		if (allegro_debug)
			gear->print("allegro keyboard not installed\n");
		return Off;
	}
	if (!al_install_mouse())
		return Off;
	al_set_new_display_flags(ALL_PLUGIN(gear)->init);
	al_set_new_window_title((const char*) gear->title);
	if (!(ALL_PLUGIN(gear)->window = al_create_display(gear->w, gear->h))) {
		if (allegro_debug)
			printf("error creating window\n");
		return Off;
	}
	al_set_exe_name((const char*) gear->title);
	al_set_app_name((const char*) gear->title);
	al_init_user_event_source(&ALL_PLUGIN(gear)->event_fake);
	if (!(ALL_PLUGIN(gear)->event_row = al_create_event_queue()))
		return Off;
	if (!al_init_image_addon() && allegro_debug)
		printf("allegro image addon not started\n");
	if (!al_set_system_mouse_cursor(ALL_PLUGIN(gear)->window,
			ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT)) {
		printf("allegro mouse cursor not created\n");
		return Off;
	}
	al_init_font_addon();
	if (!al_init_ttf_addon() && allegro_debug)
		printf("Allegro ttf not supported\n");
	if (!al_init_primitives_addon() && allegro_debug)
		printf("Allegro primitives not supported\n");
	if (!al_install_joystick() && allegro_debug)
		printf("Allegro joystick not supported\n");
	al_reconfigure_joysticks();
	if (al_get_num_joysticks() > 0) {
		ALL_PLUGIN(gear)->joystick = al_get_joystick(
				al_get_num_joysticks() - 1);
		gear->joystick->button_max = al_get_joystick_num_buttons(
		ALL_PLUGIN(gear)->joystick) + 1;
		gear->joystick->button = (STATUS *) calloc(gear->joystick->button_max,
				sizeof(STATUS));
		al_register_event_source(ALL_PLUGIN(gear)->event_row,
				al_get_joystick_event_source());
	}
	al_register_event_source(ALL_PLUGIN(gear)->event_row,
			al_get_display_event_source(ALL_PLUGIN(gear)->window));
	al_register_event_source(ALL_PLUGIN(gear)->event_row,
			&ALL_PLUGIN(gear)->event_fake);
	al_register_event_source(ALL_PLUGIN(gear)->event_row,
			al_get_mouse_event_source());
	al_register_event_source(ALL_PLUGIN(gear)->event_row,
			al_get_keyboard_event_source());
	if (!al_init_video_addon() && allegro_debug)
		printf("allegro video addon failed\n");
	if (allegro_debug)
		printf("allegro main activated\n");
	printf("done allegro\n");
	return On;
}

STATUS allegro_key[ALLEGRO_KEY_MAX];

ALLEGRO_PLUGIN_CALL STATUS ALLEGRO_PLUGIN_TYPE allegro_plugin_event(
		vf2d * scroll) {
	if (!gear)
		return Off;
	while (al_get_next_event(ALL_PLUGIN(gear)->event_row,
			&ALL_PLUGIN(gear)->event)) {
		switch (ALL_PLUGIN(gear)->event.type) {
#if (defined(WIN32) || defined(WIN64) || defined(WINDOWS_XP))
		case ALLEGRO_EVENT_VIDEO_FRAME_SHOW:
			gear->event_on(MACHINE_VIDEO_PLAYING);
			break;
		case ALLEGRO_EVENT_VIDEO_FINISHED:
			gear->event_off(MACHINE_VIDEO_PLAYING);
			break;
#endif
		case ALLEGRO_EVENT_DISPLAY_HALT_DRAWING: {
			printf("here\n");
		}
			break;
		case ALLEGRO_EVENT_DISPLAY_RESIZE: {
			al_acknowledge_resize(ALL_PLUGIN(gear)->window);
			gear->w = ALL_PLUGIN(gear)->event.display.width;
			gear->h = ALL_PLUGIN(gear)->event.display.height;
			gear->x = ALL_PLUGIN(gear)->event.display.x;
			gear->y = ALL_PLUGIN(gear)->event.display.y;
		}
			break;
		case ALLEGRO_EVENT_DISPLAY_LOST:
		case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT: {

			if (gear->is_event(MACHINE_FOCUS_ON))
				gear->event_off(MACHINE_FOCUS_ON);
		}
			break;
		case ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY:
		case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:
		case ALLEGRO_EVENT_DISPLAY_FOUND: {
			gear->x = ALL_PLUGIN(gear)->event.display.x;
			gear->y = ALL_PLUGIN(gear)->event.display.y;
			if (!gear->is_event(MACHINE_FOCUS_ON))
				gear->event_on(MACHINE_FOCUS_ON);
		}
			break;
		case ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY: {
			if (gear->is_event(MACHINE_FOCUS_ON))
				gear->event_off(MACHINE_FOCUS_ON);
			if (gear->is_event(MACHINE_MOUSE_IN))
				gear->event_off(MACHINE_MOUSE_IN);
		}
			break;
		case ALLEGRO_EVENT_JOYSTICK_AXIS: {
			switch (ALL_PLUGIN(gear)->event.joystick.axis) {
			case 0: {
				if (ALL_PLUGIN(gear)->event.joystick.pos < 0)
					BIT_ON(gear->joystick->arrow, MACHINE_JOY_LEFT);
				else if (ALL_PLUGIN(gear)->event.joystick.pos > 0)
					BIT_ON(gear->joystick->arrow, MACHINE_JOY_RIGHT);
				else {
					BIT_OFF(gear->joystick->arrow, MACHINE_JOY_LEFT);
					BIT_OFF(gear->joystick->arrow, MACHINE_JOY_RIGHT);
				}
			}
				break;
			case 1: {
				if (ALL_PLUGIN(gear)->event.joystick.pos < 0)
					BIT_ON(gear->joystick->arrow, MACHINE_JOY_UP);
				else if (ALL_PLUGIN(gear)->event.joystick.pos > 0)
					BIT_ON(gear->joystick->arrow, MACHINE_JOY_DOWN);
				else {
					BIT_OFF(gear->joystick->arrow, MACHINE_JOY_UP);
					BIT_OFF(gear->joystick->arrow, MACHINE_JOY_DOWN);
				}
			}
				break;
			}
		}
			break;
		case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN: {
			if (ALL_PLUGIN(gear)->event.joystick.button < 12)
				gear->joystick->button[ALL_PLUGIN(gear)->event.joystick.button] =
						On;
		}
			break;
		case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP: {
			if (ALL_PLUGIN(gear)->event.joystick.button < 12)
				gear->joystick->button[ALL_PLUGIN(gear)->event.joystick.button] =
						Off;
		}
			break;
		case ALLEGRO_EVENT_MOUSE_AXES: {
			gear->mouse->x = ALL_PLUGIN(gear)->event.mouse.x
					+ (scroll ? scroll->x : 0);
			gear->mouse->y = ALL_PLUGIN(gear)->event.mouse.y
					+ (scroll ? scroll->y : 0);
			if (ALL_PLUGIN(gear)->event.mouse.z < 0)
				BIT_ON(gear->mouse->wheel, MACHINE_WHEEL_UP);
			if (ALL_PLUGIN(gear)->event.mouse.z > 0)
				BIT_ON(gear->mouse->wheel, MACHINE_WHEEL_DOWN);
			if (!gear->is_event(MACHINE_MOUSE_IN))
				gear->event_on(MACHINE_MOUSE_IN);
		}
			break;
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN: {
			switch (ALL_PLUGIN(gear)->event.mouse.button) {
			case 1:
				BIT_ON(gear->mouse->button, MACHINE_MOUSE_LEFT);
				break;
			case 2:
				BIT_ON(gear->mouse->button, MACHINE_MOUSE_RIGHT);
				break;
			case 3: {
				BIT_ON(gear->mouse->button, MACHINE_MOUSE_MIDDLE);
			}
				break;
			}
		}
			break;
		case ALLEGRO_EVENT_MOUSE_BUTTON_UP: {
			switch (ALL_PLUGIN (gear)->event.mouse.button) {
			case 1: {
				BIT_OFF(gear->mouse->button, MACHINE_MOUSE_LEFT);
			}
				break;
			case 2:
				BIT_OFF(gear->mouse->button, MACHINE_MOUSE_RIGHT);
				break;
			case 3:
				BIT_OFF(gear->mouse->button, MACHINE_MOUSE_MIDDLE);
				break;
			}
		}
			break;
		case ALLEGRO_EVENT_KEY_DOWN: {
			gear->event_on(MACHINE_KEY_PRESSED);
			allegro_key[ALL_PLUGIN(gear)->event.keyboard.keycode] = On;
		}
			break;
		case ALLEGRO_EVENT_KEY_UP: {
			gear->event_off(MACHINE_KEY_PRESSED);
			allegro_key[ALL_PLUGIN(gear)->event.keyboard.keycode] = Off;
		}
			break;
		case ALLEGRO_EVENT_DISPLAY_CLOSE: {
			gear->event_off(MACHINE_RUNNING);
		}
			break;
		}
	}
	return gear->is_event(MACHINE_RUNNING);
}

ALLEGRO_PLUGIN_CALL void ALLEGRO_PLUGIN_TYPE allegro_plugin_show(void) {
	if (!gear)
		return;
	switch (gear->up->driver) {
	case GRAPHIC_PRIMARY:
#if !defined (ALLEGRO_PLUGIN_ACTIVATED)
		al_flip_display();
#else
		if (allegro_debug) printf("allegro plugin desactivated\n");
#endif
		break;
	case GRAPHIC_OPENGL:
		break;
	case GRAPHIC_DIRECTX:
		break;
	case GRAPHIC_CONSOLE:
	default:
		break;
	}
}

ALLEGRO_PLUGIN_CALL void ALLEGRO_PLUGIN_TYPE allegro_plugin_destroy() {
	if (!gear)
		return;
#if !defined (ALLEGRO_PLUGIN_ACTIVATED)
	if (ALL_PLUGIN(gear)->window) {
		al_destroy_display(ALL_PLUGIN(gear)->window);
		ALL_PLUGIN(gear)->window = NULL;
		if (allegro_debug)
			printf("Allegro window destroyed\n");
	}
	al_destroy_user_event_source(&ALL_PLUGIN(gear)->event_fake);
	if (ALL_PLUGIN(gear)->event_row) {
		al_destroy_event_queue(ALL_PLUGIN(gear)->event_row);
		ALL_PLUGIN(gear)->event_row = NULL;
		if (allegro_debug)
			printf("Allegro event row destroyed\n");
	}
#if (defined(WIN32) || defined(WIN64) || defined(WINDOWS_XP))
	if (ALL_PLUGIN(gear)->video) {
		al_close_video(ALL_PLUGIN(gear)->video);
		ALL_PLUGIN(gear)->video = NULL;
		al_shutdown_video_addon();
	}
#endif
	al_shutdown_ttf_addon();
	al_shutdown_font_addon();
	al_shutdown_primitives_addon();
	al_shutdown_image_addon();
	al_uninstall_keyboard();
	al_uninstall_mouse();
	al_uninstall_joystick();
#endif
}

ALLEGRO_PLUGIN_CALL void ALLEGRO_PLUGIN_TYPE allegro_window_center_get(void) {
	if (!gear)
		return;
#if !defined (ALLEGRO_PLUGIN_ACTIVATED)
	if (gear->is_event(MACHINE_FULLSCREEN))
		return;
	int w_max = 0, h_max = 0;
	allegro_max_resolution(&w_max, &h_max);
	gear->x =
			(gear->x == WINDOW_LEFT_CENTER) ?
					((w_max - gear->w) * .5) - 10 : gear->x;
	gear->y =
			(gear->y == WINDOW_TOP_CENTER) ?
					((h_max - gear->h) * .5) - 33 : gear->y;
	al_set_window_position(ALL_PLUGIN(gear)->window, gear->x, gear->y);
#endif
}

ALLEGRO_PLUGIN_CALL void ALLEGRO_PLUGIN_TYPE allegro_plugin_clear_up(
		PixelSet * p) {
	if (!gear)
		return;
	switch (gear->up->driver) {
	case GRAPHIC_PRIMARY: {
#if !defined (ALLEGRO_PLUGIN_ACTIVATED)
		PixelSet a = { 255, 0, 0, 255 };
		if (!p)
			p = &a;
		al_set_target_bitmap(al_get_backbuffer(ALL_PLUGIN(gear)->window));
		al_clear_to_color(al_map_rgb(p->r, p->g, p->b));
#else
		if (allegro_debug) printf("allegro plugin desactivated\n");
#endif
	}
		break;
	case GRAPHIC_OPENGL:
		break;
	case GRAPHIC_DIRECTX:
		break;
	case GRAPHIC_CONSOLE:
	default:
		break;
	}

}

ALLEGRO_PLUGIN_CALL int ALLEGRO_PLUGIN_TYPE allegro_max_resolution(int * w,
		int * h) {
	if (!gear)
		return 0;
#if !defined (ALLEGRO_PLUGIN_ACTIVATED)
	int num = al_get_num_video_adapters();
	if (num) {
		//printf("redimensionando\n");
		ALLEGRO_MONITOR_INFO info;
		if (al_get_monitor_info(0, &info)) {
			*w = info.x2 - info.x1;
			*h = info.y2 - info.y1;
			return (num);
		}
	}
#endif
	return 0;
}

void static allegro_zero(AllegroSet * p) {
	if (!p)
		return;
#if !defined (ALLEGRO_PLUGIN_ACTIVATED)
	p->init = ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE
			| ALLEGRO_OPENGL_FORWARD_COMPATIBLE; //ALLEGRO_RESIZABLE | ALLEGRO_WINDOWED | ALLEGRO_FULLSCREEN;
	p->window = NULL;
	p->event_row = NULL;
	p->joystick = NULL;
#if (defined(WIN32) || defined(WIN64)  || defined(WINDOWS_XP))
	p->video = NULL;
#endif
#endif
}

static AllegroSet * allegro_plugin_push(void) {
	AllegroSet * p = (AllegroSet *) malloc(sizeof(AllegroSet));
	allegro_zero(p);
	return p;
}

static int res = 0;

ALLEGRO_PLUGIN_CALL void ALLEGRO_PLUGIN_TYPE allegro_res_min() {
	res = 0;
}

ALLEGRO_PLUGIN_CALL void ALLEGRO_PLUGIN_TYPE allegro_res_max() {
	if (!gear)
		return;
#if defined (ALLEGRO_PLUGIN_ACTIVATED)
	res = al_get_num_display_modes () - 1;
#endif
}

ALLEGRO_PLUGIN_CALL STATUS ALLEGRO_PLUGIN_TYPE allegro_res_preview(int * w,
		int * h) {
	if (!gear)
		return Off;
#if defined (ALLEGRO_PLUGIN_ACTIVATED)
	ALLEGRO_DISPLAY_MODE info;
	if (al_get_display_mode (res--, &info) != NULL) {
		*w = info.width;
		*h = info.height;
	}
	res = al_get_num_display_modes () - 1;
	al_get_display_mode (res, &info);
	*w = info.width;
	*h = info.height;
#endif
	return Off;
}

ALLEGRO_PLUGIN_CALL STATUS ALLEGRO_PLUGIN_TYPE allegro_res_next(int * w,
		int * h) {
	if (!gear)
		return Off;
#if defined (ALLEGRO_PLUGIN_ACTIVATED)
	ALLEGRO_DISPLAY_MODE info;
	if (al_get_display_mode (res++, &info) != NULL) {
		*w = info.width;
		*h = info.height;
	}
	res = 0;
	al_get_display_mode (res, &info);
	*w = info.width;
	*h = info.height;
#endif
	return Off;
}

ALLEGRO_PLUGIN_CALL void ALLEGRO_PLUGIN_TYPE allegro_window_size_set() {
	if (!gear)
		return;
#if defined (ALLEGRO_PLUGIN_ACTIVATED)
	al_resize_display (ALL_PLUGIN(machine)->window, machine->w, machine->h);
#endif
}

int key_to_allegro(KEYBOARD_MAP map) {
	switch (map) {
	case KEY_A: {
		return (ALLEGRO_KEY_A);
	}
		break;
	case KEY_B: {
		return (ALLEGRO_KEY_B);
	}
		break;
	case KEY_C: {
		return (ALLEGRO_KEY_C);
	}
		break;
	case KEY_D: {
		return (ALLEGRO_KEY_D);
	}
		break;
	case KEY_E: {
		return (ALLEGRO_KEY_E);
	}
		break;
	case KEY_F: {
		return (ALLEGRO_KEY_F);
	}
		break;
	case KEY_G: {
		return (ALLEGRO_KEY_G);
	}
		break;
	case KEY_H: {
		return (ALLEGRO_KEY_H);
	}
		break;
	case KEY_I: {
		return (ALLEGRO_KEY_I);
	}
		break;
	case KEY_J: {
		return (ALLEGRO_KEY_J);
	}
		break;
	case KEY_K: {
		return (ALLEGRO_KEY_K);
	}
		break;
	case KEY_L: {
		return (ALLEGRO_KEY_L);
	}
		break;
	case KEY_M: {
		return (ALLEGRO_KEY_M);
	}
		break;
	case KEY_N: {
		return (ALLEGRO_KEY_N);
	}
		break;
	case KEY_O: {
		return (ALLEGRO_KEY_O);
	}
		break;
	case KEY_P: {
		return (ALLEGRO_KEY_P);
	}
		break;
	case KEY_Q: {
		return (ALLEGRO_KEY_Q);
	}
		break;
	case KEY_R: {
		return (ALLEGRO_KEY_R);
	}
		break;
	case KEY_S: {
		return (ALLEGRO_KEY_S);
	}
		break;
	case KEY_T: {
		return (ALLEGRO_KEY_T);
	}
		break;
	case KEY_U: {
		return (ALLEGRO_KEY_U);
	}
		break;
	case KEY_V: {
		return (ALLEGRO_KEY_V);
	}
		break;
	case KEY_W: {
		return (ALLEGRO_KEY_W);
	}
		break;
	case KEY_X: {
		return (ALLEGRO_KEY_X);
	}
		break;
	case KEY_Y: {
		return (ALLEGRO_KEY_Y);
	}
		break;
	case KEY_Z: {
		return (ALLEGRO_KEY_Z);
	}
		break;
	case KEY_0: {
		return (ALLEGRO_KEY_0);
	}
		break;
	case KEY_1: {
		return (ALLEGRO_KEY_1);
	}
		break;
	case KEY_2: {
		return (ALLEGRO_KEY_2);
	}
		break;
	case KEY_3: {
		return (ALLEGRO_KEY_3);
	}
		break;
	case KEY_4: {
		return (ALLEGRO_KEY_4);
	}
		break;
	case KEY_5: {
		return (ALLEGRO_KEY_5);
	}
		break;
	case KEY_6: {
		return (ALLEGRO_KEY_6);
	}
		break;
	case KEY_7: {
		return (ALLEGRO_KEY_7);
	}
		break;
	case KEY_8: {
		return (ALLEGRO_KEY_8);
	}
		break;
	case KEY_9: {
		return (ALLEGRO_KEY_9);
	}
		break;
	case KEY_PAD_0: {
		return (ALLEGRO_KEY_PAD_0);
	}
		break;
	case KEY_PAD_1: {
		return (ALLEGRO_KEY_PAD_1);
	}
		break;
	case KEY_PAD_2: {
		return (ALLEGRO_KEY_PAD_2);
	}
		break;
	case KEY_PAD_3: {
		return (ALLEGRO_KEY_PAD_3);
	}
		break;
	case KEY_PAD_4: {
		return (ALLEGRO_KEY_PAD_4);
	}
		break;
	case KEY_PAD_5: {
		return (ALLEGRO_KEY_PAD_5);
	}
		break;
	case KEY_PAD_6: {
		return (ALLEGRO_KEY_PAD_6);
	}
		break;
	case KEY_PAD_7: {
		return (ALLEGRO_KEY_PAD_7);
	}
		break;
	case KEY_PAD_8: {
		return (ALLEGRO_KEY_PAD_8);
	}
		break;
	case KEY_PAD_9: {
		return (ALLEGRO_KEY_PAD_9);
	}
		break;
	case KEY_F1: {
		return (ALLEGRO_KEY_F1);
	}
		break;
	case KEY_F2: {
		return (ALLEGRO_KEY_F2);
	}
		break;
	case KEY_F3: {
		return (ALLEGRO_KEY_F3);
	}
		break;
	case KEY_F4: {
		return (ALLEGRO_KEY_F4);
	}
		break;
	case KEY_F5: {
		return (ALLEGRO_KEY_F5);
	}
		break;
	case KEY_F6: {
		return (ALLEGRO_KEY_F6);
	}
		break;
	case KEY_F7: {
		return (ALLEGRO_KEY_F7);
	}
		break;
	case KEY_F8: {
		return (ALLEGRO_KEY_F8);
	}
		break;
	case KEY_F9: {
		return (ALLEGRO_KEY_F9);
	}
		break;
	case KEY_F10: {
		return (ALLEGRO_KEY_F10);
	}
		break;
	case KEY_F11: {
		return (ALLEGRO_KEY_F11);
	}
		break;
	case KEY_F12: {
		return (ALLEGRO_KEY_F12);
	}
		break;
	case KEY_ESC: {
		return (ALLEGRO_KEY_ESCAPE);
	}
		break;
	case KEY_TILDE: {
		return (ALLEGRO_KEY_TILDE);
	}
		break;
	case KEY_MINUS: {
		return (ALLEGRO_KEY_MINUS);
	}
		break;
	case KEY_EQUALS: {
		return (ALLEGRO_KEY_EQUALS);
	}
		break;
	case KEY_BACKSPACE: {
		return (ALLEGRO_KEY_BACKSPACE);
	}
		break;
	case KEY_TAB: {
		return (ALLEGRO_KEY_TAB);
	}
		break;
	case KEY_OPENBRACE: {
		return (ALLEGRO_KEY_OPENBRACE);
	}
		break;
	case KEY_CLOSEBRACE: {
		return (ALLEGRO_KEY_CLOSEBRACE);
	}
		break;
	case KEY_ENTER: {
		return (ALLEGRO_KEY_ENTER);
	}
		break;
	case KEY_SEMICOLON: {
		return (ALLEGRO_KEY_SEMICOLON);
	}
		break;
	case KEY_QUOTE: {
		return (ALLEGRO_KEY_QUOTE);
	}
		break;
	case KEY_BACKSLASH: {
		return (ALLEGRO_KEY_BACKSLASH);
	}
		break;
	case KEY_BACKSLASH2: {
		return (ALLEGRO_KEY_BACKSLASH2);
	}
		break;
	case KEY_COMMA: {
		return (ALLEGRO_KEY_COMMA);
	}
		break;
	case KEY_FULLSTOP: {
		return (ALLEGRO_KEY_FULLSTOP);
	}
		break;
	case KEY_SLASH: {
		return (ALLEGRO_KEY_SLASH);
	}
		break;
	case KEY_SPACE: {
		return (ALLEGRO_KEY_SPACE);
	}
		break;
	case KEY_INSERT: {
		return (ALLEGRO_KEY_INSERT);
	}
		break;
	case KEY_DELETE: {
		return (ALLEGRO_KEY_DELETE);
	}
		break;
	case KEY_HOME: {
		return (ALLEGRO_KEY_HOME);
	}
		break;
	case KEY_END: {
		return (ALLEGRO_KEY_END);
	}
		break;
	case KEY_PGUP: {
		return (ALLEGRO_KEY_PGUP);
	}
		break;
	case KEY_PGDN: {
		return (ALLEGRO_KEY_PGDN);
	}
		break;
	case KEY_LEFT: {
		return (ALLEGRO_KEY_LEFT);
	}
		break;
	case KEY_RIGHT: {
		return (ALLEGRO_KEY_RIGHT);
	}
		break;
	case KEY_UP: {
		return (ALLEGRO_KEY_UP);
	}
		break;
	case KEY_DOWN: {
		return (ALLEGRO_KEY_DOWN);
	}
		break;
	case KEY_PAD_SLASH: {
		return (ALLEGRO_KEY_PAD_SLASH);
	}
		break;
	case KEY_PAD_ASTERISK: {
		return (ALLEGRO_KEY_PAD_ASTERISK);
	}
		break;
	case KEY_PAD_MINUS: {
		return (ALLEGRO_KEY_PAD_MINUS);
	}
		break;
	case KEY_PAD_PLUS: {
		return (ALLEGRO_KEY_PAD_PLUS);
	}
		break;
	case KEY_PAD_DELETE: {
		return (ALLEGRO_KEY_PAD_DELETE);
	}
		break;
	case KEY_PAD_ENTER: {
		return (ALLEGRO_KEY_PAD_ENTER);
	}
		break;
	case KEY_PRINTSCREEN: {
		return (ALLEGRO_KEY_PRINTSCREEN);
	}
		break;
	case KEY_PAUSE: {
		return (ALLEGRO_KEY_PAUSE);
	}
		break;
	case KEY_ABNT_C1: {
		return (ALLEGRO_KEY_ABNT_C1);
	}
		break;
	case KEY_YEN: {
		return (ALLEGRO_KEY_YEN);
	}
		break;
	case KEY_KANA: {
		return (ALLEGRO_KEY_KANA);
	}
		break;
	case KEY_CONVERT: {
		return (ALLEGRO_KEY_CONVERT);
	}
		break;
	case KEY_NOCONVERT: {
		return (ALLEGRO_KEY_NOCONVERT);
	}
		break;
	case KEY_AT: {
		return (ALLEGRO_KEY_AT);
	}
		break;
	case KEY_CIRCUMFLEX: {
		return (ALLEGRO_KEY_CIRCUMFLEX);
	}
		break;
	case KEY_COLON2: {
		return (ALLEGRO_KEY_COLON2);
	}
		break;
	case KEY_KANJI: {
		return (ALLEGRO_KEY_KANJI);
	}
		break;
	case KEY_PAD_EQUALS: {
		return (ALLEGRO_KEY_PAD_EQUALS);
	}
		break;
	case KEY_BACKQUOTE: {
		return (ALLEGRO_KEY_BACKQUOTE);
	}
		break;
	case KEY_SEMICOLON2: {
		return (ALLEGRO_KEY_SEMICOLON2);
	}
		break;
	case KEY_COMMAND: {
		return (ALLEGRO_KEY_COMMAND);
	}
		break;
	case KEY_UNKNOWN: {
		return (ALLEGRO_KEY_UNKNOWN);
	}
		break;
		/*
		 case KEY_MODIFIERS:{
		 return(ALLEGRO_KEY_MODIFIERS);
		 }break;
		 */
	case KEY_LSHIFT: {
		return (ALLEGRO_KEY_LSHIFT);
	}
		break;
	case KEY_RSHIFT: {
		return (ALLEGRO_KEY_RSHIFT);
	}
		break;
	case KEY_LCTRL: {
		return (ALLEGRO_KEY_LCTRL);
	}
		break;
	case KEY_RCTRL: {
		return (ALLEGRO_KEY_RCTRL);
	}
		break;
	case KEY_LALT: {
		return (ALLEGRO_KEY_ALT);
	}
		break;
	case KEY_RALT: {
		return (ALLEGRO_KEY_ALTGR);
	}
		break;
	case KEY_LWIN: {
		return (ALLEGRO_KEY_LWIN);
	}
		break;
	case KEY_RWIN: {
		return (ALLEGRO_KEY_RWIN);
	}
		break;
	case KEY_MENU: {
		return (ALLEGRO_KEY_MENU);
	}
		break;
	case KEY_SCROLLLOCK: {
		return (ALLEGRO_KEY_SCROLLLOCK);
	}
		break;
	case KEY_NUMLOCK: {
		return (ALLEGRO_KEY_NUMLOCK);
	}
		break;
	case KEY_CAPSLOCK: {
		return (ALLEGRO_KEY_CAPSLOCK);
	}
		break;
	case KEY_NONE:
	case KEYBOARD_MAX:
		break;
	}
	return (0);
}

ALLEGRO_PLUGIN_CALL STATUS ALLEGRO_PLUGIN_TYPE allegro_plugin_key(
		KEYBOARD_MAP value) {
	return allegro_key[key_to_allegro(value)];
}

ALLEGRO_PLUGIN_CALL STATUS ALLEGRO_PLUGIN_TYPE allegro_title(char * title) {
	if (!gear || !title)
		return Off;
	al_set_window_title(ALL_PLUGIN(gear)->window, (const char*) title);
	return On;
}
