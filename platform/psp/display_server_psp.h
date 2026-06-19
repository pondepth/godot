/**************************************************************************/
/*  display_server_psp.h                                                  */
/**************************************************************************/

#pragma once

#include "servers/display/display_server_headless.h"

#include <pspctrl.h>

class DisplayServerPSP : public DisplayServerHeadless {
	GDSOFTCLASS(DisplayServerPSP, DisplayServerHeadless);

	SceCtrlData pad = {};
	uint32_t previous_buttons = 0;

	static Vector<String> get_rendering_drivers_func();
	static DisplayServer *create_func(const String &p_rendering_driver, DisplayServerEnums::WindowMode p_mode, DisplayServerEnums::VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i *p_position, const Vector2i &p_resolution, int p_screen, DisplayServerEnums::Context p_context, int64_t p_parent_window, Error &r_error);

	void _poll_controller();

public:
	static void register_psp_driver();

	String get_name() const override { return "psp"; }
	int get_screen_count() const override { return 1; }
	Size2i screen_get_size(int p_screen = DisplayServerEnums::SCREEN_OF_MAIN_WINDOW) const override { return Size2i(480, 272); }
	Rect2i screen_get_usable_rect(int p_screen = DisplayServerEnums::SCREEN_OF_MAIN_WINDOW) const override { return Rect2i(0, 0, 480, 272); }
	Size2i window_get_size(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override { return Size2i(480, 272); }
	DisplayServerEnums::WindowMode window_get_mode(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override { return DisplayServerEnums::WINDOW_MODE_FULLSCREEN; }
	bool window_can_draw(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override { return true; }
	bool can_any_window_draw() const override { return true; }

	void process_events() override;
	void swap_buffers() override;

	DisplayServerPSP();
};
