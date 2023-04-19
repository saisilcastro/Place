#ifdef ASSHOLE

#include <plugin/sdl-plugin.h>
#include <stdio.h>

STATUS sdl_debug = Off;
extern MachineSet * machine;

static SDLSet * sdl_plugin_set() {
#if defined (SDL_PLUGIN_ACTIVATED)
    SDLSet * sdl = (SDLSet *) malloc (sizeof(SDLSet));
    if (!sdl) return NULL;
    sdl->screen = NULL;
    sdl->pallet = NULL;
    sdl->joystick = NULL;
    sdl->driver = 0;  // SDL_WINDOW_OPENGL | SDL_WINDOW_INPUT_GRABBED ...
    sdl->init = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;  //SDL_RENDERER_PRESENTVSYNC
    return sdl;
#endif
    return NULL;
}

SDL_PLUGIN_CALL void SDL_PLUGIN_TYPE sdl_window_center_get();
SDL_PLUGIN_CALL STATUS SDL_PLUGIN_TYPE sdl_plugin_main() {
    if (!machine || !machine->up || machine->up->system != SYSTEM_SDL) return Off;
#if defined (SDL_PLUGIN_ACTIVATED)
    if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) != 0) {
	if (sdl_debug) printf ("SDL2 error %s\n", SDL_GetError ());
	return Off;
    }
    if (!(machine->plugin = sdl_plugin_set ())) return Off;
    switch (machine->up->driver) {
	case GRAPHIC_PRIMARY:
	    break;
	case GRAPHIC_OPENGL: {
	    BIT_ON(SDL_PLUGIN(machine)->driver, SDL_WINDOW_OPENGL);
	    SDL_GL_SetAttribute (SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	    SDL_GL_SetAttribute (SDL_GL_CONTEXT_MINOR_VERSION, 1);
	    SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);
	    SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, 24);
	}
	    break;
	case GRAPHIC_DIRECTX: {
	    BIT_ON(SDL_PLUGIN(machine)->driver, SDL_WINDOW_OPENGL);
	    SDL_GL_SetAttribute (SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	    SDL_GL_SetAttribute (SDL_GL_CONTEXT_MINOR_VERSION, 1);
	    SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);
	    SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, 24);
	}
	    break;
	case GRAPHIC_CONSOLE:
	    break;
    }
    if (machine->x == WINDOW_LEFT_CENTER || machine->y == WINDOW_TOP_CENTER) sdl_window_center_get ();
    if (!B16_IS_ON(machine->event, MACHINE_FULLSCREEN)) {
	BIT_ON(SDL_PLUGIN(machine)->driver, SDL_WINDOW_SHOWN);
	BIT_ON(SDL_PLUGIN(machine)->driver, SDL_WINDOW_RESIZABLE);
    }
    if (!SDL_SetHint (SDL_HINT_RENDER_SCALE_QUALITY, "1") && sdl_debug) printf ("Warning: Linear texture filtering not enabled!");
    if ((SDL_PLUGIN(machine)->screen = SDL_CreateWindow ((const char*) machine->title, machine->x, machine->y, machine->w, machine->h,
    SDL_PLUGIN(machine)->driver))) {
	switch (machine->up->driver) {
	    case GRAPHIC_PRIMARY: {
		if ((SDL_PLUGIN(machine)->pallet = SDL_CreateRenderer (SDL_PLUGIN(machine)->screen, -1, SDL_PLUGIN(machine)->init))) {
		    SDL_SetRenderDrawColor (SDL_PLUGIN(machine)->pallet, 0x00, 0x00, 0x00, 0xFF);
		    if (B16_IS_ON(machine->event, MACHINE_FULLSCREEN)) SDL_SetWindowFullscreen (SDL_PLUGIN(machine)->screen, SDL_WINDOW_FULLSCREEN);
		    else SDL_SetWindowFullscreen (SDL_PLUGIN(machine)->screen, 0);
		    if (TTF_Init () == -1 && sdl_debug) printf ("SDL TTF Error: %s\n", TTF_GetError ());
		    int img_flag = IMG_INIT_PNG & IMG_INIT_TIF & IMG_INIT_WEBP & IMG_INIT_JPG;
		    if (IMG_Init (img_flag) != img_flag && sdl_debug) printf ("SDL2 Image plugin Error: [%s]\n", IMG_GetError ());
		    if (SDL_NumJoysticks () > 0) {
			SDL_JoystickEventState (SDL_ENABLE);
			SDL_PLUGIN(machine)->joystick = SDL_JoystickOpen (0);
			if (SDL_PLUGIN(machine)->joystick) {
			    machine->joystick->button_max = SDL_JoystickNumButtons (
			    SDL_PLUGIN(machine)->joystick);
			    if (machine->joystick->button_max) machine->joystick->button = (STATUS *) calloc (machine->joystick->button_max + 1,
				    sizeof(STATUS));
			}
		    }
		    if (sdl_debug) printf ("sdl main started\n");
		    return On;
		}
	    }
		break;
	    case GRAPHIC_OPENGL: {
		if ((SDL_PLUGIN(machine)->glContext = SDL_GL_CreateContext (SDL_PLUGIN(machine)->screen))) {
		    if (B16_IS_ON(machine->event, MACHINE_VSYNC)) SDL_GL_SetSwapInterval (1);
		    /*
		     if (gl_start()) {
		     if (sdl_debug) printf("sdl main(gl) started\n");
		     return On;
		     }
		     */
		}
	    }
		break;
	    case GRAPHIC_DIRECTX:
		break;
	    case GRAPHIC_CONSOLE:
		break;
	}
    }

