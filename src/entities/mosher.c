#include "gfc_types.h"
#include "physics.h"
#include "mosher.h"
#include "audio.h"
#include "tmp_text.h"
#include "gfc_list.h"
#include "shop.h"

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
		body->angular_velocity -= delta*SDL_clamp(5.0*r, -5.0, 5.0);
	} else {
		body->angular_velocity -= delta*SDL_clamp(5.0*r, -30.0, 30.0);
	}
}

extern int g_selected_powerup;
void enemy_update(PhysicsBody *body, PhysicsWorld *world, float delta);
void player_update(PhysicsBody *body, PhysicsWorld *world, float delta);

PhysicsBody *init_enemy_mosher(PhysicsWorld *world) {
	PhysicsBody *enemy = allocate_physics_body(world);
	if(enemy == NULL) {
		return enemy;
	}
	enemy->physics_type = RIGID;
	enemy->position = vector2d(gfc_crandom()*200.0+300.0, gfc_crandom()*200.0+300.0);
	enemy->shape_type = CAPSULE;
	enemy->shape.circle.radius = 40.0;
	enemy->shape.capsule.height = 150.0;
	enemy->center_of_mass = vector2d(0.0, 75.0);
	enemy->mass = 10.0;
	float l = enemy->shape.capsule.height+enemy->shape.capsule.radius*2.0;
	enemy->moment_of_inertia = enemy->mass*l*l/3.0;
	enemy->physics_material.bounce = 0.1;
	enemy->physics_material.friction = 1.0;
	enemy->update = enemy_update;
	enemy->mask = 1;
	enemy->layer = 1;
	enemy->tags = TAG_ENEMY;
	return enemy;
}

#define STRONG_LEGS_FACTOR 1.7
PhysicsBody *init_player_mosher(PhysicsWorld *world) {
	PhysicsBody *player = allocate_physics_body(world);
	player->shape_type = CAPSULE;
	player->shape.circle.radius = 50.0;
	player->shape.capsule.height = 200.0;
	player->physics_type = RIGID;
	player->mass = 10.0;
	if(g_selected_powerup == TALL) {
		player->mass *= 0.75;
		player->shape.capsule.height *= 2;
		player->shape.capsule.radius *= 0.75;
	}
	float l = player->shape.capsule.height+player->shape.capsule.radius*2.0;
	player->moment_of_inertia = player->mass*l*l/3.0;
	player->physics_material.friction = 1.0;
	player->physics_material.bounce = 1.0;
	player->position = vector2d(600.0, 200.0);
	player->center_of_mass = vector2d(0.0, player->shape.capsule.height/2);
	player->tags = TAG_PLAYER;
	player->update = player_update;
	player->mask = 1;
	player->layer = 1;
	world->player_idx = physics_get_body_id(world, player);
	if(g_selected_powerup == STRONG_LEGS) {
		player->gravity_scale = STRONG_LEGS_FACTOR;
	}
	return player;
}

double get_distance_to_beat(List *beats, double beat_time) {
	double min_dist = INFINITY;
	unsigned int len = gfc_list_get_count(beats);
	for(int i = 0; i < len; i++) {
		double b = *((double *)gfc_list_get_nth(beats, i));
		double dist = b - beat_time;
		if(fabs(dist) < fabs(min_dist)) {
			min_dist = dist;
		}
	}
	return min_dist;
}

double get_beat_position(List *beats, double beat_time) {
	int len = gfc_list_get_count(beats);
	for(int i = len-1; i >= 0; i--) {
		double b = *((double *)gfc_list_get_nth(beats, i));
		if(beat_time > b) {
			return beat_time-b;
		}
	}
	return 0.0;
}

float get_mosher_bounce(PhysicsBody *body) {
	if(body->tags & TAG_PLAYER) {
		if(g_selected_powerup == BOUNCY) {
			return 1.0;
		}
		return 0.3;
	} else {
		return 0.3;
	}
}

extern List *g_nearby_beats;
extern List *g_nearby_secondary_beats;
extern double g_beat_interval;
extern int g_points;
extern int g_level_points;

Bool mosher_update(PhysicsBody *body, PhysicsWorld *world, float delta) {
	apply_righting(body, delta);

	// check if we are dead
	if(body->tags & TAG_DEAD) {
		body->physics_material.bounce = get_mosher_bounce(body);
		body->timer -= delta;
		if(body->timer <= 0) {
			if(body->tags & TAG_PLAYER) {
				world->player_idx = -1;
			}
			body->inuse = 0;
		}
		return true;
	}

	// check if we should die
	if(fabs(body->rotation) > M_PI/4.0) {
		body->tags |= TAG_DEAD;
		body->timer = 1.0;
		body->physics_material.bounce = get_mosher_bounce(body);
		body->center_of_mass = vector2d(0.0, 0.0);
		body->layer = 0;
		body->mask = 2;
		return true;
	}
	return false;
}

