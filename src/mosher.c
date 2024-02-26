#include "gfc_types.h"
#include "physics.h"
#include "mosher.h"
#include "audio.h"
#include "tmp_text.h"

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
	enemy->mass = 5.0;
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
	player->mass = 10.0;
	float l = player->shape.capsule.height+player->shape.capsule.radius*2.0;
	player->moment_of_inertia = player->mass*l*l/3.0;
	player->physics_material.friction = 1.0;
	player->physics_material.bounce = 1.0;
	player->position = vector2d(600.0, 200.0);
	player->center_of_mass = vector2d(0.0, 100.0);
	player->tags = TAG_PLAYER;
	player->update = mosher_update;
	player->mask = 1;
	player->layer = 1;
	world->player_idx = physics_get_body_id(world, player);
	return player;
}

void mosher_update(PhysicsBody *body, PhysicsWorld *world, float delta) {
	if(body->tags & TAG_DEAD) {
		body->physics_material.bounce = 0.3;
		body->timer -= delta;
		if(body->timer <= 0) {
			body->inuse = 0;
		}
		return;
	}

	Vector2D impulse;
	Bool skip_body = false;
	double beat_pos = get_beat_position();
	apply_righting(body, delta);
	if(body->tags & TAG_PLAYER) {
		if(!(world->mouse_buttons&1 && !world->prev_mouse_buttons&1)) {
			skip_body = true;
		} else {
			double dist = get_beat_distance(beat_pos);
			TextLine t;
			int c;
			Color color;
			if(dist < 0.05) {
				c = sprintf(t, "Perfect");
				color = gfc_color(0.47, 0.85, 0.22, 1.0);
			} else if (dist < 0.1) {
				c = sprintf(t, "Close");
				color = gfc_color(0.94, 0.69, 0.25, 1.0);
			} else {
				c = sprintf(t, "Miss");
				color = gfc_color(0.96, 0.29, 0.21, 1.0);
			}
			Vector2D pos;
			vector2d_add(pos, body->position, vector2d(0.0, -200.0));
			init_tmp_text(t, c, pos, color);
		}
		vector2d_sub(impulse, vector2d(world->mouse_x, 0.0), body->position);
	} else {
		skip_body = true;
		if(beat_pos < body->timer) {
			skip_body = false;
		}
		if(world->player_idx >= 0) {
			vector2d_sub(impulse, vector2d(physics_get_body(world, world->player_idx)->position.x, 0.0), body->position);
		}
		if(fabs(body->rotation) > M_PI/4.0) {
			body->tags |= TAG_DEAD;
			body->timer = 1.0;
			body->physics_material.bounce = 0.3;
			body->center_of_mass = vector2d(0.0, 0.0);
			body->layer = 0;
			body->mask = 2;
			return;
		}
	}
	body->timer = beat_pos;
	Vector2D collision_point;
	vector2d_add(collision_point, body->position, vector2d_rotate(body->center_of_mass, body->rotation));
	impulse.x = impulse.x*body->mass;
	impulse.y = body->mass*world->jump_velocity;
	if(!skip_body) {
		body->linear_velocity = vector2d(0.0, 0.0);
		apply_impulse(body, impulse, collision_point);
		drop_to_floor(body, world);
		body->physics_material.bounce = 1.0;
	} else {
		body->physics_material.bounce = 0.3;
	}
}