#else
    printf("sdl main not activated\n");
    getch();
#endif
    return Off;
}

SDL_PLUGIN_CALL STATUS SDL_PLUGIN_TYPE sdl_plugin_audio() {
    if (!machine || !machine->audio || !machine->up || machine->up->audio != SYSTEM_AUDIO_SDL) return Off;
#if defined (SDL_PLUGIN_ACTIVATED)
    machine->audio->format = MIX_DEFAULT_FORMAT;
    if ((SDL_InitSubSystem (SDL_INIT_AUDIO) == 0)
	    && Mix_OpenAudio (machine->audio->frequency, machine->audio->format, machine->audio->channel, machine->audio->chunk_size) == -1) {
	if (sdl_debug) printf ("Could not init sdl audio\n");
	return Off;
    }
    return On;
#else
    printf("sdl audio not activated\n");
    getch();
#endif
    return Off;
}

SDL_PLUGIN_CALL STATUS SDL_PLUGIN_TYPE sdl_plugin_network() {
    if (!machine || !machine->up || machine->up->network != SYSTEM_NETWORK_SDL) return Off;
#if defined (SDL_PLUGIN_ACTIVATED)
    return SDLNet_Init () == -1 ? Off : On;
#else
    printf("sdl network not activated\n");
    getch();
#endif
    return Off;
}

SDL_PLUGIN_CALL void SDL_PLUGIN_TYPE sdl_plugin_destroy() {
    if (!machine || !machine->plugin) return;
#if defined (SDL_PLUGIN_ACTIVATED)
    if (SDL_PLUGIN(machine)->screen) {
	SDL_DestroyWindow (SDL_PLUGIN(machine)->screen);
	SDL_PLUGIN(machine)->screen = NULL;

    }
    if (SDL_PLUGIN(machine)->pallet) {
	SDL_DestroyRenderer (SDL_PLUGIN(machine)->pallet);
	SDL_PLUGIN(machine)->pallet = NULL;
    }
    if (SDL_PLUGIN(machine)->joystick) {
	SDL_JoystickClose (SDL_PLUGIN(machine)->joystick);
	SDL_PLUGIN(machine)->joystick = NULL;
    }
    IMG_Quit ();
    TTF_Quit ();
    if (machine->up->audio == SYSTEM_AUDIO_SDL) {
	Mix_CloseAudio ();
	Mix_Quit ();
	if (sdl_debug) printf ("SDL2 audio destroyed\n");
    }
    if (machine->up->network == SYSTEM_NETWORK_SDL) {
	SDLNet_Quit ();
	if (sdl_debug) printf ("SDL2 net destroyed\n");
    }
    SDL_Quit ();
    if (sdl_debug) printf ("SDL2 destroyed\n");
#endif
}

