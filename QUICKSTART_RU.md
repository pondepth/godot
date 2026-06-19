# Godot 4.7 PSP — быстрый старт

1. Запустите `START_GODOT_PSP.cmd`. Откроется готовый 3D starter с камерой, светом, физикой, текстурированным rigid body и PSP-управлением.
2. Меняйте или заменяйте сцену как в обычном Godot 4.7. Целевое разрешение уже установлено в 480 × 272.
3. Для PSP используйте low-poly `MeshInstance3D`, `StandardMaterial3D`, текстуры до 512 × 512 и не более четырёх видимых источников света.
4. Откройте **Project → Export → PSP** и нажмите **Export Project**. Готовые файлы появятся в `platform/psp/demo_3d/build/`:

```text
EBOOT.PBP
data.pck
```

5. Скопируйте оба файла в `/PSP/GAME/YourGame/` или откройте `EBOOT.PBP` в PPSSPP.

Если редактор сообщает об отсутствующем template, нужны `bin/psp_debug.pbp` и `bin/psp_release.pbp`. Workflow `.github/workflows/psp.yml` собирает их в официальном PSPDEV-контейнере.

Текущие ограничения: нет теней, skeletal animation, custom spatial shaders, MultiMesh и корректной сортировки полупрозрачных поверхностей. Physics 3D, статические mesh-ресурсы, динамические transforms, camera, depth, albedo textures, alpha scissor и directional/omni/spot lights поддерживаются.
