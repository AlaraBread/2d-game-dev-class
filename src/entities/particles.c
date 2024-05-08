#include "particles.h"

// https://stackoverflow.com/questions/4633177/how-to-wrap-a-float-to-the-interval-pi-pi
// answer by Tim Čas
/* wrap x -> [0,max) */
static double wrapMax(double x, double max)
{
    /* integer math: `(max + x % max) % max` */
    return fmod(max + fmod(x, max), max);
}
/* wrap x -> [min,max) */
static double wrapMinMax(double x, double min, double max)
{
    return min + wrapMax(x - min, max - min);
}

Particle *g_particles = NULL;
void init_particles() {
	g_particles = calloc(sizeof(Particle), MAX_PARTICLES);
	atexit(free_particles);
}

void free_particle(Particle *particle);
void free_particles() {
	if(!g_particles) return;
	for(int i = 0; i < MAX_PARTICLES; i++) {
		free_particle(&g_particles[i]);
	}
	free(g_particles);
	g_particles = NULL;
}

void free_particle(Particle *particle) {
	if(!particle->inuse) return;
	gf2d_sprite_free(particle->sprite);
	particle->inuse = false;
}

const char *get_random_white_puff() {
	switch(rand()%25) {
		case 0: return "images/particles/whitePuff00.png";
		case 1: return "images/particles/whitePuff01.png";
		case 2: return "images/particles/whitePuff02.png";
		case 3: return "images/particles/whitePuff03.png";
		case 4: return "images/particles/whitePuff04.png";
		case 5: return "images/particles/whitePuff05.png";
		case 6: return "images/particles/whitePuff06.png";
		case 7: return "images/particles/whitePuff07.png";
		case 8: return "images/particles/whitePuff08.png";
		case 9: return "images/particles/whitePuff09.png";
		case 10: return "images/particles/whitePuff10.png";
		case 11: return "images/particles/whitePuff11.png";
		case 12: return "images/particles/whitePuff12.png";
		case 13: return "images/particles/whitePuff13.png";
		case 14: return "images/particles/whitePuff14.png";
		case 15: return "images/particles/whitePuff15.png";
		case 16: return "images/particles/whitePuff16.png";
		case 17: return "images/particles/whitePuff17.png";
		case 18: return "images/particles/whitePuff18.png";
		case 19: return "images/particles/whitePuff19.png";
		case 20: return "images/particles/whitePuff20.png";
		case 21: return "images/particles/whitePuff21.png";
		case 22: return "images/particles/whitePuff22.png";
		case 23: return "images/particles/whitePuff23.png";
		case 24: return "images/particles/whitePuff24.png";
	}
	return "images/particles/whitePuff24.png";
}

unsigned int g_last_allocated_particle = MAX_PARTICLES-1;
Particle *spawn_particle() {
	for(int i = (g_last_allocated_particle+1)%MAX_PARTICLES;
			i != g_last_allocated_particle;
			i = (i+1)%MAX_PARTICLES) {
		if(!g_particles[i].inuse) {
			g_last_allocated_particle = i;
			Particle *particle = &g_particles[i];
			memset(particle, 0, sizeof(Particle));
			particle->inuse = true;
			particle->sprite = NULL;
			particle->ttl = PARTICLE_TTL;
			particle->sprite = gf2d_sprite_load_image(get_random_white_puff());
			return particle;
		}
	}
	return NULL;
}

#define PARTICLE_VELOCITY 500.0
#define PARTICLE_ANGULAR_VELOCITY 5.0
#define PARTICLE_GRAVITY 500.0

float g_particle_cooldown = 0.0;
void hit_floor(Vector2D position) {
	if(g_particle_cooldown > 0.0) {
		return;
	}
	g_particle_cooldown = 0.2;
	for(int i = 0; i < 10; i++) {
		Particle *particle = spawn_particle();
		if(!particle) return;
		particle->position = position;
		vector2d_add(particle->position, particle->position, vector2d(gfc_crandom()*50, gfc_crandom()*50));
		particle->velocity = vector2d(PARTICLE_VELOCITY*gfc_crandom(), PARTICLE_VELOCITY*gfc_crandom()-10);
		particle->angular_velocity = PARTICLE_ANGULAR_VELOCITY*gfc_crandom();
		particle->rotation = gfc_crandom()*M_PI*2.0;
	}
}

void update_particles(float delta) {
	g_particle_cooldown -= delta;
	for(int i = 0; i < MAX_PARTICLES; i++) {
		Particle *particle = &g_particles[i];
		if(!particle->inuse) continue;
		particle->ttl -= delta;
		particle->velocity.y += PARTICLE_GRAVITY*delta;
		Vector2D movement;
		vector2d_scale(movement, particle->velocity, delta);
		vector2d_add(particle->position, particle->position, movement);
		particle->rotation += particle->angular_velocity*delta;
		particle->rotation = wrapMinMax(particle->rotation, 0.0, M_PI*2.0);
		if(particle->ttl <= 0.0) {
			free_particle(particle);
		}
	}
}

void draw_particles() {
	for(int i = 0; i < MAX_PARTICLES; i++) {
		Particle *particle = &g_particles[i];
		if(!particle->inuse) continue;
		Color color_shift = gfc_color(1.0, 1.0, 1.0, 0.25*(particle->ttl)/PARTICLE_TTL);
		const float scale = 0.5;
		Vector2D scale_v = vector2d(scale, scale);
		Vector2D position = particle->position;
		float rotation = particle->rotation*GFC_RADTODEG;
		Vector2D center = vector2d(128, 128);
		gf2d_sprite_draw(particle->sprite, position, &scale_v, &center, &rotation, NULL, &color_shift, 0);
	}
}