SDL_PLUGIN_CALL STATUS SDL_PLUGIN_TYPE sdl_max_resolution(int * w, int * h) {
#if defined (SDL_PLUGIN_ACTIVATED)
    if (!machine || !machine->plugin) return Off;
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode (0, &dm) != 0) {
	return Off;
    } else {
	*w = dm.w;
	*h = dm.h;
    }
#endif
    return On;
}

SDL_PLUGIN_CALL void SDL_PLUGIN_TYPE sdl_window_center_get() {
#if defined (SDL_PLUGIN_ACTIVATED)
    if (!machine || !machine->plugin) return;
    int w_max = 0, h_max = 0;
    sdl_max_resolution (&w_max, &h_max);
    machine->x = machine->x == WINDOW_LEFT_CENTER ? (w_max - machine->w) * .5 : machine->x;
    machine->y = machine->y == WINDOW_TOP_CENTER ? (h_max - machine->h) * .5 : machine->y;
    SDL_SetWindowPosition (SDL_PLUGIN(machine)->screen, machine->x, machine->y);
#endif
}

#if defined (SDL_PLUGIN_ACTIVATED)
STATUS sdl_key [SDL_NUM_SCANCODES] = {
Off
};  // SDL_NUM_SCANCODES
#else
STATUS sdl_key[1];
#endif

#include <math.h>

