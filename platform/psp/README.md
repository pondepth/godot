# PSP platform backend

This directory contains the experimental Godot 4.7 PSP runtime target.

The implementation borrows the proven platform shape from Homebrodot's Godot 2.1 branch—PSP OS entry point, PSPGL, controller polling and `EBOOT.PBP` packaging—but targets Godot 4's current display and rendering interfaces.

The first scene backend supports static `MeshInstance3D` geometry, perspective and orthographic cameras, depth testing, vertex colors, UVs, alpha scissor, the albedo color/texture of `StandardMaterial3D`, and up to four directional/omni/spot lights. Godot Physics 3D is included. See `demo_3d/` for a minimal rigid-body project.

See the repository-level `PSP_PORT_STATUS.md` for supported features, build commands and the roadmap.
