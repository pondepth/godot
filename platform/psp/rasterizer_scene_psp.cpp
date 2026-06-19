/**************************************************************************/
/*  rasterizer_scene_psp.cpp                                              */
/**************************************************************************/

#include "rasterizer_scene_psp.h"

#include "servers/rendering/dummy/storage/material_storage.h"
#include "servers/rendering/dummy/storage/light_storage.h"
#include "servers/rendering/dummy/storage/mesh_storage.h"
#include "servers/rendering/rendering_server.h"

static void psp_store_projection(const Projection &p_projection, float *r_matrix) {
	for (int column = 0; column < 4; column++) {
		for (int row = 0; row < 4; row++) {
			r_matrix[column * 4 + row] = p_projection.columns[column][row];
		}
	}
}

static void psp_store_transform(const Transform3D &p_transform, float *r_matrix) {
	for (int column = 0; column < 3; column++) {
		for (int row = 0; row < 3; row++) {
			r_matrix[column * 4 + row] = p_transform.basis.rows[row][column];
		}
		r_matrix[column * 4 + 3] = 0.0f;
	}
	r_matrix[12] = p_transform.origin.x;
	r_matrix[13] = p_transform.origin.y;
	r_matrix[14] = p_transform.origin.z;
	r_matrix[15] = 1.0f;
}

RenderGeometryInstance *RasterizerScenePSP::geometry_instance_create(RID p_base) {
	RSE::InstanceType type = RendererDummy::Utilities::get_singleton()->get_base_type(p_base);
	ERR_FAIL_COND_V(type != RSE::INSTANCE_MESH, nullptr);

	GeometryInstancePSP *instance = geometry_instance_alloc.alloc();
	instance->base = p_base;
	return instance;
}

void RasterizerScenePSP::geometry_instance_free(RenderGeometryInstance *p_geometry_instance) {
	GeometryInstancePSP *instance = static_cast<GeometryInstancePSP *>(p_geometry_instance);
	ERR_FAIL_NULL(instance);
	geometry_instance_alloc.free(instance);
}

const Vector<RasterizerScenePSP::CachedSurface> *RasterizerScenePSP::_get_mesh_surfaces(RID p_mesh) {
	RendererDummy::MeshStorage *mesh_storage = RendererDummy::MeshStorage::get_singleton();
	ERR_FAIL_NULL_V(mesh_storage, nullptr);
	RendererDummy::DummyMesh *mesh = mesh_storage->get_mesh(p_mesh);
	ERR_FAIL_NULL_V(mesh, nullptr);

	Vector<CachedSurface> *cached = mesh_cache.getptr(p_mesh);
	if (cached && cached->size() == mesh->surfaces.size()) {
		return cached;
	}

	Vector<CachedSurface> surfaces;
	surfaces.resize(mesh->surfaces.size());
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	ERR_FAIL_NULL_V(rendering_server, nullptr);
	for (int i = 0; i < mesh->surfaces.size(); i++) {
		const RenderingServerTypes::SurfaceData &source = mesh->surfaces[i];
		const Array arrays = rendering_server->mesh_create_arrays_from_surface_data(source);
		CachedSurface &surface = surfaces.write[i];
		surface.primitive = source.primitive;
		surface.vertices = arrays[RSE::ARRAY_VERTEX];
		surface.normals = arrays[RSE::ARRAY_NORMAL];
		surface.uvs = arrays[RSE::ARRAY_TEX_UV];
		surface.colors = arrays[RSE::ARRAY_COLOR];
		surface.indices = arrays[RSE::ARRAY_INDEX];
		bool indices_fit = true;
		surface.indices_16.resize(surface.indices.size());
		for (int index = 0; index < surface.indices.size(); index++) {
			if (surface.indices[index] < 0 || surface.indices[index] > 65535) {
				indices_fit = false;
				break;
			}
			surface.indices_16.write[index] = static_cast<uint16_t>(surface.indices[index]);
		}
		if (!indices_fit) {
			surface.indices_16.clear();
		}
	}
	mesh_cache.insert(p_mesh, surfaces);
	return mesh_cache.getptr(p_mesh);
}