SDL_PLUGIN_CALL STATUS SDL_PLUGIN_TYPE sdl_plugin_event(vf2d * scroll) {
    if (!machine || !machine->plugin) return Off;
#if defined(SDL_PLUGIN_ACTIVATED)
    SDL_WaitEventTimeout (&SDL_PLUGIN(machine)->event, 0);
    switch (SDL_PLUGIN(machine)->event.type) {
	case SDL_WINDOWEVENT: {
	    switch (SDL_PLUGIN(machine)->event.window.event) {
		case SDL_WINDOWEVENT_RESIZED: {
		    machine->w = SDL_PLUGIN(machine)->event.window.data1;
		    machine->h = SDL_PLUGIN(machine)->event.window.data2;
		}
		    break;
		case SDL_WINDOWEVENT_MOVED: {
		    machine->x = SDL_PLUGIN(machine)->event.window.data1;
		    machine->y = SDL_PLUGIN(machine)->event.window.data2;
		    //printf("l: %i t: %i\n", machine->left, machine->top);
		}
		    break;
		case SDL_WINDOWEVENT_SHOWN:
		case SDL_WINDOWEVENT_ENTER: {
		    if (!B16_IS_ON(machine->event, MACHINE_FOCUS_ON)) B16_ON(machine->event, MACHINE_FOCUS_ON);
		    machine->mouse->x = SDL_PLUGIN(machine)->event.motion.x;
		    machine->mouse->y = SDL_PLUGIN(machine)->event.motion.y;
		}
		    break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
		case SDL_WINDOWEVENT_NONE:
		case SDL_WINDOWEVENT_HIDDEN:
		case SDL_WINDOWEVENT_LEAVE: {
		    B16_OFF(machine->event, MACHINE_FOCUS_ON);
		}
		    break;
	    }
	}
	    return On;
	case SDL_MOUSEBUTTONDOWN: {
	    switch (SDL_PLUGIN(machine)->event.button.button) {
		case SDL_BUTTON_LEFT: {
		    B16_ON(machine->mouse->button, MACHINE_MOUSE_LEFT);
		}
		    break;
		case SDL_BUTTON_MIDDLE: {
		    B16_ON(machine->mouse->button, MACHINE_MOUSE_MIDDLE);
		}
		    break;
		case SDL_BUTTON_RIGHT: {
		    B16_ON(machine->mouse->button, MACHINE_MOUSE_RIGHT);
		}
		    break;
	    }
	}
	    break;
	case SDL_MOUSEBUTTONUP: {
	    switch (SDL_PLUGIN(machine)->event.button.button) {
		case SDL_BUTTON_LEFT: {
		    B16_OFF(machine->mouse->button, MACHINE_MOUSE_LEFT);
		}
		    break;
		case SDL_BUTTON_MIDDLE: {
		    B16_OFF(machine->mouse->button, MACHINE_MOUSE_MIDDLE);
		}
		    break;
		case SDL_BUTTON_RIGHT: {
		    B16_OFF(machine->mouse->button, MACHINE_MOUSE_RIGHT);
		}
		    break;
	    }
	}
	    break;
	case SDL_MOUSEMOTION: {
	    if (!B16_IS_ON(machine->event, MACHINE_MOUSE_IN)) B16_ON(machine->event, MACHINE_MOUSE_IN);
	    machine->mouse->x = SDL_PLUGIN(machine)->event.motion.x + (scroll ? scroll->x : 0);
	    machine->mouse->y = SDL_PLUGIN(machine)->event.motion.y + (scroll ? scroll->y : 0);
	}
	    return On;
	case SDL_MOUSEWHEEL: {
	    if (SDL_PLUGIN(machine)->event.wheel.y == -1) {
		B16_ON(machine->mouse->wheel, MACHINE_WHEEL_DOWN);
	    }
	    if (SDL_PLUGIN(machine)->event.wheel.y == 1) {
		B16_ON(machine->mouse->wheel, MACHINE_WHEEL_UP);
	    }
	}
	    return On;
	case SDL_FINGERMOTION:
	case SDL_FINGERDOWN: {
	    machine->touch->x = SDL_PLUGIN(machine)->event.tfinger.x * machine->w;
	    machine->touch->y = SDL_PLUGIN(machine)->event.tfinger.y * machine->h;
	    machine->touch->area =
	    SDL_PLUGIN(machine)->event.tfinger.pressure;
	    B16_ON(machine->touch->status, MACHINE_FINGER_PRESS);
	}
	    return On;
	case SDL_FINGERUP: {
	    B16_OFF(machine->touch->status, MACHINE_FINGER_PRESS);
	}
	    return On;
	case SDL_MULTIGESTURE: {
	    if (fabs (SDL_PLUGIN(machine)->event.mgesture.dTheta) > 3.14 / 180.0) {
		machine->touch->x = SDL_PLUGIN(machine)->event.mgesture.x * machine->w;
		machine->touch->y = SDL_PLUGIN(machine)->event.mgesture.y * machine->h;
		//currentTexture = &gRotateTexture;
	    }  //Pinch detected
	    else if (fabs (SDL_PLUGIN(machine)->event.mgesture.dDist) > 0.002) {
		machine->touch->x = SDL_PLUGIN(machine)->event.mgesture.x * machine->w;
		machine->touch->y = SDL_PLUGIN(machine)->event.mgesture.y * machine->h;

		//Pinch open
		if (SDL_PLUGIN(machine)->event.mgesture.dDist > 0) {
		    //currentTexture = &gPinchOpenTexture;
		}  //Pinch close
		else {
		    //currentTexture = &gPinchCloseTexture;
		}
	    }
	}
	    return On;
	case SDL_JOYAXISMOTION: {
	    if (SDL_PLUGIN(machine)->event.jaxis.which == 0) {
		switch (SDL_PLUGIN(machine)->event.jaxis.axis) {
		    case 0: {
			if (SDL_PLUGIN(machine)->event.jaxis.value < -8000) {
			    B16_ON(machine->joystick->arrow, MACHINE_JOY_LEFT);
			} else if (SDL_PLUGIN(machine)->event.jaxis.value > 8000) {
			    B16_ON(machine->joystick->arrow, MACHINE_JOY_RIGHT);
			} else {
			    B16_OFF(machine->joystick->arrow, MACHINE_JOY_LEFT);
			    B16_OFF(machine->joystick->arrow, MACHINE_JOY_RIGHT);
			}
		    }
			break;
		    case 4: {
			if (SDL_PLUGIN(machine)->event.jaxis.value < -8000) {
			    B16_ON(machine->joystick->arrow, MACHINE_JOY_UP);
			} else if (SDL_PLUGIN(machine)->event.jaxis.value > 8000) {
			    B16_ON(machine->joystick->arrow, MACHINE_JOY_DOWN);
			} else {
			    B16_OFF(machine->joystick->arrow, MACHINE_JOY_UP);
			    B16_OFF(machine->joystick->arrow, MACHINE_JOY_DOWN);
			}
		    }
			break;
		}
	    }
	}
	    return On;
	case SDL_JOYBUTTONDOWN: {
	    if (SDL_PLUGIN(machine)->event.jbutton.button < 12) machine->joystick->button [SDL_PLUGIN(machine)->event.jbutton.button] = On;
	}
	    return On;
	case SDL_JOYBUTTONUP: {
	    if (SDL_PLUGIN(machine)->event.jbutton.button < 12) machine->joystick->button [SDL_PLUGIN(machine)->event.jbutton.button] = Off;
	}
	    return On;
	case SDL_WINDOW_SHOWN:
	case SDL_WINDOW_MOUSE_CAPTURE: {
	    if (!B16_IS_ON(machine->event, MACHINE_FOCUS_ON)) B16_ON(machine->event, MACHINE_FOCUS_ON);
	    if (!B16_IS_ON(machine->event, MACHINE_MOUSE_IN)) B16_ON(machine->event, MACHINE_MOUSE_IN);
	}
	    return On;
	case SDL_WINDOW_HIDDEN:
	case SDL_WINDOW_MINIMIZED: {
	    if (B16_IS_ON(machine->event, MACHINE_FOCUS_ON)) B16_OFF(machine->event, MACHINE_FOCUS_ON);
	    if (B16_IS_ON(machine->event, MACHINE_MOUSE_IN)) B16_OFF(machine->event, MACHINE_MOUSE_IN);
	}
	    return On;
	case SDL_KEYUP: {
	    B16_OFF(machine->event, MACHINE_PRESSED);
	    sdl_key [SDL_PLUGIN(machine)->event.key.keysym.scancode] = Off;
	}
	    break;
	case SDL_KEYDOWN: {
	    B16_ON(machine->event, MACHINE_PRESSED);
	    sdl_key [SDL_PLUGIN(machine)->event.key.keysym.scancode] = On;
	}
	    break;
	case SDL_QUIT:
	    return Off;
    }
#endif
    return B16_IS_ON(machine->event, MACHINE_RUNNING);
}

