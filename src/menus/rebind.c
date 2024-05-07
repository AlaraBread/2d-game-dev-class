#include "ui_element.h"
#include "rebind.h"
#include "simple_json.h"
#include "physics.h"
#include "midi.h"
#include "portmidi.h"

extern Uint32 g_prev_mouse_buttons;
extern Uint32 g_mouse_buttons;
extern const Uint8 *g_keys;
extern Uint8 g_prev_keys[SDL_NUM_SCANCODES];
extern float g_midi_cc[129];
extern float g_prev_midi_cc[129];
extern float g_midi_note[128];
extern float g_prev_midi_note[128];
extern int g_mouse_x;
extern int g_mouse_y;

const char *get_action_name(InputAction action) {
	switch(action) {
		case JUMP: return "jump";
		case POWERUP: return "use powerup";
		case PAUSE: return "pause";
		case DIRECTION: return "jump direction";
		case UI_ACCEPT: return "ui accept";
		case UI_NEXT: return "ui next";
		case UI_PREV: return "ui prev";
		default: return "";
	}
}

const char *get_action_id(InputAction action) {
	switch(action) {
		case JUMP: return "jump";
		case POWERUP: return "powerup";
		case PAUSE: return "pause";
		case DIRECTION: return "direction";
		case UI_ACCEPT: return "ui_accept";
		case UI_NEXT: return "ui_next";
		case UI_PREV: return "ui_prev";
		default: return "";
	}
}

Bool is_action_axis(InputAction action) {
	return action == DIRECTION;
}

SJson *g_bindings = NULL;

// not in sjson header for some reason?
void sj_object_delete_key(SJson *object, const char *key);

void set_binding(InputAction action, Binding binding) {
	SJson *b = sj_object_new();
	sj_object_insert(b, "type", sj_new_int(binding.type));
	sj_object_insert(b, "value", sj_new_int(binding.value.scancode));
	const char *action_id = get_action_id(action);
	sj_object_delete_key(g_bindings, action_id);
	sj_object_insert(g_bindings, action_id, b);
}

Binding get_binding(InputAction action) {
	const char *action_id = get_action_id(action);
	SJson *b = sj_object_get_value(g_bindings, action_id);
	SJson *type = sj_object_get_value(b, "type");
	SJson *value = sj_object_get_value(b, "value");

	Binding binding;
	binding.type = NONE;
	binding.value.mouse_button = 0;

	if(!sj_get_integer_value(type, (int *)&binding.type)) {
		return binding;
	}
	if(!sj_get_integer_value(value, (int *)&binding.value.mouse_button)) {
		return binding;
	}
	return binding;
}

Bool is_binding_pressed(PhysicsWorld *world, Binding binding) {
	switch(binding.type) {
		case MOUSE: return world->mouse_buttons & SDL_BUTTON(binding.value.mouse_button);
		case KEYBOARD: return world->keys[binding.value.scancode];
		case MIDI_NOTE: return world->midi_note[(int)binding.value.midi_note] > 0.0;
		case MIDI_CC: return world->midi_cc[(int)binding.value.midi_cc] > 0.5;
		default: return false;
	}
}

Bool global_is_binding_pressed(Binding binding) {
	switch(binding.type) {
		case MOUSE: return g_mouse_buttons & SDL_BUTTON(binding.value.mouse_button);
		case KEYBOARD: return g_keys[binding.value.scancode];
		case MIDI_NOTE: return g_midi_note[(int)binding.value.midi_note] > 0.0;
		case MIDI_CC: return g_midi_cc[(int)binding.value.midi_cc] > 0.5;
		default: return false;
	}
}

Bool global_was_binding_pressed(Binding binding) {
	switch(binding.type) {
		case MOUSE: return g_prev_mouse_buttons & SDL_BUTTON(binding.value.mouse_button);
		case KEYBOARD: return g_prev_keys[binding.value.scancode];
		case MIDI_NOTE: return g_prev_midi_note[(int)binding.value.midi_note] > 0.0;
		case MIDI_CC: return g_prev_midi_cc[(int)binding.value.midi_cc] > 0.5;
		default: return false;
	}
}

float get_axis_binding_value(PhysicsWorld *world, Binding binding) {
	switch(binding.type) {
		case MOUSE_MOTION: return world->mouse_x;
		case MIDI_CC: return world->midi_cc[(int)binding.value.midi_cc];
		default: return 0.0;
	}
}

float global_get_axis_binding_value(Binding binding) {
	switch(binding.type) {
		case MOUSE_MOTION: return (float)g_mouse_x/1200;
		case MIDI_CC: return g_midi_cc[(int)binding.value.midi_cc];
		default: return 0.0;
	}
}

float get_axis(PhysicsWorld *world, InputAction action) {
	return get_axis_binding_value(world, get_binding(action));
}

float global_get_axis(InputAction action) {
	return global_get_axis_binding_value(get_binding(action));
}

Bool is_action_pressed(PhysicsWorld *world, InputAction action) {
	Binding binding = get_binding(action);
	return is_binding_pressed(world, binding);
}

Bool is_action_just_pressed(PhysicsWorld *world, InputAction action) {
	Binding binding = get_binding(action);
	return is_binding_pressed(world, binding) && !is_binding_pressed(world->prev, binding);
}

Bool global_is_action_pressed(InputAction action) {
	Binding binding = get_binding(action);
	return global_is_binding_pressed(binding);
}

Bool global_is_action_just_pressed(InputAction action) {
	Binding binding = get_binding(action);
	return global_is_binding_pressed(binding) && !global_was_binding_pressed(binding);
}

