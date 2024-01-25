#include "gf2d_draw.h"

typedef struct Entity {
	struct Entity *other;
	Bool inuse;
} Entity;

unsigned int g_max_entities;
unsigned int g_last_allocated_entity;
Entity *g_entities;

void free_entity_system();

void init_entity_system(unsigned int max_entities) {
	g_max_entities = max_entities;
	g_entities = calloc(sizeof(max_entities), max_entities);
	g_last_allocated_entity = max_entities-1;
	atexit(free_entity_system);
}

void free_entity_system() {
	free(g_entities);
}

Entity *allocate_entity() {
	for(int i = (g_last_allocated_entity+1)%g_max_entities;
			i != g_last_allocated_entity;
			i = (i+1)%g_max_entities) {
		if(!g_entities[i].inuse) {
			g_last_allocated_entity = i;
			Entity *entity = &g_entities[i];
			memset(entity, 0, sizeof(Entity));
			entity->inuse = true;
			return entity;
		}
	}
	return NULL;
}

int get_entity_id(Entity *body) {
	return body-g_entities;
}

Entity *get_entity(int id) {
	return &g_entities[id];
}