bool RasterizerScenePSP::_bind_texture(RID p_texture) {
	ERR_FAIL_NULL_V(texture_cache, false);
	return texture_cache->bind(p_texture, true) != nullptr;
}

RID RasterizerScenePSP::_get_surface_material(const GeometryInstancePSP *p_instance, int p_surface) const {
	if (p_instance->material_override.is_valid()) {
		return p_instance->material_override;
	}
	if (p_surface < p_instance->surface_materials.size() && p_instance->surface_materials[p_surface].is_valid()) {
		return p_instance->surface_materials[p_surface];
	}
	RendererDummy::MeshStorage *mesh_storage = RendererDummy::MeshStorage::get_singleton();
	return mesh_storage ? mesh_storage->mesh_surface_get_material(p_instance->base, p_surface) : RID();
}

void RasterizerScenePSP::_setup_lights(const PagedArray<RID> &p_lights) {
	RendererDummy::LightStorage *light_storage = RendererDummy::LightStorage::get_singleton();
	ERR_FAIL_NULL(light_storage);

	const GLfloat ambient[4] = { 0.18f, 0.18f, 0.18f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
	int enabled_count = 0;
	for (uint64_t i = 0; i < p_lights.size() && enabled_count < 4; i++) {
		const RID instance_rid = p_lights[i];
		const RID light_rid = light_storage->light_instance_get_base(instance_rid);
		if (!light_rid.is_valid()) {
			continue;
		}

		const GLenum gl_light = GL_LIGHT0 + enabled_count;
		const Color color = light_storage->light_get_color(light_rid);
		const float energy = light_storage->light_get_param(light_rid, RSE::LIGHT_PARAM_ENERGY);
		const GLfloat diffuse[4] = { color.r * energy, color.g * energy, color.b * energy, 1.0f };
		const GLfloat no_ambient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		const GLfloat no_specular[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glLightfv(gl_light, GL_DIFFUSE, diffuse);
		glLightfv(gl_light, GL_AMBIENT, no_ambient);
		glLightfv(gl_light, GL_SPECULAR, no_specular);

		const Transform3D transform = light_storage->light_instance_get_transform(instance_rid);
		const RSE::LightType type = light_storage->light_get_type(light_rid);
		if (type == RSE::LIGHT_DIRECTIONAL) {
			const Vector3 direction_to_light = transform.basis.get_column(2).normalized();
			const GLfloat position[4] = { direction_to_light.x, direction_to_light.y, direction_to_light.z, 0.0f };
			glLightfv(gl_light, GL_POSITION, position);
			glLightf(gl_light, GL_CONSTANT_ATTENUATION, 1.0f);
			glLightf(gl_light, GL_LINEAR_ATTENUATION, 0.0f);
			glLightf(gl_light, GL_QUADRATIC_ATTENUATION, 0.0f);
			glLightf(gl_light, GL_SPOT_CUTOFF, 180.0f);
		} else {
			const GLfloat position[4] = { transform.origin.x, transform.origin.y, transform.origin.z, 1.0f };
			glLightfv(gl_light, GL_POSITION, position);
			const float range = MAX(0.01f, light_storage->light_get_param(light_rid, RSE::LIGHT_PARAM_RANGE));
			const float attenuation = MAX(0.01f, light_storage->light_get_param(light_rid, RSE::LIGHT_PARAM_ATTENUATION));
			glLightf(gl_light, GL_CONSTANT_ATTENUATION, 1.0f);
			glLightf(gl_light, GL_LINEAR_ATTENUATION, attenuation * 2.0f / range);
			glLightf(gl_light, GL_QUADRATIC_ATTENUATION, attenuation / (range * range));
			if (type == RSE::LIGHT_SPOT) {
				const Vector3 direction = -transform.basis.get_column(2).normalized();
				const GLfloat spot_direction[3] = { direction.x, direction.y, direction.z };
				glLightfv(gl_light, GL_SPOT_DIRECTION, spot_direction);
				glLightf(gl_light, GL_SPOT_CUTOFF, MIN(90.0f, light_storage->light_get_param(light_rid, RSE::LIGHT_PARAM_SPOT_ANGLE)));
				glLightf(gl_light, GL_SPOT_EXPONENT, CLAMP(light_storage->light_get_param(light_rid, RSE::LIGHT_PARAM_SPOT_ATTENUATION) * 16.0f, 0.0f, 128.0f));
			} else {
				glLightf(gl_light, GL_SPOT_CUTOFF, 180.0f);
			}
		}
		glEnable(gl_light);
		enabled_count++;
	}

	for (int i = enabled_count; i < 4; i++) {
		glDisable(GL_LIGHT0 + i);
	}
	lights_active = enabled_count > 0;
	if (lights_active) {
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_NORMALIZE);
	} else {
		glDisable(GL_LIGHTING);
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_NORMALIZE);
	}
}

