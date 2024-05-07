#ifndef __ALARA_REBIND__
#define __ALARA_REBIND__

#include "ui_element.h"
#include "gfc_types.h"
#include "physics.h"

typedef enum {
	JUMP,
	POWERUP,
	PAUSE,
	DIRECTION,
	UI_NEXT,
	UI_PREV,
	UI_ACCEPT,
} InputAction;

typedef enum {
	NONE,
	MOUSE,
	KEYBOARD,
	MIDI_NOTE,
	MIDI_CC,
	MOUSE_MOTION,
} BindingType;

typedef struct Binding_S {
	BindingType type;
	union {
		Uint32 mouse_button;
		Uint32 scancode;
		unsigned char midi_cc;
		unsigned char midi_note;
	} value;
} Binding;

UIElement *create_rebind_button(Vector2D position, Vector2D size, InputAction action);

Binding get_binding(InputAction action);
Bool is_action_pressed(PhysicsWorld *world, InputAction action);
Bool is_action_just_pressed(PhysicsWorld *world, InputAction action);
Bool global_is_action_pressed(InputAction action);
Bool global_is_action_just_pressed(InputAction action);
float get_axis_binding_value(PhysicsWorld *world, Binding binding);
float global_get_axis_binding_value(Binding binding);
float get_axis(PhysicsWorld *world, InputAction action);
float global_get_axis(InputAction action);

void save_bindings();
void load_bindings();
void free_bindings();

#endif