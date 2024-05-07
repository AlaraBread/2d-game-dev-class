#include "gfc_types.h"
#include "physics.h"
#include "mosher.h"
#include "audio.h"
#include "tmp_text.h"
#include "gfc_list.h"
#include "shop.h"
#include "end_screen.h"
#include "dance_floor.h"
#include "map.h"
#include "mods.h"
#include "rebind.h"

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
		body->angular_velocity -= delta*SDL_clamp(5.0*r, -20.0, 20.0);
	} else {
		body->angular_velocity -= delta*SDL_clamp(5.0*r, -30.0, 30.0);
	}
}

extern int g_selected_powerup;
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

void enemy_update(PhysicsBody *body, PhysicsWorld *world, float delta);
void player_update(PhysicsBody *body, PhysicsWorld *world, float delta);

PhysicsBody *init_enemy_mosher(PhysicsWorld *world, EnemySpawn spawn) {
	PhysicsBody *enemy = allocate_physics_body(world);
	if(enemy == NULL) {
		return enemy;
	}
	enemy->physics_type = RIGID;
	enemy->physics_material.friction = 1.0;
	enemy->position = vector2d(gfc_crandom()*200.0+300.0, gfc_crandom()*200.0+300.0);
	enemy->shape_type = CAPSULE;
	enemy->shape.circle.radius = 15.0;
	enemy->shape.capsule.height = 150.0;
	enemy->mass = 10.0;
	enemy->type = spawn.enemy_type;
	float com_ratio = 1.0;
	switch(spawn.enemy_type) {
		case SHORT:
			enemy->shape.capsule.height *= 0.5;
			enemy->shape.capsule.radius *= 1.25;
			enemy->sprite = gf2d_sprite_load_image("images/short.png");
			break;
		case AGGRESSIVE:
			enemy->shape.capsule.height *= 0.8;
			enemy->sprite = gf2d_sprite_load_image("images/angry.png");
			break;
		case LAZY:
			enemy->shape.capsule.height *= 1.3;
			enemy->sprite = gf2d_sprite_load_image("images/lazy.png");
			enemy->mass *= 0.25;
			com_ratio = 0.75;
			break;
		case NORMAL:
			enemy->sprite = gf2d_sprite_load_image("images/normal.png");
			break;
		case SCARED:
			enemy->sprite = gf2d_sprite_load_image("images/scared.png");
			break;
	}
	float l = enemy->shape.capsule.height+enemy->shape.capsule.radius*2.0;
	enemy->moment_of_inertia = enemy->mass*l*l/3.0;
	enemy->center_of_mass = vector2d(0.0, com_ratio*enemy->shape.capsule.height/2);
	enemy->update = enemy_update;
	enemy->mask = 1;
	enemy->layer = 1;
	enemy->tags = TAG_ENEMY;
	enemy->physics_material.bounce = get_mosher_bounce(enemy);
	enemy->sprite_offset = vector2d(-enemy->shape.capsule.radius, -enemy->shape.capsule.height/2-enemy->shape.capsule.radius);
	return enemy;
}

long int g_missed_beats = 0;
#define STRONG_LEGS_FACTOR 1.7
PhysicsBody *init_player_mosher(PhysicsWorld *world) {
	g_missed_beats = 0;
	PhysicsBody *player = allocate_physics_body(world);
	player->shape_type = CAPSULE;
	player->shape.circle.radius = 30.0;
	player->shape.capsule.height = 200.0;
	player->physics_type = RIGID;
	player->mass = 20.0;
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
	player->sprite = gf2d_sprite_load_image("images/player.png");
	player->sprite_offset = vector2d(-player->shape.capsule.radius, -player->shape.capsule.height/2-player->shape.capsule.radius);
	world->player_idx = physics_get_body_id(world, player);
	if(g_selected_powerup == STRONG_LEGS) {
		player->gravity_scale = STRONG_LEGS_FACTOR;
	}
	return player;
}

