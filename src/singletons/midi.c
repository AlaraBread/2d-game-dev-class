#include <stdlib.h>
#include <string.h>
#include <portmidi.h>
#include <porttime.h>
#include "simple_logger.h"
#include "midi.h"
#include "ui_element.h"
#include "rollback.h"
#include "controls_menu.h"

extern Rollback g_rollback;

PmEvent g_midi_event_buffer[MIDI_BUFFER_SIZE];
int g_midi_buffer_length = 0;
float g_midi_cc[129];
float g_prev_midi_cc[129];
float g_midi_note[128];
float g_prev_midi_note[128];
PortMidiStream *g_midi_stream = NULL;

void no_midi(UIElement *button);
void midi_device_selected(UIElement *button);

void midi_select() {
	PhysicsWorld *world = rollback_cur_physics(&g_rollback);
	physics_clear_bodies(world);
	clear_ui_elements();

	List *buttons = gfc_list_new();
	int num_devices = Pm_CountDevices();
	UIElement *none_button = create_button(vector2d(1200/2, 100), vector2d(300, 50), "none");
	none_button->click = no_midi;
	none_button->font_size = 2;
	gfc_list_append(buttons, none_button);
	float y = 100+70;
	for(int i = 0; i < num_devices; i++) {
		const PmDeviceInfo *device_info = Pm_GetDeviceInfo(i);
		if(!device_info->input) {
			continue;
		}
		UIElement *button = create_button(vector2d(1200/2, y), vector2d(300, 50), device_info->name);
		button->font_size = 2;
		y += 70;
		button->click = midi_device_selected;
		button->index = i;
		gfc_list_append(buttons, button);
	}
	UIElement *label = create_label(vector2d(1200/2, 0), CENTER, START);
	strcpy(label->text, "select a midi device");
	setup_list(buttons, NULL, NULL);
	gfc_list_delete(buttons);
}

void no_midi(UIElement *button) {
	midi_end();
	controls_menu();
}

void midi_device_selected(UIElement *button) {
	midi_end();
	PmDeviceID device_id = button->index;
	Pm_OpenInput(&g_midi_stream, device_id, NULL, MIDI_BUFFER_SIZE, NULL, NULL);
	controls_menu();
}

void midi_end() {
	if(!g_midi_stream) return;
	Pm_Close(g_midi_stream);
	g_midi_stream = NULL;
}

void midi_poll() {
	if(!g_midi_stream) return;
	g_midi_buffer_length = Pm_Read(g_midi_stream, g_midi_event_buffer, MIDI_BUFFER_SIZE);
	memcpy(g_prev_midi_note, g_midi_note, sizeof(g_midi_note));
	memcpy(g_prev_midi_cc, g_midi_cc, sizeof(g_midi_cc));
	for(int i = 0; i < g_midi_buffer_length; i++) {
		PmEvent *event = &g_midi_event_buffer[i];
		PmMessage message = event->message;
		unsigned char message_type = (message & 0xF0) >> 4;
		unsigned char d1 = (message & 0xFF00) >> 8;
		unsigned char d2 = (message & 0xFF0000) >> 16;
		switch(message_type) {
			case NOTE_OFF:
				d2 = 0;
			case NOTE_ON:
				g_midi_note[d1&0x7F] = (float)(d2&0x7F)/0x7F;
				break;
			case CONTROL_CHANGE:
				g_midi_cc[d1&0x7F] = (float)(d2&0x7F)/0x7F;
				break;
			case PITCH_WHEEL:
				g_midi_cc[128] = (float)(((int)d2<<7)|(int)d1)/(1<<14);
				break;
		}
	}
}