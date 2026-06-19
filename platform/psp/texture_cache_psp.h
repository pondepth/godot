/**************************************************************************/
/*  texture_cache_psp.h                                                   */
/**************************************************************************/

#pragma once

#include "core/templates/hash_map.h"
#include "core/templates/rid.h"

#ifdef WINDOWS_ENABLED
#include <windows.h>
#endif
#include <GL/gl.h>

class PSPTextureCache {
public:
	struct Texture {
		GLuint id = 0;
		int width = 0;
		int height = 0;
	};

private:
	HashMap<RID, Texture> textures;

public:
	const Texture *bind(RID p_texture, bool p_repeat);
	void clear();
};
