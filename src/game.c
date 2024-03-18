#include <SDL.h>
#include "simple_logger.h"

#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "rollback.h"
#include "audio.h"
#include "mosher.h"
#include "font.h"
#include "entity.h"
#include "main_menu.h"
#include "points.h"
#include "save.h"

Rollback g_rollback;

Uint32 g_prev_mouse_buttons;
Uint32 g_mouse_buttons;
const Uint8 *g_keys;
Uint8 g_prev_keys[SDL_NUM_SCANCODES];

int g_mouse_x;
int g_mouse_y;
int g_prev_mouse_x;
int g_prev_mouse_y;

extern Bool g_paused;

void run_physics_frame();

int main(int argc, char *argv[])
{
	/*program initializtion*/
	init_logger("gf2d.log",0);
	slog("---==== BEGIN ====---");
	gf2d_graphics_initialize(
		"gf2d",
		1200,
		720,
		1200,
		720,
		vector4d(0,0,0,255),
		0);
	gf2d_graphics_set_frame_delay(16);
	gf2d_sprite_init(1024);
	SDL_ShowCursor(SDL_ENABLE);

	init_entity_system(500);
	init_audio();
	init_font();
	load();

	// physics setup
	g_rollback = init_rollback(200, 15);

	main_menu();

	g_prev_mouse_buttons = 0;

	/*main game loop*/
	while(1)
	{
		Bool done = false;
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_WINDOWEVENT: {
					switch (event.window.event) {
						case SDL_WINDOWEVENT_CLOSE:   // exit game
							done = true;
							break;
					}
					break;
				}
			}
		}
		if(done) {
			break;
		}
		int num_keys;
		g_keys = SDL_GetKeyboardState(&num_keys); // get the keyboard state for this frame
		/*update things here*/

		g_mouse_buttons = SDL_GetMouseState(&g_mouse_x, &g_mouse_y);

		gf2d_graphics_clear_screen();// clears drawing buffers

		run_physics_frame();
		entity_frame();

		g_prev_mouse_buttons = g_mouse_buttons;
		g_prev_mouse_x = g_mouse_x;
		g_prev_mouse_y = g_mouse_y;
		memcpy(g_prev_keys, g_keys, num_keys*sizeof(Uint8));

		gf2d_graphics_next_frame();// render current draw frame and skip to the next frame
		//printf("%f\n", gf2d_graphics_get_frames_per_second());
	}
	free_rollback(&g_rollback);
	save();
	slog("---==== END ====---");
	return 0;
}

void run_physics_frame() {
	audio_tick(0.016);

	PhysicsWorld *physics_world = rollback_cur_physics(&g_rollback);

	physics_world->mouse_x = g_mouse_x;
	physics_world->mouse_y = g_mouse_y;
	physics_world->prev_mouse_buttons = g_prev_mouse_buttons;
	physics_world->mouse_buttons = g_mouse_buttons;

	if(!g_paused) {
		physics_world = rollback_step(&g_rollback, 0.016);
	}

	physics_debug_draw(physics_world);
	physics_draw_sprites(physics_world);
}

/*eol@eof*/
