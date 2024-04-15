#include "gfc_types.h"
#include "gfc_vector.h"
#include "ui_element.h"

void tmp_text_update(UIElement *element) {
	element->timer -= 1;
	element->color.a = ((float)element->timer)/element->total_timer;
	if(element->timer <= 0) {
		free_ui_element(element);
	}
}

UIElement *init_tmp_text(char *text, int length, Vector2D position, Color color) {
	UIElement *element = allocate_ui_element();
	memcpy(element->text, text, sizeof(char)*length);
	element->position = position;
	element->text_align_x = CENTER;
	element->text_align_y = CENTER;
	element->font_size = 5;
	element->color = color;
	element->draw = draw_text_rect;
	element->update = tmp_text_update;
	element->total_timer = 30;
	element->timer = element->total_timer;
	return element;
}