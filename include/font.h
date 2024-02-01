#ifndef __ALARA_FONT__
#define __ALARA_FONT__

#define FONT_OVERSAMPLING 4
#define NUM_FONTS 40

void init_font();
void font_render(const char *text, Color color, Vector2D position, Vector2D size);

#endif