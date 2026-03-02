# Platform Scope (Current Project Direction)

## Primary target
- **Nintendo Wii is the only intended playable/shipping target.**

## Secondary targets
- **Linux and Windows are debug/development targets only.**
- They are kept for faster iteration, logging, and tooling.
- Behavior should stay as close to Wii gameplay logic as practical.

## Legacy platform code policy
- Android, iOS, and Raspberry Pi files currently remain in-tree for history/reference.
- They are considered **legacy** and should not be expanded with new features.
- New platform work should focus on:
  1. Wii runtime/backend and controller support.
  2. Linux/Windows debug harness parity and diagnostics.

## Cleanup approach
- Prefer **non-destructive deprecation first** (mark legacy, stop wiring into active targets).
- If/when deletion is desired, remove legacy trees in isolated commits so history remains easy to audit.