double get_distance_to_beat(double *beats, List *nearby_beats, int *used_beats, double beat_time, long int *index) {
	double min_dist = INFINITY;
	unsigned int len = gfc_list_get_count(nearby_beats);
	for(int i = 0; i < len; i++) {
		long int j = (long int)gfc_list_get_nth(nearby_beats, i);
		if(used_beats[j]) {
			continue;
		}
		double b = beats[j];
		double dist = b - beat_time;
		if(fabs(dist) < fabs(min_dist)) {
			min_dist = dist;
			*index = j;
		}
	}
	return min_dist;
}

double get_first_beat(double *beats, List *nearby_beats, int *used_beats, int *index) {
	int len = gfc_list_get_count(nearby_beats);
	for(int i = 0; i < len; i++) {
		long int j = (long int)gfc_list_get_nth(nearby_beats, i);
		if(used_beats[j]) {
			continue;
		}
		*index = j;
		return beats[j];
	}
	return -1.0;
}

double get_beat_position(double *beats, List *nearby_beats, double beat_time) {
	int len = gfc_list_get_count(nearby_beats);
	for(int i = len-1; i >= 0; i--) {
		double b = beats[(long int)gfc_list_get_nth(nearby_beats, i)];
		if(beat_time > b) {
			return beat_time-b;
		}
	}
	return 0.0;
}

extern List *g_nearby_beats;
extern List *g_nearby_secondary_beats;
extern double g_beat_interval;
extern double g_end;
extern int g_points;
extern int g_level_points;
extern int g_game_state;

Bool mosher_update(PhysicsBody *body, PhysicsWorld *world, float delta) {
	// check if we are dead
	if(body->tags & TAG_DEAD) {
		body->physics_material.bounce = get_mosher_bounce(body);
		body->timer -= delta;
		if(body->timer <= 0) {
			if(body->tags & TAG_PLAYER) {
				world->player_idx = -1;
				g_game_state = LOST;
				create_end_screen();
			}
			physics_body_free(body);
		}
		return true;
	}

	apply_righting(body, delta);

	// dont do anything if the game is over
	if(g_game_state == WON || g_game_state == LOST) {
		return true;
	}

	// check if we should die
	float die_threshold = M_PI/4.0;
	if(body->tags & TAG_PLAYER) {
		die_threshold = M_PI/3.0;
	}
	if(fabs(body->rotation) > die_threshold) {
		body->tags |= TAG_DEAD;
		body->timer = 1.0;
		body->physics_material.bounce = get_mosher_bounce(body);
		body->center_of_mass = vector2d(0.0, 0.0);
		body->layer = 0;
		body->mask = 2;
		if(body->tags & TAG_PLAYER) {
			g_game_state = DYING;
			music_fade_out();
		}
		return true;
	}
	return false;
}

extern Bool g_mods_enabled[NUM_MODS];

void hit_beat() {
	g_missed_beats = 0;
}

void miss_beat() {
	g_missed_beats++;
	int beat_miss_threshold = g_mods_enabled[PERFECTIONIST] ? 1 : 5;
	if(g_missed_beats >= beat_miss_threshold) {
		g_game_state = LOST;
		music_fade_out();
		create_end_screen();
	}
}

extern double *g_beats;
extern double *g_secondary_beats;
extern int *g_used_beats;
extern int *g_used_secondary_beats;

