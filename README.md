# display_tool

A lightweight viewer using GLFW + OpenGL with optional Nuklear UI.

Features:
- 2D view: pan with left-drag, scroll to zoom
- 3D view: orbit with right-drag, scroll to dolly (hold TAB to zoom 3D)
- Toggle between 2D/3D with SPACE
- Optional Nuklear-based control panel (if `nuklear.h` and backends are available)

## Build

Requirements:
- CMake >= 3.16
- OpenGL
- GLFW 3
- (Optional) Nuklear headers (`nuklear.h`, `nuklear_glfw_gl2.h`)

Commands:
```bash
mkdir -p build && cd build
cmake -DENABLE_NUKLEAR=ON ..
cmake --build . -j
./display_tool
```

If Nuklear is not found, the app still builds without UI.

## Notes
- On some systems you may need development packages, e.g. Ubuntu:
  ```bash
  sudo apt-get install libglfw3-dev libglew-dev mesa-common-dev 
  ```
