# Wii GX render backend notes

Current render abstraction boundaries in this tree:

- **Renderer init**: `RenderBackend::init` configures fixed-function GX state; Wii startup/VI/GX bootstrapping happens in `handheld/src/main_wii.cpp`.
- **Texture upload**: `Textures::assignTexture` and `Textures::tick` in `handheld/src/client/renderer/Textures.cpp`.
- **Draw submission / batching**: `Tesselator::end` in `handheld/src/client/renderer/Tesselator.cpp` and GX submission helpers in `handheld/src/client/renderer/RenderBackend.cpp` + `renderer_gx.cpp`.
- **State transitions + matrix setup**: `GameRenderer` camera/gui setup and render pass flow in `handheld/src/client/renderer/GameRenderer.cpp`.

## Strict rendering boundary

- High-level gameplay/client code must call only renderer engine interfaces (`RenderBackend`, renderer abstractions, or helper wrappers built on top of them).
- Platform graphics APIs (`gl*`, `egl*`, `GX*`, and corresponding constants) are backend-only implementation details.
- The non-Wii GL path remains supported as an **optional legacy backend** behind `RenderBackend`; direct GL calls are still forbidden in high-level client gameplay/render code.

## GX-only mapping

The Wii target now routes fixed-function operations through the GX backend (no GLES dependency in the Wii build).

- Texture creation/upload/filtering
  - `RenderBackend::configureTextureSampling`
  - `RenderBackend::uploadTexture2D`
  - `RenderBackend::updateTexture2D`
- Depth/cull/blend state
  - `RenderBackend::setDepthState`
  - `RenderBackend::setCullState`
  - `RenderBackend::setBlendState`
- Batched quad/chunk submission
  - `RenderBackend::bindArrayBuffer`
  - `RenderBackend::uploadArrayBuffer`
  - `RenderBackend::submitTexturedMesh`
- Matrix setup / projection
  - `RenderBackend::setPerspective`
  - `RenderBackend::setOrtho`

## Migration phases and done criteria

Every phase has the same done criteria:

1. zero direct `gl*` (or other API-specific graphics token) usage in that phase subtree.
2. all render state / projection / matrix operations routed via backend interfaces.

Phases:

- **Phase A: GUI / HUD / screens**
  - subtree: `handheld/src/client/gui/**`
  - check command: `tools/check_render_api_boundaries.py --phase A`
- **Phase B: entity / item renderers**
  - subtree: entity/item renderer and model paths (see checker phase definition)
  - check command: `tools/check_render_api_boundaries.py --phase B`
- **Phase C: level / world rendering**
  - subtree: level/chunk/tile/tesselator/culling renderer paths (see checker phase definition)
  - check command: `tools/check_render_api_boundaries.py --phase C`
- **Phase D: particles / effects / debug overlays**
  - subtree: `handheld/src/client/particle/**` + perf/debug overlay helpers
  - check command: `tools/check_render_api_boundaries.py --phase D`

After phase completion, run:

- `tools/check_render_api_boundaries.py --phase all`

## Final tree-wide confirmation

After all phases are complete, run a tree-wide token scan to ensure only backend implementation files contain platform API graphics calls:

- `tools/check_render_api_boundaries.py --final`

The checker enforces a backend-only allowlist for API-specific calls (RenderBackend implementation, GX backend files, and platform bootstrap files).

## Compatibility layer for shader-era assumptions

`RenderBackend::shaderCompatUseProgram` and
`RenderBackend::shaderCompatSetUniformMat4` are no-op compatibility hooks for fixed-function targets (including GX), so shader-era call sites can be routed through a predictable API without requiring programmable pipeline support.