void player_update(PhysicsBody *body, PhysicsWorld *world, float delta) {
	if(mosher_update(body, world, delta)) {
		return;
	}

	double beat_time = get_beat_time();

	// check for a missed beat
	int first_beat_idx = -1;
	double first_beat = get_first_beat(g_beats, g_nearby_beats, g_used_beats, &first_beat_idx);

	if(first_beat_idx != -1 && (beat_time - first_beat)*g_beat_interval > 0.25) {
		// we missed this beat
		g_used_beats[first_beat_idx] = 3;
		miss_beat();
	}

	// fart/vaccum powerup
	if((g_selected_powerup == FART || g_selected_powerup == VACCUM) &&
			body->cooldown <= 0.0 &&
			is_action_just_pressed(world, POWERUP)) {
		body->cooldown = 5.0;
		for(int i = 0; i < world->max_physics_bodies; i++) {
			PhysicsBody *e = &world->physics_bodies[i];
			if(!e->inuse || !(e->tags & TAG_ENEMY)) {
				continue;
			}
			Vector2D impulse;
			vector2d_sub(impulse, body->position, e->position);
			float dist = vector2d_magnitude(impulse);
			vector2d_scale(impulse, impulse, 1.0/(dist*dist));
			if(g_selected_powerup == VACCUM) {
				vector2d_scale(impulse, impulse, 2500000.0);
			} else {
				vector2d_scale(impulse, impulse, -2500000.0);
			}
			apply_central_impulse(e, impulse);
		}
	}
	body->cooldown = SDL_max(body->cooldown-delta, 0.0);

	Vector2D impulse;

	if(beat_time >= g_end || !music_is_playing()) {
		g_game_state = WON;
		create_end_screen();
		return;
	}

	if(!is_action_just_pressed(world, JUMP)) {
		body->physics_material.bounce = get_mosher_bounce(body);
		return;
	}

	long int secondary_idx = -1;
	long int primary_idx = -1;
	double secondary_beat_dist = fabs(get_distance_to_beat(g_secondary_beats, g_nearby_secondary_beats, g_used_secondary_beats, beat_time, &secondary_idx));
	double dist = fabs(get_distance_to_beat(g_beats, g_nearby_beats, g_used_beats, beat_time, &primary_idx));
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
	int use_code = 0;
	if(dist < 0.05) {
		c = sprintf(t, "perfect");
		color = gfc_color(0.47, 0.85, 0.22, 1.0);
		p = 3;
		use_code = 1;
		hit_beat();
	} else if (dist < 0.1) {
		c = sprintf(t, "close");
		color = gfc_color(0.94, 0.69, 0.25, 1.0);
		p = 1;
		use_code = 2;
		hit_beat();
	} else {
		c = sprintf(t, "miss");
		color = gfc_color(0.96, 0.29, 0.21, 1.0);
		use_code = 3;
		miss_beat();
	}

	if(dist < 0.25) {
		if(secondary && secondary_idx != -1) {
			g_used_secondary_beats[secondary_idx] = use_code;
		} else if (primary_idx != -1) {
			g_used_beats[primary_idx] = use_code;
		}
	}

	if(secondary) {
		color = gfc_color(0.70, 0.36, 0.98, 1.0);
	}
	g_level_points += p;
	g_points += p;

	Vector2D pos;
	vector2d_add(pos, body->position, vector2d(0.0, -200.0));
	init_tmp_text(t, c, pos, color);

	Binding binding = get_binding(DIRECTION);
	if(binding.type == MOUSE_MOTION) {
		vector2d_sub(impulse, vector2d(world->mouse_x, 0.0), body->position);
	} else {
		impulse.x = (get_axis_binding_value(world, binding)-0.5) * 1000;
	}
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

	if(body->type == LAZY) {
		return;
	}

	Vector2D impulse;
	double beat_time = get_beat_time();

	double beat_pos = get_beat_position(g_beats, g_nearby_beats, beat_time);
	if(beat_pos >= body->timer) {
		body->physics_material.bounce = 0.3;
		body->timer = beat_pos;
		return;
	}
	body->timer = beat_pos;

	if(world->player_idx >= 0) {
		vector2d_sub(impulse, vector2d(physics_get_body(world, world->player_idx)->position.x, 0.0), body->position);
		switch(body->type) {
			case AGGRESSIVE:
				impulse.x *= 2.0;
				break;
			case SCARED:
				impulse.x *= -1;
				break;
			case SHORT:
				impulse.x *= 0.75;
				break;
			default:
		}
		if(g_mods_enabled[ANGRY]) {
			impulse.x *= 2.0;
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