#include "gfc_types.h"
#include "gfc_vector.h"
#include "entity.h"

void tmp_text_update(Entity *ent) {
	ent->timer -= 1;
	ent->color.a = ((float)ent->timer)/ent->total_timer;
	if(ent->timer <= 0) {
		free_entity(ent);
	}
}

Entity *init_tmp_text(char *text, int length, Vector2D position, Color color) {
	Entity *ent = allocate_entity();
	memcpy(ent->text, text, length);
	ent->position = position;
	ent->text_align_x = CENTER;
	ent->text_align_y = CENTER;
	ent->font_size = 5;
	ent->color = color;
	ent->draw = draw_text_rect;
	ent->update = tmp_text_update;
	ent->total_timer = 30;
	ent->timer = ent->total_timer;
}