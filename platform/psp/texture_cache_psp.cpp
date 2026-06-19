/**************************************************************************/
/*  texture_cache_psp.cpp                                                 */
/**************************************************************************/

#include "texture_cache_psp.h"

#include "core/io/image.h"
#include "servers/rendering/dummy/storage/texture_storage.h"

const PSPTextureCache::Texture *PSPTextureCache::bind(RID p_texture, bool p_repeat) {
	if (!p_texture.is_valid()) {
		glDisable(GL_TEXTURE_2D);
		return nullptr;
	}

	Texture *cached = textures.getptr(p_texture);
	if (!cached) {
		RendererDummy::TextureStorage *storage = RendererDummy::TextureStorage::get_singleton();
		ERR_FAIL_NULL_V(storage, nullptr);
		Ref<Image> image = storage->texture_2d_get(p_texture);
		if (image.is_null() || image->is_empty()) {
			glDisable(GL_TEXTURE_2D);
			return nullptr;
		}

		image = image->duplicate();
		if (image->is_compressed() && image->decompress() != OK) {
			glDisable(GL_TEXTURE_2D);
			return nullptr;
		}
		if (image->get_format() != Image::FORMAT_RGBA8) {
			image->convert(Image::FORMAT_RGBA8);
		}

		Texture texture;
		texture.width = image->get_width();
		texture.height = image->get_height();
		if (image->get_width() > 512 || image->get_height() > 512) {
			const float scale = MIN(512.0f / image->get_width(), 512.0f / image->get_height());
			image->resize(MAX(1, static_cast<int>(image->get_width() * scale)), MAX(1, static_cast<int>(image->get_height() * scale)), Image::INTERPOLATE_BILINEAR);
		}
		image->resize_to_po2(false, Image::INTERPOLATE_NEAREST);

		glGenTextures(1, &texture.id);
		glBindTexture(GL_TEXTURE_2D, texture.id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		const PackedByteArray data = image->get_data();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->get_width(), image->get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, data.ptr());
		textures.insert(p_texture, texture);
		cached = textures.getptr(p_texture);
	}

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, cached->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, p_repeat ? GL_REPEAT : GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, p_repeat ? GL_REPEAT : GL_CLAMP);
	return cached;
}

void PSPTextureCache::clear() {
	for (const KeyValue<RID, Texture> &entry : textures) {
		if (entry.value.id) {
			GLuint texture = entry.value.id;
			glDeleteTextures(1, &texture);
		}
	}
	textures.clear();
}
