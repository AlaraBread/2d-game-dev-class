#include "gfc_types.h"
#include "gf2d_sprite.h"
#include "font.h"

#ifndef __ALARA_ENTITY__
#define __ALARA_ENTITY__

#define MAX_ENTITY_SPRITES 3

typedef struct Entity_S {
	Bool inuse;
	TextLine text;
	Color color;
	Color bg_color;
	Color border_color;
	Bool clicked;
	Vector2D position;
	Vector2D size;
	int font_size;
	TextAlign text_align_x;
	TextAlign text_align_y;
	unsigned int timer;
	void (*think)(struct Entity_S *self);
	void (*update)(struct Entity_S *self);
	void (*draw)(struct Entity_S *self);
	void (*cleanup)(struct Entity_S *self);
	void (*mouse_enter)(struct Entity_S *self);
	void (*mouse_exit)(struct Entity_S *self);
	void (*click)(struct Entity_S *self);
	void (*mouse_down)(struct Entity_S *self);
	void (*mouse_up)(struct Entity_S *self);
} Entity;

void init_entity_system(unsigned int max_entities);
Entity *allocate_entity();
void clear_entities();
int get_entity_id(Entity *ent);
Entity *get_entity(int id);
void draw_text_rect(Entity *ent);
void button_mouse_enter(Entity *button);
void button_mouse_exit(Entity *button);
void button_mouse_down(Entity *button);
void button_mouse_up(Entity *button);

#endif