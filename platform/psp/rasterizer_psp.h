/**************************************************************************/
/*  rasterizer_psp.h                                                      */
/**************************************************************************/

#pragma once

#include "servers/rendering/dummy/rasterizer_canvas_dummy.h"
#include "servers/rendering/dummy/rasterizer_dummy.h"

#include "rasterizer_scene_psp.h"
#include "texture_cache_psp.h"

#include "core/templates/hash_map.h"

#ifdef WINDOWS_ENABLED
#include <windows.h>
#endif
#include <GL/gl.h>

class RasterizerCanvasPSP : public RasterizerCanvasDummy {
	PSPTextureCache *texture_cache = nullptr;

	const PSPTextureCache::Texture *_bind_texture(RID p_texture);
	void _draw_rect(const Item::CommandRect *p_rect, const Transform2D &p_transform, const Color &p_modulate);
	void _draw_primitive(const Item::CommandPrimitive *p_primitive, const Transform2D &p_transform, const Color &p_modulate);

public:
	RasterizerCanvasPSP(PSPTextureCache *p_texture_cache) : texture_cache(p_texture_cache) {}
	void canvas_render_items(RID p_to_render_target, Item *p_item_list, const Color &p_modulate, Light *p_light_list, Light *p_directional_list, const Transform2D &p_canvas_transform, RSE::CanvasItemTextureFilter p_default_filter, RSE::CanvasItemTextureRepeat p_default_repeat, bool p_snap_2d_vertices_to_pixel, bool &r_sdf_used, RenderingServerTypes::RenderInfo *r_render_info = nullptr) override;
};

class RasterizerPSP : public RasterizerDummy {
	static RendererCompositor *_create_current();
	PSPTextureCache *texture_cache = nullptr;

public:
	static void make_current();
	void initialize() override;
	void begin_frame(double p_frame_step) override;
	void finalize() override;
	bool is_opengl() override { return true; }

	RasterizerPSP();
	~RasterizerPSP();
};
