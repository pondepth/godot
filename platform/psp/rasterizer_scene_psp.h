/**************************************************************************/
/*  rasterizer_scene_psp.h                                                */
/**************************************************************************/

#pragma once

#include "servers/rendering/dummy/rasterizer_scene_dummy.h"

#include "texture_cache_psp.h"

#include "core/templates/hash_map.h"
#include "core/templates/paged_allocator.h"

#ifdef WINDOWS_ENABLED
#include <windows.h>
#endif
#include <GL/gl.h>

class RasterizerScenePSP : public RasterizerSceneDummy {
	class GeometryInstancePSP : public RenderGeometryInstance {
	public:
		RID base;
		RID material_override;
		Vector<RID> surface_materials;
		Transform3D transform;
		AABB aabb;

		void _mark_dirty() override {}
		void set_skeleton(RID) override {}
		void set_material_override(RID p_override) override { material_override = p_override; }
		void set_material_overlay(RID) override {}
		void set_surface_materials(const Vector<RID> &p_materials) override { surface_materials = p_materials; }
		void set_mesh_instance(RID) override {}
		void set_transform(const Transform3D &p_transform, const AABB &p_aabb, const AABB &) override {
			transform = p_transform;
			aabb = p_aabb;
		}
		void set_pivot_data(float, bool) override {}
		void set_lod_bias(float) override {}
		void set_layer_mask(uint32_t) override {}
		void set_fade_range(bool, float, float, bool, float, float) override {}
		void set_parent_fade_alpha(float) override {}
		void set_transparency(float) override {}
		void set_use_baked_light(bool) override {}
		void set_use_dynamic_gi(bool) override {}
		void set_use_lightmap(RID, const Rect2 &, int) override {}
		void set_lightmap_capture(const Color *) override {}
		void set_instance_shader_uniforms_offset(int32_t) override {}
		void set_cast_double_sided_shadows(bool) override {}
		void reset_motion_vectors() override {}
		Transform3D get_transform() override { return transform; }
		AABB get_aabb() override { return aabb; }
		void clear_light_instances() override {}
		void pair_light_instance(const RID, RSE::LightType, uint32_t) override {}
		void pair_reflection_probe_instances(const RID *, uint32_t) override {}
		void pair_decal_instances(const RID *, uint32_t) override {}
		void pair_voxel_gi_instances(const RID *, uint32_t) override {}
		void set_softshadow_projector_pairing(bool, bool) override {}
	};

	struct CachedSurface {
		RSE::PrimitiveType primitive = RSE::PRIMITIVE_TRIANGLES;
		PackedVector3Array vertices;
		PackedVector3Array normals;
		PackedVector2Array uvs;
		PackedColorArray colors;
		PackedInt32Array indices;
		Vector<uint16_t> indices_16;
	};

	PagedAllocator<GeometryInstancePSP> geometry_instance_alloc;
	HashMap<RID, Vector<CachedSurface>> mesh_cache;
	PSPTextureCache *texture_cache = nullptr;
	bool lights_active = false;

	const Vector<CachedSurface> *_get_mesh_surfaces(RID p_mesh);
	bool _bind_texture(RID p_texture);
	RID _get_surface_material(const GeometryInstancePSP *p_instance, int p_surface) const;
	void _setup_lights(const PagedArray<RID> &p_lights);
	void _draw_surface(const CachedSurface &p_surface, RID p_material);

public:
	RasterizerScenePSP(PSPTextureCache *p_texture_cache) : texture_cache(p_texture_cache) {}
	RenderGeometryInstance *geometry_instance_create(RID p_base) override;
	void geometry_instance_free(RenderGeometryInstance *p_geometry_instance) override;
	uint32_t geometry_instance_get_pair_mask() override { return 0; }
	uint32_t get_max_lights_total() override { return 4; }
	uint32_t get_max_lights_per_mesh() override { return 4; }

	void render_scene(const Ref<RenderSceneBuffers> &p_render_buffers, const CameraData *p_camera_data, const CameraData *p_prev_camera_data, const PagedArray<RenderGeometryInstance *> &p_instances, const PagedArray<RID> &p_lights, const PagedArray<RID> &p_reflection_probes, const PagedArray<RID> &p_voxel_gi_instances, const PagedArray<RID> &p_decals, const PagedArray<RID> &p_lightmaps, const PagedArray<RID> &p_fog_volumes, RID p_environment, RID p_camera_attributes, RID p_compositor, RID p_shadow_atlas, RID p_occluder_debug_tex, RID p_reflection_atlas, RID p_reflection_probe, int p_reflection_probe_pass, float p_screen_mesh_lod_threshold, const RenderShadowData *p_render_shadows, int p_render_shadow_count, const RenderSDFGIData *p_render_sdfgi_regions, int p_render_sdfgi_region_count, float p_window_output_max_value, const RenderSDFGIUpdateData *p_sdfgi_update_data = nullptr, RenderingServerTypes::RenderInfo *r_info = nullptr) override;

	void clear_mesh_cache();
};