SDL_PLUGIN_CALL void SDL_PLUGIN_TYPE sdl_title(char * title) {
    if (!machine || !machine->plugin) return;
#if defined(SDL_PLUGIN_ACTIVATED)
    SDL_SetWindowTitle (SDL_PLUGIN(machine)->screen, title);
#endif
}

int static key_to_sdl(KEYBOARD_MAP p);

SDL_PLUGIN_CALL STATUS SDL_PLUGIN_TYPE sdl_plugin_key(KEYBOARD_MAP p) {
    return (sdl_key [key_to_sdl (p)]);
}

int static key_to_sdl(KEYBOARD_MAP p) {
#if defined (SDL_PLUGIN_ACTIVATED)
    switch (p) {
	case KEY_A:
	    return SDL_SCANCODE_A;
	case KEY_B:
	    return SDL_SCANCODE_B;
	case KEY_C:
	    return SDL_SCANCODE_C;
	case KEY_D:
	    return SDL_SCANCODE_D;
	case KEY_E:
	    return SDL_SCANCODE_E;
	case KEY_F:
	    return SDL_SCANCODE_F;
	case KEY_G:
	    return SDL_SCANCODE_G;
	case KEY_H:
	    return SDL_SCANCODE_H;
	case KEY_I:
	    return SDL_SCANCODE_I;
	case KEY_J:
	    return SDL_SCANCODE_J;
	case KEY_K:
	    return SDL_SCANCODE_K;
	case KEY_L:
	    return SDL_SCANCODE_L;
	case KEY_M:
	    return SDL_SCANCODE_M;
	case KEY_N:
	    return SDL_SCANCODE_N;
	case KEY_O:
	    return SDL_SCANCODE_O;
	case KEY_P:
	    return SDL_SCANCODE_P;
	case KEY_Q:
	    return SDL_SCANCODE_Q;
	case KEY_R:
	    return SDL_SCANCODE_R;
	case KEY_S:
	    return SDL_SCANCODE_S;
	case KEY_T:
	    return SDL_SCANCODE_T;
	case KEY_U:
	    return SDL_SCANCODE_U;
	case KEY_V:
	    return SDL_SCANCODE_V;
	case KEY_W:
	    return SDL_SCANCODE_W;
	case KEY_X:
	    return SDL_SCANCODE_X;
	case KEY_Y:
	    return SDL_SCANCODE_Y;
	case KEY_Z:
	    return SDL_SCANCODE_Z;
	case KEY_0:
	    return SDL_SCANCODE_0;
	case KEY_1:
	    return SDL_SCANCODE_1;
	case KEY_2:
	    return SDL_SCANCODE_2;
	case KEY_3:
	    return SDL_SCANCODE_3;
	case KEY_4:
	    return SDL_SCANCODE_4;
	case KEY_5:
	    return SDL_SCANCODE_5;
	case KEY_6:
	    return SDL_SCANCODE_6;
	case KEY_7:
	    return SDL_SCANCODE_7;
	case KEY_8:
	    return SDL_SCANCODE_8;
	case KEY_9:
	    return SDL_SCANCODE_9;
	case KEY_PAD_0:
	    return SDL_SCANCODE_KP_0;
	case KEY_PAD_1:
	    return SDL_SCANCODE_KP_1;
	case KEY_PAD_2:
	    return SDL_SCANCODE_KP_2;
	case KEY_PAD_3:
	    return SDL_SCANCODE_KP_3;
	case KEY_PAD_4:
	    return SDL_SCANCODE_KP_4;
	case KEY_PAD_5:
	    return SDL_SCANCODE_KP_5;
	case KEY_PAD_6:
	    return SDL_SCANCODE_KP_6;
	case KEY_PAD_7:
	    return SDL_SCANCODE_KP_7;
	case KEY_PAD_8:
	    return SDL_SCANCODE_KP_8;
	case KEY_PAD_9:
	    return SDL_SCANCODE_KP_9;
	case KEY_F1: {
	    return (SDL_SCANCODE_F1);
	}
	    break;
	case KEY_F2:
	    return SDL_SCANCODE_F2;
	case KEY_F3:
	    return SDL_SCANCODE_F3;
	case KEY_F4:
	    return SDL_SCANCODE_F4;
	case KEY_F5:
	    return SDL_SCANCODE_F5;
	case KEY_F6:
	    return SDL_SCANCODE_F6;
	case KEY_F7:
	    return SDL_SCANCODE_F7;
	case KEY_F8:
	    return SDL_SCANCODE_F8;
	case KEY_F9:
	    return SDL_SCANCODE_F9;
	case KEY_F10:
	    return SDL_SCANCODE_F10;
	case KEY_F11:
	    return SDL_SCANCODE_F11;
	case KEY_F12:
	    return SDL_SCANCODE_F12;
	case KEY_ESC:
	    return SDL_SCANCODE_ESCAPE;
	case KEY_TILDE:
	    return SDL_SCANCODE_BACKSLASH;
	case KEY_MINUS:
	    return SDL_SCANCODE_MINUS;
	case KEY_EQUALS:
	    return SDL_SCANCODE_EQUALS;
	case KEY_BACKSPACE:
	    return SDL_SCANCODE_BACKSPACE;
	case KEY_TAB:
	    return SDL_SCANCODE_TAB;
	case KEY_OPENBRACE:
	    return SDL_SCANCODE_KP_LEFTBRACE;
	case KEY_CLOSEBRACE:
	    return SDL_SCANCODE_KP_RIGHTBRACE;
	case KEY_ENTER:
	    return SDL_SCANCODE_RETURN;
	case KEY_SEMICOLON:
	    return SDL_SCANCODE_SEMICOLON;
	case KEY_QUOTE:
	    return SDL_SCANCODE_APOSTROPHE;
	case KEY_BACKSLASH:
	    return SDL_SCANCODE_BACKSLASH;
	case KEY_BACKSLASH2:
	    return SDL_SCANCODE_BACKSLASH;
	case KEY_COMMA:
	    return SDL_SCANCODE_COMMA;
	case KEY_FULLSTOP:
	    return SDL_SCANCODE_AC_STOP;
	case KEY_SLASH:
	    return SDL_SCANCODE_SLASH;
	case KEY_SPACE:
	    return SDL_SCANCODE_SPACE;
	case KEY_INSERT:
	    return SDL_SCANCODE_INSERT;
	case KEY_DELETE:
	    return (SDL_SCANCODE_DELETE);
	case KEY_HOME:
	    return SDL_SCANCODE_HOME;
	case KEY_END:
	    return SDL_SCANCODE_END;
	case KEY_PGUP:
	    return SDL_SCANCODE_PAGEUP;
	case KEY_PGDN:
	    return SDL_SCANCODE_PAGEDOWN;
	case KEY_LEFT:
	    return SDL_SCANCODE_LEFT;
	case KEY_RIGHT:
	    return SDL_SCANCODE_RIGHT;
	case KEY_UP:
	    return SDL_SCANCODE_UP;
	case KEY_DOWN:
	    return SDL_SCANCODE_DOWN;
	case KEY_PAD_SLASH:
	    return SDL_SCANCODE_SLASH;
	case KEY_PAD_ASTERISK:
	    return SDL_SCANCODE_KP_MEMMULTIPLY;
	case KEY_PAD_MINUS:
	    return SDL_SCANCODE_KP_MINUS;
	case KEY_PAD_PLUS:
	    return SDL_SCANCODE_KP_PLUS;
	case KEY_PAD_DELETE:
	    return SDL_SCANCODE_KP_COMMA;
	case KEY_PAD_ENTER:
	    return SDL_SCANCODE_KP_ENTER;
	case KEY_PRINTSCREEN:
	    return SDL_SCANCODE_PRINTSCREEN;
	case KEY_PAUSE:
	    return (SDL_SCANCODE_PAUSE);
	case KEY_ABNT_C1:
	    return 0;  //SDL_SCANCODE_ABNT_C1;
	case KEY_YEN:
	    return SDL_SCANCODE_INTERNATIONAL3;
	case KEY_KANA:
	    return SDL_SCANCODE_LANG3;
	case KEY_CONVERT:
	    return SDL_SCANCODE_LANG2;
	case KEY_NOCONVERT:
	    return 0;  //SDL_SCANCODE_NOCONVERT;
	case KEY_AT:
	    return SDL_SCANCODE_KP_AT;
	case KEY_CIRCUMFLEX:
	    return SDL_SCANCODE_KP_POWER;
	case KEY_COLON2:
	    return SDL_SCANCODE_KP_COLON;
	case KEY_KANJI:
	    return 0;  //SDL_SCANCODE_KANJI;
	case KEY_PAD_EQUALS:
	    return SDL_SCANCODE_KP_EQUALS;
	case KEY_BACKQUOTE:
	    return SDL_SCANCODE_GRAVE;
	case KEY_SEMICOLON2:
	    return 0;  //SDL_SCANCODE_SEMICOLON2;
	case KEY_COMMAND:
	    return SDL_SCANCODE_LGUI;
	case KEY_UNKNOWN:
	    return SDL_SCANCODE_UNKNOWN;
	    /*
	     case KEY_MODIFIERS:{
	     //return(SDL_SCANCODE_MODIFIERS);
	     }break;*/
	case KEY_LSHIFT:
	    return SDL_SCANCODE_LSHIFT;
	case KEY_RSHIFT:
	    return SDL_SCANCODE_RSHIFT;
	case KEY_LCTRL:
	    return SDL_SCANCODE_LCTRL;
	case KEY_RCTRL:
	    return SDL_SCANCODE_RCTRL;
	case KEY_ALT:
	    return SDL_SCANCODE_LALT;
	case KEY_ALTGR:
	    return SDL_SCANCODE_RALT;
	case KEY_LWIN:
	    return SDL_SCANCODE_APPLICATION;
	case KEY_RWIN:
	    return SDL_SCANCODE_APPLICATION;
	case KEY_MENU:
	    return SDL_SCANCODE_MENU;
	case KEY_SCROLLLOCK:
	    return SDL_SCANCODE_SCROLLLOCK;
	case KEY_NUMLOCK:
	    return (SDL_SCANCODE_NUMLOCKCLEAR);
	case KEY_CAPSLOCK:
	    return SDL_SCANCODE_CAPSLOCK;
	case KEYBOARD_MAX:
	    break;
    }
#endif
    return 0;
}

