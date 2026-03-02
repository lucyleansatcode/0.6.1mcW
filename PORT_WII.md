# Wii Port Milestones

## Milestone 1: Boots to main loop and clears screen
- Goal: Bring up the Wii target, enter the game main loop, and reliably clear the framebuffer each frame.
- Exit criteria:
  - Binary launches on target hardware/emulator without immediate fault.
  - Main loop runs continuously and the screen clear color updates correctly.
- Manual checklist:
  - [ ] FPS target met (>= 30 FPS in empty loop).
  - [ ] Memory usage ceiling respected (<= 64 MB resident during loop soak).
  - [ ] No crashes during a 15+ minute session.

## Milestone 2: Renders static world chunk
- Goal: Display a prebuilt/static chunk with camera framing and basic depth/culling behavior.
- Exit criteria:
  - One static chunk is visible and stable on screen.
  - Geometry submission path works without per-frame corruption/artifacts.
- Manual checklist:
  - [ ] FPS target met (>= 30 FPS with one visible chunk).
  - [ ] Memory usage ceiling respected (<= 96 MB resident while rendering).
  - [ ] No crashes during a 15+ minute session.

## Milestone 3: Controller-only menu navigation
- Goal: Support complete menu traversal and selection using Wii controller inputs only.
- Exit criteria:
  - Main menu and settings navigation are usable without keyboard/touch input.
  - Focus movement, confirm, and back actions are mapped and debounced.
- Manual checklist:
  - [ ] FPS target met (>= 30 FPS while navigating menus).
  - [ ] Memory usage ceiling respected (<= 96 MB resident in menu flow).
  - [ ] No crashes during a 15+ minute session.

## Milestone 4: In-game movement/combat
- Goal: Enable player movement, camera look, and core combat interactions via controller.
- Exit criteria:
  - Player can move, jump, and look around with stable input handling.
  - Basic combat interactions (attack/use) execute correctly in-game.
- Manual checklist:
  - [ ] FPS target met (>= 25 FPS in typical gameplay scene).
  - [ ] Memory usage ceiling respected (<= 112 MB resident in active gameplay).
  - [ ] No crashes during a 15+ minute session.

## Milestone 5: Save/load cycle
- Goal: Persist world/player state and restore it on next launch.
- Exit criteria:
  - Save operation writes valid data to Wii storage path.
  - Load operation restores previously saved world/player state.
- Manual checklist:
  - [ ] FPS target met (>= 25 FPS before and after save/load).
  - [ ] Memory usage ceiling respected (<= 112 MB resident across cycle).
  - [ ] No crashes during a 15+ minute session.

## Milestone 6: Performance pass
- Goal: Profile and optimize the Wii build to stabilize frame-time and memory use.
- Exit criteria:
  - Top frame-time hotspots identified and reduced.
  - Asset/system budgets documented and enforced.
- Manual checklist:
  - [ ] FPS target met (>= 30 FPS sustained in representative scenes).
  - [ ] Memory usage ceiling respected (<= 112 MB resident under stress).
  - [ ] No crashes during a 15+ minute session.

## Unresolved platform gaps (parallel work tracker)

### Rendering / GPU
- [ ] GX pipeline parity audit vs current renderer backend (state setup, blend/depth behavior).
- [ ] Texture format/transcode plan for Wii-friendly formats and memory budget.
- [ ] Shader/fixed-function compatibility strategy for effects currently assuming modern GL paths.

### Input
- [ ] Final mapping for Wiimote, Nunchuk, and Classic Controller profiles.
- [ ] Rumble, pointer, and deadzone calibration policy.
- [ ] Input abstraction updates for hot-swap and disconnect/reconnect handling.

### Audio
- [ ] Backend integration choice (libogc/SDL audio path) and latency validation.
- [ ] Streamed music vs SFX mixing budget on Wii CPU.
- [ ] Missing codec/decoder portability issues inventory.

### Storage / Save Data
- [ ] Save path and permission model for SD/NAND targets.
- [ ] Corruption-safe save writes (temp + atomic rename) on target filesystem.
- [ ] Save migration/versioning checks for legacy worlds.

### Networking
- [ ] Socket backend validation and platform-specific compatibility fixes.
- [ ] Bandwidth/packet budget verification under Wii CPU constraints.
- [ ] Online feature scope decision for first playable milestone.

### Build / Toolchain
- [ ] devkitPPC + libogc reproducible build setup documentation.
- [ ] Third-party library compatibility matrix for Wii target.
- [ ] CI path for compiling Wii artifacts (or documented local-only fallback).

### Runtime Stability / QA
- [ ] Crash logging/minidump equivalent strategy on retail-like hardware.
- [ ] Long-session soak test matrix (menus, gameplay, save/load, reconnect).
- [ ] Regression checklist ownership and triage cadence.
