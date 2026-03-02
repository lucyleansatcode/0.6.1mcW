# Wii GX render backend notes

Current render abstraction boundaries in this tree:

- **Renderer init**: `glInit` in `handheld/src/client/renderer/gles.cpp` (desktop GL loader) and `RenderBackend::init` in `RenderBackend.cpp`.
- **Texture upload**: `Textures::assignTexture` and `Textures::tick` in `handheld/src/client/renderer/Textures.cpp`.
- **Draw submission / batching**: `Tesselator::end` in `handheld/src/client/renderer/Tesselator.cpp` and helper submission in `gles.cpp`.
- **State transitions + matrix setup**: `GameRenderer` camera/gui setup and render pass flow in `handheld/src/client/renderer/GameRenderer.cpp`.

## GLES-era to GX mapping

The new backend shim routes common fixed-function operations so Wii-specific GX paths can be provided behind one interface.

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

## Compatibility layer for shader-era assumptions

`RenderBackend::shaderCompatUseProgram` and
`RenderBackend::shaderCompatSetUniformMat4` are no-op compatibility hooks for fixed-function targets (including GX), so shader-era call sites can be routed through a predictable API without requiring programmable pipeline support.
