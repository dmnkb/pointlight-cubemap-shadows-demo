# Pointlight Cubemap Shadows Demo

Entirely vibe coded referencing [https://learnopengl.com/](https://learnopengl.com/Advanced-Lighting/Shadows/Point-Shadows) showing **point light shadow mapping with cubemap arrays**.

Runs on macOS (OpenGL 4.1 core profile, the maximum supported version) and other platforms.

## Why OpenGL 4.1?

I work mostly on macOS. There, OpenGL support is frozen at **4.1 core profile**.  
This project is written to stay compatible with that constraint.

## Prerequisites

- **CMake ≥ 3.20**
- A C++23 compiler (tested with Clang on macOS)
- OpenGL 4.1-capable GPU (any Mac GPU, or modern GPU on other platforms)

All dependencies are vendored:

- [GLFW](https://www.glfw.org/) — window/context creation
- [GLAD](https://glad.dav1d.de/) — OpenGL function loader
- [GLM](https://github.com/g-truc/glm) — math library

## Building

```bash
git clone https://github.com/dmnkb/pointlight-cubemap-shadows-demo.git
cd pointlight-cubemap-shadows-demo
cmake -B build
cmake --build build
```

## Running

```bash
./build/PointLightShadow
```

You should see a simple room scene with multiple animated point lights, each casting real-time cubemap shadows.
