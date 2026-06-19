/**************************************************************************/
/*  light_storage.cpp                                                     */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "light_storage.h"

using namespace RendererDummy;

LightStorage *LightStorage::singleton = nullptr;

LightStorage *LightStorage::get_singleton() {
	return singleton;
}

LightStorage::LightStorage() {
	singleton = this;
}

LightStorage::~LightStorage() {
	singleton = nullptr;
}

bool LightStorage::free(RID p_rid) {
	if (owns_light(p_rid)) {
		light_free(p_rid);
		return true;
	} else if (owns_light_instance(p_rid)) {
		light_instance_free(p_rid);
		return true;
	} else if (owns_lightmap(p_rid)) {
		lightmap_free(p_rid);
		return true;
	} else if (owns_lightmap_instance(p_rid)) {
		lightmap_instance_free(p_rid);
		return true;
	}

	return false;
}

RID LightStorage::_light_allocate(RSE::LightType) {
	return light_owner.allocate_rid();
}

void LightStorage::_light_initialize(RID p_rid, RSE::LightType p_type) {
	Light light;
	light.type = p_type;
	light.params[RSE::LIGHT_PARAM_ENERGY] = 1.0f;
	light.params[RSE::LIGHT_PARAM_INDIRECT_ENERGY] = 1.0f;
	light.params[RSE::LIGHT_PARAM_SPECULAR] = 0.5f;
	light.params[RSE::LIGHT_PARAM_RANGE] = 5.0f;
	light.params[RSE::LIGHT_PARAM_ATTENUATION] = 1.0f;
	light.params[RSE::LIGHT_PARAM_SPOT_ANGLE] = 45.0f;
	light.params[RSE::LIGHT_PARAM_SPOT_ATTENUATION] = 1.0f;
	light_owner.initialize_rid(p_rid, light);
}

void LightStorage::light_free(RID p_rid) {
	Light *light = light_owner.get_or_null(p_rid);
	ERR_FAIL_NULL(light);
	light->dependency.deleted_notify(p_rid);
	light_owner.free(p_rid);
}

void LightStorage::light_set_color(RID p_light, const Color &p_color) {
	Light *light = light_owner.get_or_null(p_light);
	ERR_FAIL_NULL(light);
	light->color = p_color;
	light->version++;
	light->dependency.changed_notify(Dependency::DEPENDENCY_CHANGED_LIGHT);
}

void LightStorage::light_set_param(RID p_light, RSE::LightParam p_param, float p_value) {
	Light *light = light_owner.get_or_null(p_light);
	ERR_FAIL_NULL(light);
	ERR_FAIL_INDEX(p_param, RSE::LIGHT_PARAM_MAX);
	light->params[p_param] = p_value;
	light->version++;
	light->dependency.changed_notify(Dependency::DEPENDENCY_CHANGED_LIGHT);
}

void LightStorage::light_set_shadow(RID p_light, bool p_enabled) {
	Light *light = light_owner.get_or_null(p_light);
	ERR_FAIL_NULL(light);
	light->shadow = p_enabled;
	light->version++;
}

void LightStorage::light_set_negative(RID p_light, bool p_enable) {
	Light *light = light_owner.get_or_null(p_light);
	ERR_FAIL_NULL(light);
	light->negative = p_enable;
	light->version++;
}

void LightStorage::light_set_cull_mask(RID p_light, uint32_t p_mask) {
	Light *light = light_owner.get_or_null(p_light);
	ERR_FAIL_NULL(light);
	light->cull_mask = p_mask;
	light->version++;
}

void LightStorage::light_set_shadow_caster_mask(RID p_light, uint32_t p_caster_mask) {
	Light *light = light_owner.get_or_null(p_light);
	ERR_FAIL_NULL(light);
	light->shadow_caster_mask = p_caster_mask;
}

uint32_t LightStorage::light_get_shadow_caster_mask(RID p_light) const {
	const Light *light = light_owner.get_or_null(p_light);
	ERR_FAIL_NULL_V(light, 0xFFFFFFFF);
	return light->shadow_caster_mask;
}

void LightStorage::light_set_bake_mode(RID p_light, RSE::LightBakeMode p_bake_mode) {
	Light *light = light_owner.get_or_null(p_light);
	ERR_FAIL_NULL(light);
	light->bake_mode = p_bake_mode;
	light->version++;
}

bool LightStorage::light_has_shadow(RID p_light) const {
	const Light *light = light_owner.get_or_null(p_light);
	ERR_FAIL_NULL_V(light, false);
	return light->shadow;
}

