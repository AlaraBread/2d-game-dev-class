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
		g_fonts_regular[i] = TTF_OpenFont("fonts/RuslanDisplay-Regular.ttf", i*10+10);
	}
	atexit(font_free);
}

void font_render(const char *text, Color color, Vector2D position, Vector2D size) {
	int avg_size = FONT_OVERSAMPLING*((size.x+size.y)/2);
	int font_idx = (avg_size-10)/10;
	if(font_idx >= NUM_FONTS) {
		font_idx = NUM_FONTS-1;
	}
	SDL_Surface *surface = TTF_RenderUTF8_Blended_Wrapped(g_fonts_regular[font_idx], text, gfc_color_to_sdl(color), 0);
	SDL_Renderer *renderer = gf2d_graphics_get_renderer();
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_Rect dstrect = gfc_sdl_rect(position.x, position.y, size.x, size.y);
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