void RasterizerScenePSP::_draw_surface(const CachedSurface &p_surface, RID p_material) {
	if (p_surface.vertices.is_empty()) {
		return;
	}

	Color albedo(1, 1, 1, 1);
	RID albedo_texture;
	float alpha_scissor = 0.0f;
	bool transparent = false;
	bool unshaded = false;
	RSE::CullMode cull_mode = RSE::CULL_MODE_BACK;
	RendererDummy::MaterialStorage *material_storage = RendererDummy::MaterialStorage::get_singleton();
	if (material_storage && p_material.is_valid()) {
		const Variant albedo_value = material_storage->material_get_param(p_material, "albedo");
		if (albedo_value.get_type() == Variant::COLOR) {
			albedo = albedo_value;
		}
		const Variant texture_value = material_storage->material_get_param(p_material, "texture_albedo");
		if (texture_value.get_type() == Variant::RID) {
			albedo_texture = texture_value;
		}
		const Variant alpha_scissor_value = material_storage->material_get_param(p_material, "alpha_scissor_threshold");
		if (alpha_scissor_value.get_type() == Variant::FLOAT) {
			alpha_scissor = alpha_scissor_value;
		}
		transparent = material_storage->material_uses_alpha(p_material) && alpha_scissor <= 0.0f;
		unshaded = material_storage->material_is_unshaded(p_material);
		cull_mode = material_storage->material_get_cull_mode(p_material);
	}
	if (cull_mode == RSE::CULL_MODE_DISABLED) {
		glDisable(GL_CULL_FACE);
	} else {
		glEnable(GL_CULL_FACE);
		glCullFace(cull_mode == RSE::CULL_MODE_FRONT ? GL_FRONT : GL_BACK);
	}
	const bool textured = _bind_texture(albedo_texture) && p_surface.uvs.size() == p_surface.vertices.size();
	if (textured) {
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
	if (alpha_scissor > 0.0f) {
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, alpha_scissor);
	} else {
		glDisable(GL_ALPHA_TEST);
	}
	if (transparent || albedo.a < 0.999f) {
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
	} else {
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}

	const bool has_normals = p_surface.normals.size() == p_surface.vertices.size();
	if (lights_active && has_normals && !unshaded) {
		glEnable(GL_LIGHTING);
	} else {
		glDisable(GL_LIGHTING);
	}

	GLenum primitive = GL_TRIANGLES;
	switch (p_surface.primitive) {
		case RSE::PRIMITIVE_POINTS: primitive = GL_POINTS; break;
		case RSE::PRIMITIVE_LINES: primitive = GL_LINES; break;
		case RSE::PRIMITIVE_LINE_STRIP: primitive = GL_LINE_STRIP; break;
		case RSE::PRIMITIVE_TRIANGLE_STRIP: primitive = GL_TRIANGLE_STRIP; break;
		default: break;
	}

	const bool can_use_arrays = p_surface.indices.is_empty() || !p_surface.indices_16.is_empty();
	if (can_use_arrays) {
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(Vector3), p_surface.vertices.ptr());
		if (has_normals) {
			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT, sizeof(Vector3), p_surface.normals.ptr());
		} else {
			glDisableClientState(GL_NORMAL_ARRAY);
		}
		if (textured) {
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, sizeof(Vector2), p_surface.uvs.ptr());
		} else {
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		if (p_surface.colors.size() == p_surface.vertices.size()) {
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(4, GL_FLOAT, sizeof(Color), p_surface.colors.ptr());
		} else {
			glDisableClientState(GL_COLOR_ARRAY);
			glColor4f(albedo.r, albedo.g, albedo.b, albedo.a);
		}

		if (p_surface.indices.is_empty()) {
			glDrawArrays(primitive, 0, p_surface.vertices.size());
		} else {
			glDrawElements(primitive, p_surface.indices_16.size(), GL_UNSIGNED_SHORT, p_surface.indices_16.ptr());
		}
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDepthMask(GL_TRUE);
		return;
	}

	glBegin(primitive);
	const int element_count = p_surface.indices.is_empty() ? p_surface.vertices.size() : p_surface.indices.size();
	for (int element = 0; element < element_count; element++) {
		const int vertex_index = p_surface.indices.is_empty() ? element : p_surface.indices[element];
		if (vertex_index < 0 || vertex_index >= p_surface.vertices.size()) {
			continue;
		}
		const Color color = p_surface.colors.size() == p_surface.vertices.size() ? albedo * p_surface.colors[vertex_index] : albedo;
		glColor4f(color.r, color.g, color.b, color.a);
		if (p_surface.normals.size() == p_surface.vertices.size()) {
			const Vector3 normal = p_surface.normals[vertex_index];
			glNormal3f(normal.x, normal.y, normal.z);
		}
		if (textured) {
			const Vector2 uv = p_surface.uvs[vertex_index];
			glTexCoord2f(uv.x, uv.y);
		}
		const Vector3 vertex = p_surface.vertices[vertex_index];
		glVertex3f(vertex.x, vertex.y, vertex.z);
	}
	glEnd();
	glDepthMask(GL_TRUE);
}