SDL_PLUGIN_CALL void SDL_PLUGIN_TYPE sdl_plugin_clear(PixelSet * p) {
    if (!machine || !machine->plugin || !machine->up) return;
#if defined(SDL_PLUGIN_ACTIVATED)
    switch (machine->up->driver) {
	case GRAPHIC_PRIMARY: {
	    PixelSet a = pixel_rgb_start (0, 0, 0);
	    if (!p) p = &a;
	    SDL_SetRenderDrawColor (SDL_PLUGIN(machine)->pallet, p->r, p->g, p->b, p->a);
	    SDL_RenderClear (SDL_PLUGIN(machine)->pallet);
	}
	    break;
	case GRAPHIC_OPENGL:
	    //gl_clear(p, Off);
	    break;
	case GRAPHIC_DIRECTX:
	    break;
	case GRAPHIC_CONSOLE:
	    break;
    }
#endif
}

SDL_PLUGIN_CALL void SDL_PLUGIN_TYPE sdl_plugin_show() {
    if (!machine || !machine->plugin || !machine->up) return;
    switch (machine->up->driver) {
	case GRAPHIC_PRIMARY:
#if defined (SDL_PLUGIN_ACTIVATED)
	    SDL_RenderPresent (SDL_PLUGIN(machine)->pallet);
#else
	    if (sdl_debug) printf("sdl show not supported\n");
#endif
	    break;
	case GRAPHIC_OPENGL: {
	    //SDL_RenderPresent(SDL_PLUGIN(machine)->pallet);
	    //gl_show();
	}
	    break;
	case GRAPHIC_DIRECTX:
	    break;
	case GRAPHIC_CONSOLE:
	    break;
    }
}