void default_bindings() {
	free_bindings();
	g_bindings = sj_object_new();

	Binding binding;
	binding.type = MOUSE;
	binding.value.mouse_button = 1;
	set_binding(JUMP, binding);

	binding.type = MOUSE;
	binding.value.mouse_button = 3;
	set_binding(POWERUP, binding);

	binding.type = KEYBOARD;
	binding.value.scancode = SDL_SCANCODE_ESCAPE;
	set_binding(PAUSE, binding);

	binding.type = MOUSE_MOTION;
	binding.value.scancode = 0;
	set_binding(DIRECTION, binding);

	binding.type = KEYBOARD;
	binding.value.scancode = SDL_SCANCODE_SPACE;
	set_binding(UI_ACCEPT, binding);

	binding.type = KEYBOARD;
	binding.value.scancode = SDL_SCANCODE_TAB;
	set_binding(UI_NEXT, binding);

	binding.type = NONE;
	binding.value.scancode = 0;
	set_binding(UI_PREV, binding);
}

void save_bindings() {
	sj_save(g_bindings, "controls.json");
}

void load_bindings() {
	g_bindings = sj_load("controls.json");
	if(!g_bindings) {
		default_bindings();
	}
}

void free_bindings() {
	sj_free(g_bindings);
	g_bindings = NULL;
}


void rebind_button_clicked(UIElement *element) {
	if(element->timer == 0) {
		element->index = 1;
		element->timer = 1;
		element->click_start_position = vector2d(g_mouse_x, g_mouse_y);
		if(is_action_axis(element->input_action)) {
			strcpy(element->text, "move an axis...");
		} else {
			strcpy(element->text, "press a button...");
		}
	}
}

void rebind_button_reset(UIElement *element) {
	element->timer = 0;
	Binding binding = get_binding(element->input_action);
	TextLine input_name;
	switch(binding.type) {
		case MOUSE:
			sprintf(input_name, "mouse button %d", binding.value.mouse_button);
			break;
		case KEYBOARD:
			sprintf(input_name, "%s", SDL_GetKeyName(SDL_SCANCODE_TO_KEYCODE(binding.value.scancode)));
			break;
		case MIDI_NOTE:
			sprintf(input_name, "midi note %d", binding.value.midi_note);
			break;
		case MIDI_CC:
			sprintf(input_name, "midi cc %d", binding.value.midi_cc);
			break;
		case MOUSE_MOTION:
			sprintf(input_name, "mouse motion");
			break;
		case NONE:
			sprintf(input_name, "unbound");
			break;
		default:
			sprintf(input_name, "uknown input");
	}
	strcpy(element->text, input_name);
}

void rebind_button_think(UIElement *element) {
	Bool just_clicked = element->index == 1;
	element->index = 0;
	if(element->timer == 0) {
		return;
	}
	if(is_action_axis(element->input_action)) {
		if(!vector2d_distance_between_less_than(vector2d(g_mouse_x, g_mouse_y), element->click_start_position, 50)) {
			Binding binding;
			binding.type = MOUSE_MOTION;
			binding.value.mouse_button = 0;
			set_binding(element->input_action, binding);
			rebind_button_reset(element);
			return;
		}
		for(int i = 0; i < 129; i++) {
			if(g_midi_cc[i] != g_prev_midi_cc[i]) {
				Binding binding;
				binding.type = MIDI_CC;
				binding.value.midi_cc = i;
				set_binding(element->input_action, binding);
				rebind_button_reset(element);
				return;
			}
		}
		return;
	}
	if(!just_clicked && !g_mouse_buttons && g_prev_mouse_buttons) {
		for(int i = 1; i <= 5; i++) {
			if(!(g_mouse_buttons & SDL_BUTTON(i)) && (g_prev_mouse_buttons & SDL_BUTTON(i))) {
				Binding binding;
				binding.type = MOUSE;
				binding.value.mouse_button = i;
				set_binding(element->input_action, binding);
				rebind_button_reset(element);
				return;
			}
		}
	}
	for(int i = 0; i < SDL_NUM_SCANCODES; i++) {
		if(g_keys[i] && !g_prev_keys[i]) {
			Binding binding;
			binding.type = KEYBOARD;
			binding.value.scancode = i;
			set_binding(element->input_action, binding);
			rebind_button_reset(element);
			return;
		}
	}
	for(int i = 0; i < 128; i++) {
		if(g_midi_note[i] && !g_prev_midi_note[i]) {
			Binding binding;
			binding.type = MIDI_NOTE;
			binding.value.midi_note = i;
			set_binding(element->input_action, binding);
			rebind_button_reset(element);
			return;
		}
	}
	for(int i = 0; i < 129; i++) {
		if(g_midi_cc[i] != g_prev_midi_cc[i]) {
			Binding binding;
			binding.type = MIDI_CC;
			binding.value.midi_cc = i;
			set_binding(element->input_action, binding);
			rebind_button_reset(element);
			return;
		}
	}
}

void rebind_button_draw(UIElement *element) {
	draw_text_rect(element);
	float padding = 10;
	font_render_aligned(get_action_name(element->input_action), 3,
			gfc_color(1.0, 1.0, 1.0, 1.0),
			gfc_sdl_rect(element->position.x-padding, element->position.y,
			0, element->size.y),
			END, CENTER);
}

UIElement *create_rebind_button(Vector2D position, Vector2D size, InputAction action) {
	UIElement *element = create_button(position, size, get_action_name(action));
	element->font_size = 3;
	element->click = rebind_button_clicked;
	element->think = rebind_button_think;
	element->draw = rebind_button_draw;
	element->input_action = action;
	rebind_button_reset(element);
	return element;
}