RSE::LightType LightStorage::light_get_type(RID p_light) const {
	const Light *light = light_owner.get_or_null(p_light);
	ERR_FAIL_NULL_V(light, RSE::LIGHT_OMNI);
	return light->type;
}

AABB LightStorage::light_get_aabb(RID p_light) const {
	const Light *light = light_owner.get_or_null(p_light);
	ERR_FAIL_NULL_V(light, AABB());
	if (light->type == RSE::LIGHT_DIRECTIONAL) {
		return AABB(Vector3(-1e6f, -1e6f, -1e6f), Vector3(2e6f, 2e6f, 2e6f));
	}
	const float range = light->params[RSE::LIGHT_PARAM_RANGE];
	return AABB(Vector3(-range, -range, -range), Vector3(range * 2.0f, range * 2.0f, range * 2.0f));
}

float LightStorage::light_get_param(RID p_light, RSE::LightParam p_param) {
	const Light *light = light_owner.get_or_null(p_light);
	ERR_FAIL_NULL_V(light, 0.0f);
	ERR_FAIL_INDEX_V(p_param, RSE::LIGHT_PARAM_MAX, 0.0f);
	return light->params[p_param];
}

Color LightStorage::light_get_color(RID p_light) {
	const Light *light = light_owner.get_or_null(p_light);
	ERR_FAIL_NULL_V(light, Color());
	return light->negative ? Color(-light->color.r, -light->color.g, -light->color.b, light->color.a) : light->color;
}

RSE::LightBakeMode LightStorage::light_get_bake_mode(RID p_light) {
	const Light *light = light_owner.get_or_null(p_light);
	ERR_FAIL_NULL_V(light, RSE::LIGHT_BAKE_DISABLED);
	return light->bake_mode;
}

uint64_t LightStorage::light_get_version(RID p_light) const {
	const Light *light = light_owner.get_or_null(p_light);
	ERR_FAIL_NULL_V(light, 0);
	return light->version;
}

uint32_t LightStorage::light_get_cull_mask(RID p_light) const {
	const Light *light = light_owner.get_or_null(p_light);
	ERR_FAIL_NULL_V(light, 0);
	return light->cull_mask;
}

RID LightStorage::light_instance_create(RID p_light) {
	ERR_FAIL_COND_V(!owns_light(p_light), RID());
	LightInstance instance;
	instance.light = p_light;
	return light_instance_owner.make_rid(instance);
}

void LightStorage::light_instance_free(RID p_light_instance) {
	ERR_FAIL_COND(!owns_light_instance(p_light_instance));
	light_instance_owner.free(p_light_instance);
}

void LightStorage::light_instance_set_transform(RID p_light_instance, const Transform3D &p_transform) {
	LightInstance *instance = light_instance_owner.get_or_null(p_light_instance);
	ERR_FAIL_NULL(instance);
	instance->transform = p_transform;
}

void LightStorage::light_instance_set_aabb(RID p_light_instance, const AABB &p_aabb) {
	LightInstance *instance = light_instance_owner.get_or_null(p_light_instance);
	ERR_FAIL_NULL(instance);
	instance->aabb = p_aabb;
}

RID LightStorage::light_instance_get_base(RID p_light_instance) const {
	const LightInstance *instance = light_instance_owner.get_or_null(p_light_instance);
	ERR_FAIL_NULL_V(instance, RID());
	return instance->light;
}

Transform3D LightStorage::light_instance_get_transform(RID p_light_instance) const {
	const LightInstance *instance = light_instance_owner.get_or_null(p_light_instance);
	ERR_FAIL_NULL_V(instance, Transform3D());
	return instance->transform;
}

void LightStorage::light_update_dependency(RID p_light, DependencyTracker *p_instance) {
	Light *light = light_owner.get_or_null(p_light);
	ERR_FAIL_NULL(light);
	ERR_FAIL_NULL(p_instance);
	p_instance->update_dependency(&light->dependency);
}

/* LIGHTMAP API */

RID LightStorage::lightmap_allocate() {
	return lightmap_owner.allocate_rid();
}

void LightStorage::lightmap_initialize(RID p_lightmap) {
	lightmap_owner.initialize_rid(p_lightmap, Lightmap());
}

void LightStorage::lightmap_free(RID p_rid) {
	lightmap_set_textures(p_rid, RID(), false);
	lightmap_owner.free(p_rid);
}

/* LIGHTMAP INSTANCE */

RID LightStorage::lightmap_instance_create(RID p_lightmap) {
	LightmapInstance li;
	li.lightmap = p_lightmap;
	return lightmap_instance_owner.make_rid(li);
}

void LightStorage::lightmap_instance_free(RID p_lightmap) {
	lightmap_instance_owner.free(p_lightmap);
}