SDL_PLUGIN_CALL int SDL_PLUGIN_TYPE sdl_plugin_playback(AUDIO_PLAYBACK loop) {
    switch (loop) {
	case AUDIO_PLAYBACK_ONCE:
	    return 0;
	case AUDIO_PLAYBACK_LOOP:
	    return -1;
	default:
	    break;
    }
    return 0;
}

static B32 res = 0;

SDL_PLUGIN_CALL void SDL_PLUGIN_TYPE sdl_res_min() {
    res = SDL_GetNumDisplayModes (0) - 1;
}

SDL_PLUGIN_CALL void SDL_PLUGIN_TYPE sdl_res_max() {
    res = 0;
}

SDL_PLUGIN_CALL STATUS SDL_PLUGIN_TYPE sdl_res_preview(B32 * w, B32 * h) {
    if (!machine || !machine->up || machine->up->system == SYSTEM_SDL || !machine->plugin) return Off;
#if defined (SDL_PLUGIN_ACTIVATED)
    SDL_DisplayMode dm;
    if (SDL_GetDisplayMode (0, res++, &dm) == 0) {
	*w = dm.w;
	*h = dm.h;
	return On;
    }
    res = 0;
    SDL_GetDisplayMode (0, res, &dm);
    *w = dm.w;
    *h = dm.h;
#endif
    return Off;
}

