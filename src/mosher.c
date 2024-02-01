#include "gfc_types.h"
#include "physics.h"

// https://stackoverflow.com/questions/4633177/how-to-wrap-a-float-to-the-interval-pi-pi
// answer by Tim Čas
/* wrap x -> [0,max) */
double wrapMax(double x, double max)
{
    /* integer math: `(max + x % max) % max` */
    return fmod(max + fmod(x, max), max);
}
/* wrap x -> [min,max) */
double wrapMinMax(double x, double min, double max)
{
    return min + wrapMax(x - min, max - min);
}

void apply_righting(PhysicsBody *body, float delta) {
	float r = wrapMinMax(body->rotation, -M_PI, M_PI);
	body->angular_velocity -= delta*SDL_clamp(50.0*r, -50.0, 50.0);
}

void mosher_update(PhysicsBody *body, PhysicsWorld *world, float delta) {
	Vector2D impulse;
	Bool skip_body = false;
	apply_righting(body, delta);
	if(body->tags & TAG_PLAYER) {
		if(!(world->mouse_buttons&1 && !world->prev_mouse_buttons&1)) {
			skip_body = true;
		}
		vector2d_sub(impulse, vector2d(world->mouse_x, 0.0), body->position);
		vector2d_scale(impulse, impulse, 1.0);
	} else {
		skip_body = true;
		for(int i = 0; i < MAX_REPORTED_COLLISIONS; i++) {
			Collision *col = &body->collisions[i];
			PhysicsBody *other = physics_get_body(world, col->b_idx);
			if(vector2d_dot_product(col->normal, vector2d(0.0, -1.0)) > 0.0 && other->physics_type == STATIC && body->timer == 0) {
				skip_body = false;
				body->timer = 2;
			}
		}
		vector2d_sub(impulse, vector2d(physics_get_body(world, world->player_idx)->position.x, 0.0), body->position);
		vector2d_scale(impulse, impulse, 1.0);
	}
	impulse.x = impulse.x*body->mass;
	impulse.y = body->mass*world->jump_velocity;
	if(!skip_body) {
		body->linear_velocity = vector2d(0.0, 0.0);
		apply_central_impulse(body, impulse);
		body->position.y = 720.0;
		body->physics_material.bounce = 1.0;
	} else {
		body->physics_material.bounce = 0.3;
		body->timer = MAX(body->timer-1, 0);
	}
}