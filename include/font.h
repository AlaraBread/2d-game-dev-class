#ifndef __ALARA_FONT__
#define __ALARA_FONT__

#define FONT_OVERSAMPLING 4
#define NUM_FONTS 40

typedef enum {CENTER, START, END} TextAlign;

void font_init();
void font_render(const char *text, int size, Color color, Vector2D position);
void font_render_aligned(const char *text, int size, Color color, SDL_Rect rect, TextAlign x_align, TextAlign y_align);

#endif