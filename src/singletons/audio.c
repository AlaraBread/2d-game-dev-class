#include <SDL.h>
#include "simple_logger.h"
#include "gfc_types.h"
#include "rollback.h"
#include "soloud_c.h"
#include "audio.h"

double g_beat_interval = 0.0;
double g_bpm = 0.0;
double g_filtered_music_time_seconds = 0.0;
Soloud *g_soloud = NULL;
WavStream *g_music;
unsigned int g_music_voice_handle = 0;

extern const char *g_song_filename;
extern Rollback g_rollback;
extern double g_jump_velocity;

void free_audio();

void init_audio() {
	g_soloud = Soloud_create();
	Soloud_initEx(g_soloud, SOLOUD_CLIP_ROUNDOFF | SOLOUD_ENABLE_VISUALIZATION, SOLOUD_AUTO, SOLOUD_AUTO, SOLOUD_AUTO, SOLOUD_AUTO);
	atexit(free_audio);
}

void free_audio() {
	if(g_soloud != NULL) {
		stop_music();
		Soloud_deinit(g_soloud);
		Soloud_destroy(g_soloud);
		g_soloud = NULL;
	}
}

void set_bpm(double bpm) {
	g_bpm = bpm;
	g_beat_interval = 60.0/bpm;
}

double get_beat_time() {
	return get_music_speed() * g_filtered_music_time_seconds / g_beat_interval;
}

void music_volume(float volume) {
	if(!Soloud_isValidVoiceHandle(g_soloud, g_music_voice_handle)) {
		return;
	}
	Soloud_setVolume(g_soloud, g_music_voice_handle, volume);
}

void play_music() {
	if(!g_song_filename) {
		return;
	}
	g_music = WavStream_create();
	if(!g_music) {
		slog("Failed to load music\n");
		return;
	}
	if(WavStream_load(g_music, g_song_filename)) {
		slog("Failed to load music\n");
		return;
	}
	WavStream_setLooping(g_music, 0);
	WavStream_setInaudibleBehavior(g_music, true, false);

	g_music_voice_handle = Soloud_play(g_soloud, g_music);

	g_filtered_music_time_seconds = 0.0;

	music_volume(1.0);
}

void stop_music() {
	if(g_music == NULL) {
		return;
	}
	WavStream_stop(g_music);
	WavStream_destroy(g_music);
	g_music = NULL;
}

void pause_music() {
	Soloud_setPause(g_soloud, g_music_voice_handle, true);
}

void resume_music() {
	Soloud_setPause(g_soloud, g_music_voice_handle, false);
}

double g_music_speed;
void set_music_speed(double speed) {
	g_music_speed = speed;
	Soloud_setRelativePlaySpeed(g_soloud, g_music_voice_handle, speed);
}

double get_music_speed() {
	return g_music_speed;
}

void audio_tick() {
	if(!Soloud_isValidVoiceHandle(g_soloud, g_music_voice_handle)) {
		return;
	}
	g_filtered_music_time_seconds = (g_filtered_music_time_seconds*9
			+Soloud_getStreamTime(g_soloud, g_music_voice_handle))/10;
}