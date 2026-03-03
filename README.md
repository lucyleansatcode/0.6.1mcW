## MCPE 0.6.1 RVL port.
Based on source code previously leaked on 4chan, which includes the developer build of Minecraft PE 0.6.1.

## Current platform direction
- Playable target: **Wii only**.
- Debug/development targets: **Linux and Windows**.
- Android/iOS/Raspberry Pi code is treated as **legacy reference** unless explicitly reactivated.

## Building the Wii target on Windows (devkitPro)

The Wii build is driven by `handheld/project/wii/Makefile`, not Visual Studio CMake/MSBuild.

1. Install devkitPro/devkitPPC on Windows:
   - Preferred: devkitPro installer for Windows.
   - Alternate: MSYS2 + devkitPro pacman packages.
2. Open the devkitPro-enabled shell (MSYS2/devkitPro shell) and ensure these environment variables are set:

   ```sh
   echo "DEVKITPRO=$DEVKITPRO"
   echo "DEVKITPPC=$DEVKITPPC"
   test -f "$DEVKITPPC/wii_rules" && echo "wii_rules found"
   ```

   `handheld/project/wii/Makefile` expects `DEVKITPPC` and includes `$(DEVKITPPC)/wii_rules`.
   For Windows Git Bash (MINGW64) with a default devkitPro install, use:

   ```sh
   export DEVKITPRO=/c/devkitPro
   export DEVKITPPC=$DEVKITPRO/devkitPPC
   ```
3. Build from that shell with:

   ```sh
   make -C handheld/project/wii
   ```

If the toolchain path is missing, the Makefile prints guidance and exits without building.
