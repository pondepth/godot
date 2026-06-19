/**************************************************************************/
/*  display_server_psp.cpp                                                */
/**************************************************************************/

#include "display_server_psp.h"

#include "rasterizer_psp.h"

#include "core/input/input.h"

#include <GL/glut.h>

Vector<String> DisplayServerPSP::get_rendering_drivers_func() {
	Vector<String> drivers;
	drivers.push_back("dummy");
	return drivers;
}

DisplayServer *DisplayServerPSP::create_func(const String &, DisplayServerEnums::WindowMode, DisplayServerEnums::VSyncMode, uint32_t, const Vector2i *, const Vector2i &, int, DisplayServerEnums::Context, int64_t, Error &r_error) {
	r_error = OK;
	RasterizerPSP::make_current();
	return memnew(DisplayServerPSP);
}

void DisplayServerPSP::register_psp_driver() {
	register_create_function("psp", create_func, get_rendering_drivers_func);
}

static JoyButton psp_button_to_joy_button(uint32_t p_button) {
	switch (p_button) {
		case PSP_CTRL_CROSS: return JoyButton::A;
		case PSP_CTRL_CIRCLE: return JoyButton::B;
		case PSP_CTRL_SQUARE: return JoyButton::X;
		case PSP_CTRL_TRIANGLE: return JoyButton::Y;
		case PSP_CTRL_LTRIGGER: return JoyButton::LEFT_SHOULDER;
		case PSP_CTRL_RTRIGGER: return JoyButton::RIGHT_SHOULDER;
		case PSP_CTRL_SELECT: return JoyButton::BACK;
		case PSP_CTRL_START: return JoyButton::START;
		case PSP_CTRL_UP: return JoyButton::DPAD_UP;
		case PSP_CTRL_DOWN: return JoyButton::DPAD_DOWN;
		case PSP_CTRL_LEFT: return JoyButton::DPAD_LEFT;
		case PSP_CTRL_RIGHT: return JoyButton::DPAD_RIGHT;
		default: return JoyButton::INVALID;
	}
}

void DisplayServerPSP::_poll_controller() {
	sceCtrlPeekBufferPositive(&pad, 1);
	static const uint32_t buttons[] = {
		PSP_CTRL_CROSS, PSP_CTRL_CIRCLE, PSP_CTRL_SQUARE, PSP_CTRL_TRIANGLE,
		PSP_CTRL_LTRIGGER, PSP_CTRL_RTRIGGER, PSP_CTRL_SELECT, PSP_CTRL_START,
		PSP_CTRL_UP, PSP_CTRL_DOWN, PSP_CTRL_LEFT, PSP_CTRL_RIGHT,
	};

	for (uint32_t button : buttons) {
		const bool pressed = (pad.Buttons & button) != 0;
		const bool was_pressed = (previous_buttons & button) != 0;
		if (pressed != was_pressed) {
			Input::get_singleton()->joy_button(0, psp_button_to_joy_button(button), pressed);
		}
	}

	const float axis_x = CLAMP((static_cast<float>(pad.Lx) - 128.0f) / 127.0f, -1.0f, 1.0f);
	const float axis_y = CLAMP((static_cast<float>(pad.Ly) - 128.0f) / 127.0f, -1.0f, 1.0f);
	Input::get_singleton()->joy_axis(0, JoyAxis::LEFT_X, axis_x);
	Input::get_singleton()->joy_axis(0, JoyAxis::LEFT_Y, axis_y);
	previous_buttons = pad.Buttons;
}

void DisplayServerPSP::process_events() {
	_poll_controller();
	DisplayServerHeadless::process_events();
}

void DisplayServerPSP::swap_buffers() {
	glutSwapBuffers();
}

DisplayServerPSP::DisplayServerPSP() {
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
	Input::get_singleton()->joy_connection_changed(0, true, "PSP Controller", "030000004c0500006802000000010000");
}
