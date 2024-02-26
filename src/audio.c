#include <SDL.h>
#include <SDL_mixer.h>
#include "simple_logger.h"
#include "gfc_types.h"
#include "rollback.h"

Mix_Music *g_music = NULL;

double g_beat_interval;
double g_bpm;

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
	music_volume(1.0);
	atexit(free_audio);
}

void free_audio() {
	if(g_music != NULL) {
		Mix_FreeMusic(g_music);
	}
	Mix_CloseAudio();
}

extern Rollback *g_rollback;
extern double g_jump_velocity;

void set_bpm(double bpm) {
	g_bpm = bpm;
	g_beat_interval = 60.0/bpm;

	PhysicsWorld *physics_world = rollback_cur_physics(&g_rollback);
	float jump_velocity = g_jump_velocity;
	physics_world->gravity = jump_velocity/g_beat_interval;
	physics_world->jump_velocity = jump_velocity;
}

double get_beat_time() {
	return Mix_GetMusicPosition(g_music) / g_beat_interval;
}

double get_beat_position() {
	return fmod(Mix_GetMusicPosition(g_music), g_beat_interval);
}

double get_beat_distance(double beat_pos) {
	if(beat_pos > g_beat_interval/2.0) {
		return g_beat_interval-beat_pos;
	}
	return beat_pos;
}

void music_volume(float volume) {
	// volume 0 causes sdl mixer to stop playing the music
	Mix_VolumeMusic(SDL_max(MIX_MAX_VOLUME * volume, 1));
}

extern const char *g_song_filename;
void play_music() {
	if(!g_song_filename) {
		return;
	}
	g_music = Mix_LoadMUS(g_song_filename);
	if(!g_music) {
		slog("Failed to load music\n");
		return;
	}
	Mix_PlayMusic(g_music, -1);
	music_volume(1.0);
}