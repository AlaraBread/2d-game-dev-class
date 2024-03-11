#include <SDL.h>
#include <SDL_ttf.h>
#include "gfc_types.h"
#include "gfc_color.h"
#include "gf2d_graphics.h"
#include "font.h"

void font_free();

TTF_Font *g_fonts_regular[NUM_FONTS];

void init_font() {
	TTF_Init();
	for(int i = 0; i < NUM_FONTS; i++) {
		g_fonts_regular[i] = TTF_OpenFont("fonts/MadimiOne-Regular.ttf", i*10+10);
	}
	atexit(font_free);
}

void font_render_aligned(const char *text, int size, Color color, SDL_Rect rect, TextAlign x_align, TextAlign y_align) {
	int w, h;
	TTF_SizeUTF8(g_fonts_regular[size], text, &w, &h);
	Vector2D position;
	switch(x_align) {
		case CENTER:
			position.x = rect.x+(rect.w-w)/2;
			break;
		case START:
			position.x = rect.x;
			break;
		case END:
			position.x = rect.x+rect.w-w;
			break;
	}
	switch(y_align) {
		case CENTER:
			position.y = rect.y+(rect.h-h)/2;
			break;
		case START:
			position.y = rect.y;
			break;
		case END:
			position.y = rect.y+rect.h-h;
			break;
	}
	font_render(text, size, color, position);
}

void font_render(const char *text, int size, Color color, Vector2D position) {
	int w, h;
	TTF_SizeUTF8(g_fonts_regular[size], text, &w, &h);
	SDL_Surface *surface = TTF_RenderUTF8_Blended_Wrapped(g_fonts_regular[size], text, gfc_color_to_sdl(color), 0);
	SDL_Renderer *renderer = gf2d_graphics_get_renderer();
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_Rect dstrect = gfc_sdl_rect(position.x, position.y, w, h);
	SDL_RenderCopy(renderer, texture, NULL, &dstrect);
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
}

void font_free() {
	for(int i = 0; i < NUM_FONTS; i++) {
		TTF_CloseFont(g_fonts_regular[i]);
	}
	TTF_Quit();
}