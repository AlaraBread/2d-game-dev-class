#include "gf2d_draw.h"
#include "entity.h"
#include "font.h"

unsigned int g_max_entities;
unsigned int g_last_allocated_entity;
Entity *g_entities;

void free_entity_system();

void init_entity_system(unsigned int max_entities) {
	g_max_entities = max_entities;
	g_entities = calloc(sizeof(Entity), max_entities);
	g_last_allocated_entity = max_entities-1;
	atexit(free_entity_system);
}

void free_entity(Entity *ent);

void free_entity_system() {
	for(int i = 0; i < g_max_entities; i++) free_entity(&g_entities[i]);
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

void free_entity(Entity *ent) {
	if(!ent->inuse) {
		return;
	}
	if(ent->cleanup) {
		ent->cleanup(ent);
	}
	ent->inuse = false;
}

void entity_frame() {
	for(int i = 0; i < g_max_entities; i++) {
		Entity *ent = &g_entities[i];
		if(!ent->inuse) {
			continue;
		}
		if(ent->think) {
			ent->think(ent);
		}
	}
	for(int i = 0; i < g_max_entities; i++) {
		Entity *ent = &g_entities[i];
		if(!ent->inuse) {
			continue;
		}
		if(ent->update) {
			ent->update(ent);
		}
		if(ent->draw) {
			ent->draw(ent);
		}
	}
}

int get_entity_id(Entity *body) {
	return body-g_entities;
}

Entity *get_entity(int id) {
	return &g_entities[id];
}

void draw_frame_counter(Entity *ent) {
	font_render(ent->text, gfc_color(1.0, 1.0, 1.0, 1.0), ent->position, ent->size);
}

void update_frame_counter(Entity *ent) {
	ent->timer += 1;
	SDL_itoa(ent->timer, ent->text, 10);
}

void cleanup_frame_counter(Entity *ent) {
	free(ent->text);
}

Entity *init_frame_counter() {
	Entity *ent = allocate_entity();
	ent->size = vector2d(100, 100);
	ent->draw = draw_frame_counter;
	ent->update = update_frame_counter;
	ent->cleanup = cleanup_frame_counter;
	ent->text = calloc(256, sizeof(char));
	return ent;
}
