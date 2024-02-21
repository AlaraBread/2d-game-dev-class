#ifndef __ALARA_AUDIO__
#define __ALARA_AUDIO__

void init_audio();
void set_bpm(double bpm);
double get_beat_position();
double get_beat_distance(double beat_pos);
void music_volume(float volume);
void play_music();

#endif