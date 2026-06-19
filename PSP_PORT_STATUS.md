# Godot 4.7 PSP port status

This repository is based on the official `4.7-stable` tag and contains the first vertical slice of a native PSP target. It is not yet a drop-in replacement for the mature Godot 2.1 Homebrodot port.

## What is implemented

- `platform=psp` SCons target for the PSPDEV Allegrex/MIPS toolchain.
- `mips32` as a recognized Godot architecture.
- PSP entry point, exit callback, 333 MHz clock setup, filesystem base and main loop.
- A 480 x 272 display server using PSPGL/GLUT.
- PSP controls exposed as joypad 0, including the analog nub and all face/d-pad/shoulder buttons.
- An experimental 2D renderer for unshaded `CanvasItem` rectangles and simple primitives.
- A fixed-function low-poly 3D path for static `MeshInstance3D` surfaces: camera transforms, indexed triangles/strips/lines, Z-buffering, vertex colors, UVs, alpha scissor and `StandardMaterial3D` albedo color/texture.
- Up to four visible `DirectionalLight3D`, `OmniLight3D` or `SpotLight3D` nodes through PSPGL fixed-function lighting.
- Cached client-array submission with 16-bit mesh indices instead of per-vertex immediate-mode calls.
- Built-in Godot Physics 3D for collision bodies and ray queries.
- An editor exporter that writes `EBOOT.PBP` and a separate `data.pck` beside it.
- Automatic `EBOOT.PBP` creation with PSPDEV tools.

## Deliberate limitations

- Shadows, custom spatial shaders, correct transparency sorting, skinning, blend shapes, particles, MultiMesh, runtime mesh mutation and post-processing are not implemented.
- Materials currently use only `albedo`, `texture_albedo` and alpha scissor; bake detailed lighting into small textures or vertex colors.
- No canvas shaders, lights, shadows, particles, polygons, nine-patches, meshes, viewport textures or CanvasGroup effects yet.
- Text rendering has not been validated on hardware.
- Audio currently falls back to Godot's dummy driver, so Ogg/Vorbis are also excluded from the default build. A PSP audio driver is the next isolated subsystem.
- No networking, GDExtension, C#, editor-on-device or remote deploy.
- The current entropy source is the PSP SDK Mersenne Twister seeded from the system timer; it is suitable for engine seeding, not cryptography.
- The memory target is PSP-2000/3000 (64 MiB). PSP-1000's 32 MiB is not currently realistic for Godot 4.
- PSPGL is required in addition to PSPDEV/PSPSDK.

The important distinction is that this is a real Godot 4 runtime port scaffold, not a Godot 4-to-2.1 project converter. Godot 4 scenes and `data.pck` files stay in the Godot 4 format.

## Build

Build inside a Linux or WSL environment with current PSPDEV, PSPSDK and PSPGL installed and `PSPDEV` exported:

```bash
export PSPDEV=/usr/local/pspdev
export PATH="$PSPDEV/bin:$PATH"
python3 misc/scripts/build_psp.py --configuration both
```

Equivalent release command:

```bash
scons platform=psp target=template_release arch=mips32 \
  modules_enabled_by_default=no module_gdscript_enabled=yes \
  module_freetype_enabled=yes module_text_server_fb_enabled=yes \
  module_godot_physics_3d_enabled=yes disable_3d=no disable_physics_3d=no \
  disable_navigation_3d=yes \
  disable_xr=yes disable_advanced_gui=yes optimize=size lto=none eboot
```

The helper leaves `psp_debug.pbp` and/or `psp_release.pbp` in `bin/`. Put these files in the active Godot editor's export-template directory, or select them as custom templates in a PSP export preset.

The `PSP templates` GitHub Actions workflow performs the same build in the official `pspdev/pspdev` container, installs PSPGL, and uploads both PBP templates as an artifact. It can be started manually with **Run workflow** after pushing this branch.

## Create and export a game

Use these project constraints:

- viewport: 480 x 272;
- renderer project setting: `dummy` (the PSP display server replaces the dummy canvas with its native backend);
- unshaded `Sprite2D`, `ColorRect`, basic `Control`/`Node2D` drawing;
- low-poly static `MeshInstance3D` models with a `Camera3D`;
- `StandardMaterial3D` with albedo color, one albedo texture and optional alpha scissor;
- at most four visible directional/omni/spot lights, without shadows;
- keep scenes small, avoid transparency, and prefer indexed triangle meshes;
- small RGBA textures, preferably power-of-two and no larger than 512 x 512;
- GDScript only.

The source-only smoke test in `platform/psp/demo_3d/` creates a lit, textured rigid body falling onto a static floor. The analog nub applies torque. Open it in a desktop Godot 4.7 editor, create a PSP export preset, and export to `EBOOT.PBP`.

In the desktop Godot editor, create a **PSP** export preset and export to `EBOOT.PBP`. The exporter writes:

```text
EBOOT.PBP
data.pck
```

Copy both files to:

```text
/PSP/GAME/YourGame/
```

Test first in PPSSPP with logging enabled, then on a PSP-2000/3000.

## Why the Godot 2.1 code was not copied wholesale

The Homebrodot 2.1 port implements one `Rasterizer` interface and uses PSPGL. Godot 4.7 divides rendering among compositor, canvas, scene, texture, mesh, material, particles, light and rendering-device layers. The portable parts were the platform split, controller mapping, PSPGL approach, EBOOT packaging and `data.pck` layout. The old rasterizer itself is API-incompatible, so this fork starts with the existing Godot 4 dummy storage backend and replaces its canvas and scene paths. Godot 4 still owns resources and decodes imported mesh buffers; the PSP scene backend submits the supported subset to PSPGL.

## Next milestones

1. Build and boot `platform/psp/demo_3d` in PPSSPP and on a PSP-2000/3000.
2. Validate projection orientation, texture upload, depth, clipping and input on hardware.
3. Add a bounded PSP audio ring buffer and driver.
4. Move 3D submission from immediate mode to cached PSP GU/PSPGL vertex arrays.
5. Add distance cutoffs, transparent-surface sorting and a compact baked-light workflow.
6. Measure binary/RAM budgets and create a PSP-specific build profile.

## Reference sources

- [Godot 2.1.6 stable](https://github.com/godotengine/godot/tree/2.1.6-stable)
- [Homebrodot Godot 2.1 PSP platform at the reviewed revision](https://github.com/Homebrodot/Godot/tree/f5608343a1d8ab6a3892c80a0c922a8c92d5e3d5/platform/psp)
- [Godot 4.7 stable](https://github.com/godotengine/godot/tree/4.7-stable)
- [PSPDEV toolchain](https://github.com/pspdev/pspdev)
