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

#endif