void player_update(PhysicsBody *body, PhysicsWorld *world, float delta) {
	if(mosher_update(body, world, delta)) {
		return;
	}
	// magnet powerup
	if(g_selected_powerup == MAGNET && (world->mouse_buttons&SDL_BUTTON(3)) && !(world->prev_mouse_buttons&SDL_BUTTON(3))) {
		for(int i = 0; i < world->max_physics_bodies; i++) {
			PhysicsBody *e = &world->physics_bodies[i];
			if(!e->inuse || !(e->tags & TAG_ENEMY)) {
				continue;
			}
			Vector2D impulse;
			vector2d_sub(impulse, body->position, e->position);
			vector2d_scale(impulse, impulse, 50.0);
			apply_central_impulse(e, impulse);
		}
	}

	Vector2D impulse;
	double beat_time = get_beat_time();

	if(!(world->mouse_buttons&SDL_BUTTON(1) && !world->prev_mouse_buttons&SDL_BUTTON(1))) {
		body->physics_material.bounce = get_mosher_bounce(body);
		return;
	}

	double secondary_beat_dist = get_distance_to_beat(g_nearby_secondary_beats, beat_time);
	double dist = fabs(get_distance_to_beat(g_nearby_beats, beat_time));
	Bool secondary = false;
	if(secondary_beat_dist < dist) {
		dist = fabs(secondary_beat_dist);
		secondary = true;
	}
	dist = dist*g_beat_interval;
	TextLine t;
	int c;
	Color color;
	int p = 0;
	if(dist < 0.05) {
		c = sprintf(t, "Perfect");
		color = gfc_color(0.47, 0.85, 0.22, 1.0);
		p = 3;
	} else if (dist < 0.1) {
		c = sprintf(t, "Close");
		color = gfc_color(0.94, 0.69, 0.25, 1.0);
		p = 1;
	} else {
		c = sprintf(t, "Miss");
		color = gfc_color(0.96, 0.29, 0.21, 1.0);
	}
	if(secondary) {
		color = gfc_color(0.70, 0.36, 0.98, 1.0);
	}
	g_level_points += p;
	g_points += p;

	Vector2D pos;
	vector2d_add(pos, body->position, vector2d(0.0, -200.0));
	init_tmp_text(t, c, pos, color);

	vector2d_sub(impulse, vector2d(world->mouse_x, 0.0), body->position);
	Vector2D collision_point;
	vector2d_add(collision_point, body->position, vector2d_rotate(body->center_of_mass, body->rotation));
	impulse.x = impulse.x*body->mass;
	impulse.y = body->mass*world->jump_velocity;
	if(g_selected_powerup == STRONG_LEGS) {
		impulse.y *= STRONG_LEGS_FACTOR;
	}

	body->linear_velocity = vector2d(0.0, 0.0);
	apply_impulse(body, impulse, collision_point);
	drop_to_floor(body, world);
	body->physics_material.bounce = 1.0;
}

void enemy_update(PhysicsBody *body, PhysicsWorld *world, float delta) {
	if(mosher_update(body, world, delta)) {
		return;
	}

	Vector2D impulse;
	double beat_time = get_beat_time();

	double beat_pos = get_beat_position(g_nearby_beats, beat_time);
	if(beat_pos >= body->timer) {
		body->physics_material.bounce = 0.3;
		body->timer = beat_pos;
		return;
	}
	body->timer = beat_pos;

	if(world->player_idx >= 0) {
		vector2d_sub(impulse, vector2d(physics_get_body(world, world->player_idx)->position.x, 0.0), body->position);
		if(false) {
			//TODO: game modifiers
			vector2d_scale(impulse, impulse, 2.0);
		}
	}

	Vector2D collision_point;
	vector2d_add(collision_point, body->position, vector2d_rotate(body->center_of_mass, body->rotation));
	impulse.x = impulse.x*body->mass;
	impulse.y = body->mass*world->jump_velocity;

	body->linear_velocity = vector2d(0.0, 0.0);
	apply_impulse(body, impulse, collision_point);
	drop_to_floor(body, world);
	body->physics_material.bounce = 1.0;
}