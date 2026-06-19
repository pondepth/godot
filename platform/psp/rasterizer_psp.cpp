/**************************************************************************/
/*  rasterizer_psp.cpp                                                    */
/**************************************************************************/

#include "rasterizer_psp.h"

#ifndef WINDOWS_ENABLED
#include <GL/glut.h>
#endif

static void psp_gl_color(const Color &p_color) {
	glColor4f(p_color.r, p_color.g, p_color.b, p_color.a);
}

const PSPTextureCache::Texture *RasterizerCanvasPSP::_bind_texture(RID p_texture) {
	ERR_FAIL_NULL_V(texture_cache, nullptr);
	return texture_cache->bind(p_texture, false);
}

void RasterizerCanvasPSP::_draw_rect(const Item::CommandRect *p_rect, const Transform2D &p_transform, const Color &p_modulate) {
	const PSPTextureCache::Texture *texture = _bind_texture(p_rect->texture);
	const Rect2 &rect = p_rect->rect;
	Rect2 source = p_rect->source;
	if (texture && (source.size.x == 0 || source.size.y == 0)) {
		source = Rect2(0, 0, texture->width, texture->height);
	}

	float u0 = 0.0f;
	float v0 = 0.0f;
	float u1 = 1.0f;
	float v1 = 1.0f;
	if (texture) {
		u0 = source.position.x / texture->width;
		v0 = source.position.y / texture->height;
		u1 = source.get_end().x / texture->width;
		v1 = source.get_end().y / texture->height;
		if (p_rect->flags & CANVAS_RECT_FLIP_H) {
			SWAP(u0, u1);
		}
		if (p_rect->flags & CANVAS_RECT_FLIP_V) {
			SWAP(v0, v1);
		}
	}

	const Vector2 p0 = p_transform.xform(rect.position);
	const Vector2 p1 = p_transform.xform(rect.position + Vector2(rect.size.x, 0));
	const Vector2 p2 = p_transform.xform(rect.get_end());
	const Vector2 p3 = p_transform.xform(rect.position + Vector2(0, rect.size.y));
	psp_gl_color(p_modulate * p_rect->modulate);
	glBegin(GL_QUADS);
	glTexCoord2f(u0, v0); glVertex2f(p0.x, p0.y);
	glTexCoord2f(u1, v0); glVertex2f(p1.x, p1.y);
	glTexCoord2f(u1, v1); glVertex2f(p2.x, p2.y);
	glTexCoord2f(u0, v1); glVertex2f(p3.x, p3.y);
	glEnd();
}

void RasterizerCanvasPSP::_draw_primitive(const Item::CommandPrimitive *p_primitive, const Transform2D &p_transform, const Color &p_modulate) {
	const PSPTextureCache::Texture *texture = _bind_texture(p_primitive->texture);
	GLenum primitive = GL_POINTS;
	if (p_primitive->point_count == 2) {
		primitive = GL_LINES;
	} else if (p_primitive->point_count == 3) {
		primitive = GL_TRIANGLES;
	} else if (p_primitive->point_count == 4) {
		primitive = GL_QUADS;
	}

	glBegin(primitive);
	for (uint32_t i = 0; i < p_primitive->point_count; i++) {
		psp_gl_color(p_modulate * p_primitive->colors[i]);
		if (texture) {
			glTexCoord2f(p_primitive->uvs[i].x, p_primitive->uvs[i].y);
		}
		const Vector2 point = p_transform.xform(p_primitive->points[i]);
		glVertex2f(point.x, point.y);
	}
	glEnd();
}

void RasterizerCanvasPSP::canvas_render_items(RID, Item *p_item_list, const Color &p_modulate, Light *, Light *, const Transform2D &p_canvas_transform, RSE::CanvasItemTextureFilter, RSE::CanvasItemTextureRepeat, bool, bool &r_sdf_used, RenderingServerTypes::RenderInfo *) {
	r_sdf_used = false;
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 480.0, 272.0, 0.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	for (Item *item = p_item_list; item; item = item->next) {
		if (!item->visible) {
			continue;
		}

		if (item->final_clip_owner) {
			const Rect2 clip = item->final_clip_owner->final_clip_rect;
			glEnable(GL_SCISSOR_TEST);
			glScissor(static_cast<int>(clip.position.x), 272 - static_cast<int>(clip.get_end().y), static_cast<int>(clip.size.x), static_cast<int>(clip.size.y));
		} else {
			glDisable(GL_SCISSOR_TEST);
		}

		Transform2D command_transform;
		const Transform2D item_transform = p_canvas_transform * item->final_transform;
		const Color item_modulate = p_modulate * item->final_modulate;
		for (Item::Command *command = item->commands; command; command = command->next) {
			switch (command->type) {
				case Item::Command::TYPE_TRANSFORM: {
					command_transform = static_cast<Item::CommandTransform *>(command)->xform;
				} break;
				case Item::Command::TYPE_RECT: {
					_draw_rect(static_cast<Item::CommandRect *>(command), item_transform * command_transform, item_modulate);
				} break;
				case Item::Command::TYPE_PRIMITIVE: {
					_draw_primitive(static_cast<Item::CommandPrimitive *>(command), item_transform * command_transform, item_modulate);
				} break;
				default:
					break;
			}
		}
	}
	glDisable(GL_SCISSOR_TEST);
}

RendererCompositor *RasterizerPSP::_create_current() {
	return memnew(RasterizerPSP);
}

void RasterizerPSP::make_current() {
	_create_func = _create_current;
	low_end = true;
}

RasterizerPSP::RasterizerPSP() {
	texture_cache = memnew(PSPTextureCache);
	memdelete(canvas);
	canvas = memnew(RasterizerCanvasPSP(texture_cache));
	memdelete(scene);
	scene = memnew(RasterizerScenePSP(texture_cache));
}

RasterizerPSP::~RasterizerPSP() {
	memdelete(texture_cache);
}

void RasterizerPSP::initialize() {
	glViewport(0, 0, 480, 272);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void RasterizerPSP::begin_frame(double p_frame_step) {
	RasterizerDummy::begin_frame(p_frame_step);
	glViewport(0, 0, 480, 272);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 480.0, 272.0, 0.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RasterizerPSP::finalize() {
	texture_cache->clear();
	static_cast<RasterizerScenePSP *>(scene)->clear_mesh_cache();
}
