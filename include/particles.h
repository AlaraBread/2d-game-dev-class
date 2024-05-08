#include <gfc_vector.h>
#include "gf2d_sprite.h"

#ifndef __ALARA_PARTICLES__
#define __ALARA_PARTICLES__

#define MAX_PARTICLES 1000
#define PARTICLE_TTL 1.0 // in seconds

typedef struct Particle {
	Bool inuse;
	float rotation;
	float angular_velocity;
	Vector2D position;
	Vector2D velocity;
	float ttl;
	Sprite *sprite;
} Particle;

void init_particles();
void free_particles();
Particle *spawn_particle();
void hit_floor(Vector2D position);
void update_particles(float delta);
void draw_particles();

#endif