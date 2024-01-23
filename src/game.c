#include <SDL.h>
#include "simple_logger.h"

#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "rollback.h"

int main(int argc, char * argv[])
{
	/*variable declarations*/
	int done = 0;
	const Uint8 * keys;
	Sprite *sprite;
	
	int mx,my;
	float mf = 0;
	Sprite *mouse;
	Color mouseColor = gfc_color8(255,100,255,200);
	
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
	SDL_ShowCursor(SDL_DISABLE);
	
	/*demo setup*/
	sprite = gf2d_sprite_load_image("images/backgrounds/bg_flat.png");
	mouse = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);

	Sprite *rewind = gf2d_sprite_load_image("images/rewind.png");
	Sprite *fastforward = gf2d_sprite_load_image("images/fastforward.png");

	// physics setup
	Rollback rollback_world = init_rollback(2000, 20);
	PhysicsWorld *physics_world = rollback_cur_physics(&rollback_world);
	PhysicsBody *cursor_trigger = allocate_physics_body(physics_world);
	cursor_trigger->shape_type = CAPSULE;
	cursor_trigger->shape.circle.radius = 50.0;
	cursor_trigger->shape.capsule.height = 200.0;
	cursor_trigger->physics_type = RIGID;
	cursor_trigger->mass = 100.0;
	float l = cursor_trigger->shape.capsule.height+cursor_trigger->shape.capsule.radius*2.0;
	cursor_trigger->moment_of_inertia = cursor_trigger->mass*l*l/3.0;
	cursor_trigger->physics_material.friction = 1.0;
	cursor_trigger->physics_material.bounce = 1.0;
	cursor_trigger->position = vector2d(100.0, 200.0);
	cursor_trigger->center_of_mass = vector2d(0.0, 100.0);
	create_test_world(physics_world);

	/*main game loop*/
	while(!done)
	{
		SDL_PumpEvents();	// update SDL's internal event structures
		keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
		/*update things here*/

		int old_mx = mx;
		int old_my = my;

		Uint32 mouse_buttons = SDL_GetMouseState(&mx,&my);
		mf+=0.1;
		if (mf >= 16.0)mf = 0;
		
		gf2d_graphics_clear_screen();// clears drawing buffers
		// all drawing should happen betweem clear_screen and next_frame
		//backgrounds drawn first
		//gf2d_sprite_draw_image(sprite,vector2d(0,0));
		
		//UI elements last
		gf2d_sprite_draw(
				mouse,
				vector2d(mx,my),
				NULL,
				NULL,
				NULL,
				NULL,
				&mouseColor,
				(int)mf);

		physics_world = rollback_cur_physics(&rollback_world);
		if(mouse_buttons&1) {
			cursor_trigger = &physics_world->physics_bodies[0];
			cursor_trigger->position = vector2d(mx, my);
			cursor_trigger->linear_velocity = vector2d((mx-old_mx)/0.016, (my-old_my)/0.016);
			cursor_trigger->angular_velocity = 0.0;
		}

		if(mouse_buttons&4) {
			physics_world = rollback_step_back(&rollback_world, 0.016);
			gf2d_sprite_draw_image(rewind,vector2d(0,0));
		} else {
			if(mouse_buttons&2) {
				// fast forward
				rollback_step(&rollback_world, 0.016);
				rollback_step(&rollback_world, 0.016);
				rollback_step(&rollback_world, 0.016);
				gf2d_sprite_draw_image(fastforward,vector2d(0,0));
			}
			physics_world = rollback_step(&rollback_world, 0.016);
		}
		draw_sprites(physics_world);

		gf2d_graphics_next_frame();// render current draw frame and skip to the next frame
		
		if (keys[SDL_SCANCODE_ESCAPE])done = 1; // exit condition
	}
	free_rollback(&rollback_world);
	slog("---==== END ====---");
	return 0;
}
/*eol@eof*/
