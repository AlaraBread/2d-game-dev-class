#include <SDL.h>
#include <SDL_mixer.h>
#include "simple_logger.h"
#include "gfc_types.h"

Mix_Music *g_music = NULL;

void free_audio();

void init_audio() {
	if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, AUDIO_F32, 2, 512) < 0) {
		slog("Failed to open audio device.\n");
	} else {
		int frequency;
		Uint16 format;
		int channels;
		Mix_QuerySpec(&frequency, &format, &channels);
		slog("Opened audio at %d Hz %d bit%s %s audio buffer\n", frequency,
            (format&0xFF),
            (SDL_AUDIO_ISFLOAT(format) ? " (float)" : ""),
            (channels > 2) ? "surround" : (channels > 1) ? "stereo" : "mono");
	}
	Mix_VolumeMusic(MIX_MAX_VOLUME);
	g_music = Mix_LoadMUS("sound/music/laurasaidimblazed.mp3");
	if(!g_music) {
		slog("Failed to load music\n");
	}
	Mix_PlayMusic(g_music, -1);
	atexit(free_audio);
}

void free_audio() {
	if(g_music != NULL) {
		Mix_FreeMusic(g_music);
	}
	Mix_CloseAudio();
}