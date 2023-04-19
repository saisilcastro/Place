#include <wave-of.h>

int main(void){
	SystemSet up;
	up.system = SYSTEM_ALLEGRO;
	up.driver = GRAPHIC_PRIMARY;
	GearSet gear = gear_standard(up);
	gear.w = 1024;
	gear.h = 768;
	gear.put(&gear);
	gear.start();
	wave_loop(&gear);
	return 0;
}