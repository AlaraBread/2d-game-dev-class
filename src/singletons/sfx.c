#include <stdlib.h>
#include <soloud_c.h>
#include <simple_logger.h>

extern Soloud *g_soloud;
WavStream g_click = NULL;
unsigned int g_click_handle = 0;

void play_click() {
	if(!g_soloud) return;
	if(g_click_handle != 0) {
		Soloud_stop(g_soloud, g_click_handle);
		g_click_handle = 0;
	}
	g_click = WavStream_create();
	if(!g_click) {
		slog("Failed to load click");
		return;
	}
	if(WavStream_load(g_click, "sound/sfx/click.wav")) {
		slog("Failed to load click");
		return;
	}
	WavStream_setLooping(g_click, 0);
	g_click_handle = Soloud_play(g_soloud, g_click);
}