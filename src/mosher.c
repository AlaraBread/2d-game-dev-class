#include "gfc_types.h"
#include "physics.h"
#include "mosher.h"

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
	if(body->tags & TAG_PLAYER) {
		body->angular_velocity -= delta*SDL_clamp(20.0*r, -10.0, 10.0);
	} else {
		body->angular_velocity -= delta*SDL_clamp(5.0*r, -30.0, 30.0);
	}
}

PhysicsBody *init_enemy_mosher(PhysicsWorld *world) {
	PhysicsBody *enemy = allocate_physics_body(world);
	enemy->physics_type = RIGID;
	enemy->position = vector2d(gfc_crandom()*200.0+300.0, gfc_crandom()*200.0+300.0);
	enemy->shape_type = CAPSULE;
	enemy->shape.circle.radius = 40.0;
	enemy->shape.capsule.height = 150.0;
	enemy->center_of_mass = vector2d(0.0, 75.0);
	enemy->mass = 20.0;
	float l = enemy->shape.capsule.height+enemy->shape.capsule.radius*2.0;
	enemy->moment_of_inertia = enemy->mass*l*l/3.0;
	enemy->physics_material.bounce = 0.1;
	enemy->physics_material.friction = 1.0;
	enemy->update = mosher_update;
	enemy->mask = 1;
	enemy->layer = 1;
	return enemy;
}

PhysicsBody *init_player_mosher(PhysicsWorld *world) {
	PhysicsBody *player = allocate_physics_body(world);
	player->shape_type = CAPSULE;
	player->shape.circle.radius = 50.0;
	player->shape.capsule.height = 200.0;
	player->physics_type = RIGID;
	player->mass = 30.0;
	float l = player->shape.capsule.height+player->shape.capsule.radius*2.0;
	player->moment_of_inertia = player->mass*l*l/3.0;
	player->physics_material.friction = 1.0;
	player->physics_material.bounce = 1.0;
	player->position = vector2d(100.0, 200.0);
	player->center_of_mass = vector2d(0.0, 100.0);
	player->tags = TAG_PLAYER;
	player->update = mosher_update;
	player->mask = 1;
	player->layer = 1;
	world->player_idx = physics_get_body_id(world, player);
	return player;
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
		if(world->player_idx >= 0) {
			vector2d_sub(impulse, vector2d(physics_get_body(world, world->player_idx)->position.x, 0.0), body->position);
		}
		vector2d_scale(impulse, impulse, 1.0);
	}
	impulse.x = impulse.x*body->mass;
	impulse.y = body->mass*world->jump_velocity;
	if(!skip_body) {
		body->linear_velocity = vector2d(0.0, 0.0);
		apply_central_impulse(body, impulse);
		drop_to_floor(body, world);
		body->physics_material.bounce = 1.0;
	} else {
		body->physics_material.bounce = 0.3;
		body->timer = MAX(body->timer-1, 0);
	}
}