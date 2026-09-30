# SS Meridian integration report

Current M2 → M3 handoff: see `HANDOFF_FIX_REPORT.md`. The existing unlock screen now preloads invisibly and accepts one fresh ENTER/click, with no second entry prompt.

## Delivered architecture

`SS_Meridian.sln` contains one `SS_Meridian.vcxproj` application target, with Debug/Release **Win32**, **v120**, **MultiByte**, and consistent static CRT settings (`/MTd` Debug, `/MT` Release). Thirteen production translation units link into one game. `src/Main.cpp` owns the sole entry point, GLUT window/context, timer and event loop. No external mission launches or nested platform loops exist.

| Authority | Integrated module | Integration changes |
| --- | --- | --- |
| Menu ZIP, `iMain.cpp` | `src/Menu.inl` | Retained original artwork, draw helpers, hover behavior, settings/credits and MCI sounds; replaced iGraphics lifecycle with host callbacks and launch requests. |
| Mission 1 ZIP | `src/m1/` | Preserved gameplay, renderer and 1672×941 logical coordinates; namespaced asset paths and shared STB. Host starts at ARRIVAL after the old standalone START screen. |
| Mission 2 ZIP | `src/m2/`, `src/Mission2Host.inl` | Preserved Game/Presentation and current balance, including 1000 HP. Removed standalone Win32 lifecycle; retained original draw-list renderer and Windows GDI font atlas, using shared STB PNG decoding. |
| Mission 3 ZIP | `src/m3/` | Preserved gameplay, balance, renderer, incremental loading and complete final ending; namespaced both original asset roots and shared STB. |

Mission 1 `Game.cpp`, `Scenes.cpp`, `GameplayUpgrade.cpp`; Mission 2 `Game.h`, `Presentation.h`; and Mission 3 `Mission3.cpp`, `Mission3Config.cpp` are byte-identical to their supplied originals. `verification/MISSION_SOURCE_DIFF.patch` records changes inside mission source directories.

## Flow and resources

- START GAME starts M1. The host recognizes a click inside the existing CLAIMED-screen `MISSION 2 - ENTER` rectangle only after the Space Stone claim. It releases M1 textures and initializes M2 at `M2_START`.
- M2's original reward action sets `claimed`; its original unlock ENTER action reaches `M2_HANDOFF_END`. The host keeps the original unlock screen visible while preloading M3, latches one fresh ENTER/click, and enters the existing M3 Intro once ready. Neither the standalone handoff-end message nor a second ENTER prompt is shown.
- Session progress propagates ownership. Direct selection starts each mission fresh with only the appropriate prerequisite stones; Power remains unowned until earned and claimed.
- Exactly three mission cards are rendered and clickable. The supplied full panel bakes in five empty cards, so that layer is omitted; the original individual card textures are used with the menu's existing translucent-fill helper. No artwork bytes were altered. Controls text now describes the supplied missions.
- M3 retains Power claim → arena dialogue → Space/Mind/Power submissions → glow/fade/count → ship walk → Homecoming door → beach walk → reflections → sunrise → terminal `FinalJourney`. No fourth mission was added.
- Input routes only to the active module, with correct per-module coordinate conversion and edge/held-key reset at transitions. ESC returns to the menu; F11 uses the same GLUT window/context.
- Outgoing GPU textures are released before loading the next module. M1 retains its texture cache; M3 retains one upload per presented loading frame. One STB implementation supplies all decoders. Menu audio aliases close when leaving the menu.

## Assets and packaging

579 original media files (576 PNGs and three WAVs) are preserved byte-for-byte under `assets/menu`, `assets/audio`, `assets/m1`, `assets/m2`, and `assets/m3`. M3's separate `mission3_assets` tree is included under `assets/m3/mission3_assets`. `ASSET_PROVENANCE.json` records per-file SHA-256 hashes; `SOURCE_ARCHIVES.json` identifies the untouched source ZIPs.

The original M1 source references an optional countdown WAV that is absent from its ZIP. That optional hook remains; no substitute sound was generated. All supplied audio is retained.

The project copies assets and the supplied x86 GLUT DLL beside the executable. No old build intermediates or QA executables are required or included.

## Verification actually performed

- Complete application compiled and linked with clang++ in C++11 mode against macOS OpenGL/GLUT; no duplicate entry points or linker symbols.
- Structural audit passed: one solution/target/main/window/loop/STB implementation, all production files included, v120/Win32/MultiByte settings, consistent CRTs, x86 GLUT DLL, literal asset paths and all 579 media hashes.
- M1 original smoke suite passed its input-driven full route, alternate choice, retries, combat/navigation/hiding, reward ownership, and 164/164 decoded PNG/command coverage.
- M2 original logic suite and combat simulation passed, including 30 head hits, victory, reward/handoff, input, hitboxes, viewport and presentation checks. Its asset audit passed 89 PNGs, transparency, ten animation sequences and five button groups.
- M3 original logic suite passed all **131 checks**, including actual combat and the complete ending. Incremental-loader test decoded every catalog texture, verified at most one upload per step and matched releases.
- `HostIntegrationQA.cpp` passed using the **actual integrated callbacks and renderer in one offline OpenGL context**: START GAME; all three direct launches; both original handoff hitboxes; prerequisite progress; claim/reward actions; held-key isolation; loading; and the full ending from Power claim through Journey using host key callbacks. Only test fixtures accelerate mission combat; no shortcuts are compiled into the game.
- Fourteen representative screens were rendered without OpenGL errors. Selected menu, mission entry/handoff and ending images were visually inspected; final three-card presentation was re-rendered after correction. Evidence and logs are in `verification/`.

## Build result and remaining target validation

**Native VS2013/v120 was unavailable on this macOS host. No Windows build or Windows EXE is claimed or supplied.** The portable application link, gameplay tests, asset audits and offline rendering pass. Native compiler/linker behavior, Windows GDI font appearance, audio playback, F11/focus behavior and performance on the user's Windows GPU remain to be checked. VMware was not accessed.

From a VS2013 Update 5 x86 Native Tools Command Prompt, run:

```bat
msbuild SS_Meridian.sln /m /t:Build /p:Configuration=Release /p:Platform=Win32 /p:PlatformToolset=v120
```

Then run `bin\Release\SS_Meridian.exe`. `Build_Release.cmd` wraps this build. All portable verification can be repeated using `python3 tests/RunPortable.py`; macOS `--render` additionally runs the shared-host rendering test.
