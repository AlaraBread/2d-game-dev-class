#ifndef __ALARA_UI_ELEMENT__
#define __ALARA_UI_ELEMENT__

#include "gfc_types.h"
#include "gf2d_sprite.h"
#include "font.h"
#include "gfc_list.h"

#define MAX_ENTITY_SPRITES 3

#define UI_GROUP_PAUSE_MENU 1
#define UI_GROUP_END_MENU 2

typedef struct UIElement_S {
	Bool inuse;
	TextLine text;
	char filename[512];
	Color color;
	Color bg_color;
	Color border_color;
	Bool clicked;
	Vector2D position;
	Vector2D size;
	int font_size;
	int index;
	int group;
	Bool new;
	TextAlign text_align_x;
	TextAlign text_align_y;
	unsigned int timer;
	unsigned int total_timer;
	Sprite *sprite;
	int input_action;
	Vector2D click_start_position;
	struct UIElement_S *next;
	struct UIElement_S *prev;
	void (*think)(struct UIElement_S *self);
	void (*update)(struct UIElement_S *self);
	void (*predraw)(struct UIElement_S *self);
	void (*draw)(struct UIElement_S *self);
	void (*cleanup)(struct UIElement_S *self);
	void (*mouse_enter)(struct UIElement_S *self);
	void (*mouse_exit)(struct UIElement_S *self);
	void (*click)(struct UIElement_S *self);
	void (*mouse_down)(struct UIElement_S *self);
	void (*mouse_up)(struct UIElement_S *self);
} UIElement;

void init_ui_system(unsigned int max_entities);
UIElement *allocate_ui_element();
void free_ui_element(UIElement *element);
void clear_ui_elements();
void clear_ui_group(int group);
int get_ui_element_id(UIElement *element);
UIElement *get_ui_element(int id);
void draw_text_rect(UIElement *element);
void button_mouse_enter(UIElement *button);
void button_mouse_exit(UIElement *button);
void button_mouse_down(UIElement *button);
void button_mouse_up(UIElement *button);
UIElement *create_button(Vector2D position, Vector2D size, const char *text);
UIElement *create_label(Vector2D position, TextAlign text_align_x, TextAlign text_align_y);
void ui_frame();
void ui_predraw();
void setup_list(List *list, UIElement *before, UIElement *after);

#endif