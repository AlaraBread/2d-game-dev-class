#include <stdlib.h>
#include "gfc_types.h"
#include "ui_element.h"
#include "shop.h"

int g_points = 0;
int g_level_points = -1;

extern int g_high_score;
static void update_points_label(UIElement *label) {
	int p = g_points;
	if(g_level_points != -1) {
		p = g_level_points;
		if(p > g_high_score) {
			label->color = gfc_color(0.95, 0.97, 0.67, 1.0);
		} else {
			label->color = gfc_color(1.0, 1.0, 1.0, 1.0);
		}
	}
	if(label->index != p) {
		sprintf(label->text, "$%d", p);
	}
	label->index = p;
}

UIElement *create_points_label() {
	UIElement *points_label = create_label(vector2d(0, 0), START, START);
	points_label->font_size = 4;
	points_label->index = -1;
	points_label->update = update_points_label;
	update_points_label(points_label);
	return points_label;
}