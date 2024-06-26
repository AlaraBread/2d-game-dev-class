#ifndef __ALARA_AUDIO__
#define __ALARA_AUDIO__

void init_audio();
void set_bpm(double bpm);
double get_beat_time();
void music_volume(float volume);
void play_music();
void pause_music();
void resume_music();
void stop_music();
Bool music_is_playing();
void audio_tick(float delta);
void set_music_speed(double speed);
double get_music_speed();
void music_fade_out();

#endif