SDL_PLUGIN_CALL STATUS SDL_PLUGIN_TYPE sdl_res_next(B32 * w, B32 * h) {
    if (!machine || !machine->up || machine->up->system == SYSTEM_SDL || !machine->plugin) return Off;
#if defined (SDL_PLUGIN_ACTIVATED)
    SDL_DisplayMode dm;
    if (SDL_GetDisplayMode (0, res--, &dm) == 0) {
	*w = dm.w;
	*h = dm.h;
	return On;
    }
    res = SDL_GetNumDisplayModes (0) - 1;
    SDL_GetDisplayMode (0, res, &dm);
    *w = dm.w;
    *h = dm.h;
#endif
    return Off;
}

SDL_PLUGIN_CALL void SDL_PLUGIN_TYPE sdl_window_size_set() {
    if (!machine || !machine->up || machine->up->system == SYSTEM_SDL || !machine->plugin) return;
    SDL_SetWindowSize (SDL_PLUGIN(machine)->screen, machine->w, machine->h);
}

SDL_PLUGIN_CALL void SDL_PLUGIN_TYPE sdl_window_mode_set() {
    if (!machine || !machine->up || machine->up->system == SYSTEM_SDL || !machine->plugin) return;
    if (B16_IS_ON(machine->event, MACHINE_FULLSCREEN)) {
	SDL_SetWindowFullscreen (SDL_PLUGIN(machine)->screen, SDL_WINDOW_FULLSCREEN);
    } else {
	SDL_SetWindowFullscreen (SDL_PLUGIN(machine)->screen, 0);
    }
}

#endif // ASSHOLE
