#include "gf2d_draw.h"
#include "ui_element.h"
#include "font.h"
#include "util.h"
#include "ui_element.h"

unsigned int g_max_ui_elements;
unsigned int g_last_allocated_ui_element;
UIElement *g_ui_elements;

void free_ui_system();

void init_ui_system(unsigned int max_elements) {
	g_max_ui_elements = max_elements;
	g_ui_elements = calloc(sizeof(UIElement), max_elements);
	g_last_allocated_ui_element = max_elements-1;
	atexit(free_ui_system);
}

void free_ui_system() {
	for(int i = 0; i < g_max_ui_elements; i++) free_ui_element(&g_ui_elements[i]);
	free(g_ui_elements);
}

UIElement *allocate_ui_element() {
	for(int i = (g_last_allocated_ui_element+1)%g_max_ui_elements;
			i != g_last_allocated_ui_element;
			i = (i+1)%g_max_ui_elements) {
		if(!g_ui_elements[i].inuse) {
			g_last_allocated_ui_element = i;
			UIElement *element = &g_ui_elements[i];
			memset(element, 0, sizeof(UIElement));
			element->inuse = true;
			element->new = true;
			return element;
		}
	}
	return NULL;
}

void clear_ui_elements() {
	for(int i = 0; i < g_max_ui_elements; i++) free_ui_element(&g_ui_elements[i]);
	memset(g_ui_elements, 0, sizeof(UIElement) * g_max_ui_elements);
}

void clear_ui_group(int group) {
	for(int i = 0; i < g_max_ui_elements; i++) {
		UIElement *element = &g_ui_elements[i];
		if(element->group == group) {
			free_ui_element(element);
		}
	}
}

void free_ui_element(UIElement *element) {
	if(!element->inuse) {
		return;
	}
	if(element->cleanup) {
		element->cleanup(element);
	}
	element->inuse = false;
}

extern Uint32 g_prev_mouse_buttons;
extern Uint32 g_mouse_buttons;
extern int g_mouse_x;
extern int g_mouse_y;
extern int g_prev_mouse_x;
extern int g_prev_mouse_y;

Bool ui_element_rect_test(UIElement *element, int x, int y) {
	return x > element->position.x && x < element->position.x+element->size.x &&
			y > element->position.y && y < element->position.y+element->size.y;
}

void ui_element_mouse_events(UIElement *element) {
	if(!element->mouse_enter && !element->mouse_exit) {
		return;
	}
	Bool is_in_rect = ui_element_rect_test(element, g_mouse_x, g_mouse_y);
	Bool was_in_rect = ui_element_rect_test(element, g_prev_mouse_x, g_prev_mouse_y);
	if(element->new) {
		was_in_rect = false;
		element->new = false;
	}

	if(element->mouse_enter && is_in_rect && !was_in_rect) {
		element->mouse_enter(element);
	}
	if(element->mouse_exit && !is_in_rect && was_in_rect) {
		if(element->clicked && element->mouse_up) {
			element->mouse_up(element);
		}
		element->mouse_exit(element);
		element->clicked = false;
	}
	if(is_in_rect && (g_mouse_buttons & SDL_BUTTON(1))) {
		element->clicked = true;
		if(element->mouse_down) {
			element->mouse_down(element);
		}
	}
	if(is_in_rect && !(g_mouse_buttons & SDL_BUTTON(1)) && element->clicked) {
		element->clicked = false;
		if(element->mouse_up) {
			element->mouse_up(element);
		}
		if(element->click) {
			element->click(element);
		}
	}
}

void ui_predraw() {
	for(int i = 0; i < g_max_ui_elements; i++) {
		UIElement *element = &g_ui_elements[i];
		if(!element->inuse) {
			continue;
		}
		if(element->predraw) {
			element->predraw(element);
		}
	}
}

void ui_frame() {
	for(int i = 0; i < g_max_ui_elements; i++) {
		UIElement *element = &g_ui_elements[i];
		if(!element->inuse) {
			continue;
		}
		ui_element_mouse_events(element);
		if(element->think) {
			element->think(element);
		}
	}
	for(int i = 0; i < g_max_ui_elements; i++) {
		UIElement *element = &g_ui_elements[i];
		if(!element->inuse) {
			continue;
		}
		if(element->update) {
			element->update(element);
		}
		if(!element->inuse) {
			continue;
		}
		if(element->draw) {
			element->draw(element);
		}
	}
}

int get_ui_element_id(UIElement *element) {
	return element-g_ui_elements;
}

UIElement *get_ui_element(int id) {
	return &g_ui_elements[id];
}

void button_mouse_enter(UIElement *button) {
	button->bg_color = gfc_color(0.2, 0.2, 0.2, 1.0);
}

void button_mouse_exit(UIElement *button) {
	button->bg_color = gfc_color(0.1, 0.1, 0.1, 1.0);
}

void button_mouse_down(UIElement *button) {
	button->bg_color = gfc_color(0.0, 0.0, 0.0, 1.0);
}

void button_mouse_up(UIElement *button) {
	button->bg_color = gfc_color(0.2, 0.2, 0.2, 1.0);
}

void draw_text_rect(UIElement *element) {
	Rect rect = gfc_rect(element->position.x, element->position.y, element->size.x, element->size.y);
	gf2d_draw_rect_filled(rect, element->bg_color);
	gf2d_draw_rect(rect, element->border_color);
	if(element->text[0] != 0) {
		font_render_aligned(element->text, element->font_size, element->color,
				sdl_rect(element->position.x, element->position.y, element->size.x, element->size.y),
				element->text_align_x, element->text_align_y);
	}
}

UIElement *create_label(Vector2D position, TextAlign text_align_x, TextAlign text_align_y) {
	UIElement *label = allocate_ui_element();
	label->text_align_x = text_align_x;
	label->text_align_y = text_align_y;
	label->position = position;
	label->font_size = 2;
	label->color = gfc_color(1.0, 1.0, 1.0, 1.0);
	label->draw = draw_text_rect;
	return label;
}

UIElement *create_button(Vector2D position, Vector2D size, const char *text) {
	UIElement *button = allocate_ui_element();
	button->bg_color = gfc_color(0.1, 0.1, 0.1, 1.0);
	button->border_color = gfc_color(1.0, 1.0, 1.0, 1.0);
	button->color = gfc_color(1.0, 1.0, 1.0, 1.0);
	button->draw = draw_text_rect;
	button->mouse_enter = button_mouse_enter;
	button->mouse_exit = button_mouse_exit;
	button->mouse_down = button_mouse_down;
	button->mouse_up = button_mouse_up;
	button->position = vector2d(position.x-size.x/2, position.y-size.y/2);
	button->size = size;
	button->font_size = 5;
	button->text_align_x = CENTER;
	button->text_align_y = CENTER;
	memcpy(button->text, text, sizeof(char)*(strlen(text)+1));
	return button;
}