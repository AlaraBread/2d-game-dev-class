#include <SDL.h>
#include "simple_logger.h"

#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "rollback.h"
#include "audio.h"
#include "mosher.h"
#include "font.h"
#include "entity.h"

Rollback g_rollback;

Uint32 g_prev_mouse_buttons;
Uint32 g_mouse_buttons;

int g_mouse_x;
int g_mouse_y;
int g_old_mouse_x;
int g_old_mouse_y;

void run_physics_frame();

int main(int argc, char * argv[])
{
	int done = 0;
	const Uint8 * keys;
	
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

	init_frame_counter()->position = vector2d(100, 300);
	init_frame_counter()->position = vector2d(400, 300);
	init_frame_counter()->position = vector2d(700, 300);

	// physics setup
	g_rollback = init_rollback(2000, 20);
	PhysicsWorld *physics_world = rollback_cur_physics(&g_rollback);
	init_player_mosher(physics_world);
	physics_create_test_world(physics_world);
	float jump_interval = 60.0/160.0;
	float jump_velocity = 1000.0;
	physics_world->gravity = jump_velocity/jump_interval; // hit the ground again after a certain interval
	physics_world->jump_velocity = jump_velocity;

	g_prev_mouse_buttons = 0;

	/*main game loop*/
	while(!done)
	{
		SDL_PumpEvents();	// update SDL's internal event structures
		keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
		/*update things here*/

		g_mouse_buttons = SDL_GetMouseState(&g_mouse_x, &g_mouse_y);

		gf2d_graphics_clear_screen();// clears drawing buffers

		run_physics_frame();
		entity_frame();

		g_prev_mouse_buttons = g_mouse_buttons;
		g_old_mouse_x = g_mouse_x;
		g_old_mouse_y = g_mouse_y;

		gf2d_graphics_next_frame();// render current draw frame and skip to the next frame

		if (keys[SDL_SCANCODE_ESCAPE])done = 1; // exit condition
		//printf("%f\n", gf2d_graphics_get_frames_per_second());
	}
	free_rollback(&g_rollback);
	slog("---==== END ====---");
	return 0;
}

void run_physics_frame() {
	PhysicsWorld *physics_world = rollback_cur_physics(&g_rollback);

	physics_world->mouse_x = g_mouse_x;
	physics_world->mouse_y = g_mouse_y;
	physics_world->prev_mouse_buttons = g_prev_mouse_buttons;
	physics_world->mouse_buttons = g_mouse_buttons;

	if(g_mouse_buttons&4) {
		// rewind
		physics_world = rollback_step_back(&g_rollback, 0.016);

		Sprite *rewind = gf2d_sprite_load_image("images/rewind.png");
		gf2d_sprite_draw_image(rewind,vector2d(0,0));
		gf2d_sprite_free(rewind);
	} else {
		if(g_mouse_buttons&2) {
			// fast forward
			rollback_step(&g_rollback, 0.016);
			rollback_step(&g_rollback, 0.016);
			rollback_step(&g_rollback, 0.016);
			Sprite *fastforward = gf2d_sprite_load_image("images/fastforward.png");
			gf2d_sprite_draw_image(fastforward,vector2d(0,0));
			gf2d_sprite_free(fastforward);
		}
		// normal speed
		physics_world = rollback_step(&g_rollback, 0.016);
	}
	physics_debug_draw(physics_world);
	physics_draw_sprites(physics_world);
}

/*eol@eof*/
