#include "gfc_types.h"
#include "gf2d_sprite.h"

#ifndef __ALARA_ENTITY__
#define __ALARA_ENTITY__

#define MAX_ENTITY_SPRITES 3

typedef struct Entity_S {
	Bool inuse;
	char *text;
	Vector2D position;
	Vector2D size;
	unsigned int timer;
	void (*think)(struct Entity_S *self);
	void (*update)(struct Entity_S *self);
	void (*draw)(struct Entity_S *self);
	void (*cleanup)(struct Entity_S *self);
} Entity;

void init_entity_system(unsigned int max_entities);
Entity *allocate_entity();
int get_entity_id(Entity *body);
Entity *get_entity(int id);

Entity *init_frame_counter();

#endif