void RasterizerScenePSP::render_scene(const Ref<RenderSceneBuffers> &, const CameraData *p_camera_data, const CameraData *, const PagedArray<RenderGeometryInstance *> &p_instances, const PagedArray<RID> &p_lights, const PagedArray<RID> &, const PagedArray<RID> &, const PagedArray<RID> &, const PagedArray<RID> &, const PagedArray<RID> &, RID p_environment, RID, RID, RID, RID, RID, RID, int, float, const RenderShadowData *, int, const RenderSDFGIData *, int, float, const RenderSDFGIUpdateData *, RenderingServerTypes::RenderInfo *) {
	ERR_FAIL_NULL(p_camera_data);
	if (p_environment.is_valid()) {
		const Color background = environment_get_bg_color(p_environment);
		glClearColor(background.r, background.g, background.b, background.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	float projection[16];
	psp_store_projection(p_camera_data->main_projection, projection);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(projection);

	float view[16];
	psp_store_transform(p_camera_data->main_transform.affine_inverse(), view);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(view);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	_setup_lights(p_lights);

	for (uint64_t i = 0; i < p_instances.size(); i++) {
		GeometryInstancePSP *instance = static_cast<GeometryInstancePSP *>(p_instances[i]);
		if (!instance) {
			continue;
		}
		const Vector<CachedSurface> *surfaces = _get_mesh_surfaces(instance->base);
		if (!surfaces) {
			continue;
		}
		float model[16];
		psp_store_transform(instance->transform, model);
		glPushMatrix();
		glMultMatrixf(model);
		for (int surface = 0; surface < surfaces->size(); surface++) {
			_draw_surface((*surfaces)[surface], _get_surface_material(instance, surface));
		}
		glPopMatrix();
	}

	glDisable(GL_CULL_FACE);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_NORMALIZE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

void RasterizerScenePSP::clear_mesh_cache() {
	mesh_cache.clear();
}
