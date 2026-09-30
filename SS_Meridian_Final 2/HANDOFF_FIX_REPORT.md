# Mission 2 → Mission 3 handoff fix

Only two production files changed. No runtime image, audio, gameplay, balance, menu, mission-select, loader, project configuration or ending code changed.

## Exact production changes

- `src/Main.cpp`
  - Added `prepareMission3Unlock()`: begin the existing incremental M3 loader behind M2's original unlock screen; discard queued ENTER and block an already-held ENTER until release.
  - Added `enterMission3FromUnlock()`: retain the completed preload, release M2 textures, reset transient input, apply progress and confirm M3's existing entry state once. This starts the existing Intro rather than displaying a second ENTER screen.
  - Updated `pointer()`: retain the original unlock button; latch its click once and keep the unlock artwork visible while loading finishes.
  - Updated `advance()`, only its M2 unlock/handoff branch: accept a fresh ENTER edge, upload at most one M3 texture after each presented frame, and enter M3 automatically when both the accepted action and complete assets are present. Readiness alone never starts M3.
  - Updated `release()`: reset the two handoff flags when leaving/restarting a session.
- `src/m3/Mission3Renderer.cpp`
  - Updated `m3::Renderer::draw()`, only the `Enter` presentation: removed the percentage/readiness message. The existing generic ENTER instruction remains for direct Mission Select; its launch behavior is unchanged.

The existing Mission 3 unlock artwork/button remain on screen while preloading. One fresh ENTER (or one click on that button) is sufficient, before or after the assets finish. No percentage, progress bar, readiness message, replacement splash, or loading screen is added. M3 gameplay and final ending use their unchanged state machine.

## Test-only changes

- `tests/HostIntegrationQA.cpp`: changed its sequential handoff expectation to one click → completed preload → M3 Intro, without a second confirm. Its gameplay/ending checks remain.
- `tests/Mission3HandoffQA.cpp`: added focused tests for stale held ENTER, ready-without-input, early/ready single-press activation, repeat suppression, key release/focus changes, complete assets, and pixel-identical unlock frames throughout preload.

Verification results are recorded in `verification/logs/`. The patch against the previously delivered source is `verification/M2_M3_HANDOFF_FIX.patch`.

Native VS2013 and Windows execution remain unavailable in this macOS environment. Rebuild the existing Release / Win32 solution after replacing the two source files; no project-file edits are needed.

## Verification completed for this fix

- Complete production application compiled and linked in C++11 mode on macOS.
- Focused offline-OpenGL tests passed: every unlock frame was pixel-identical throughout preloading; readiness did not auto-start; held/repeated earlier ENTER was rejected; one fresh ENTER started immediately when ready or automatically after remaining uploads. An early accepted press survived release and focus changes.
- All 131 existing Mission 3 gameplay/ending checks passed.
- Updated full shared-host QA passed the existing unlock-button click → M3 Intro with no second ENTER, then the unchanged complete ending through FinalJourney, plus the original menu/direct-launch and M1 handoff checks.
- Hash comparison confirmed only the two named production files changed. All other source modules, asset-loader code and all 579 original media files are unchanged.

Focused QA can be repeated on macOS from the project directory (OpenGL/GLUT frameworks and clang++):

```sh
clang++ -std=c++11 -Wno-deprecated-declarations -Ivendor tests/Mission3HandoffQA.cpp src/Image.cpp src/m1/*.cpp src/m3/*.cpp -framework OpenGL -framework GLUT -o /tmp/SSMeridianHandoffQA
/tmp/SSMeridianHandoffQA
```

`SS_Meridian_M2_M3_Handoff_Fix.zip` contains just the two replacement production source files and this report. Extract it into the existing project root and rebuild; the full project ZIP is also updated.
