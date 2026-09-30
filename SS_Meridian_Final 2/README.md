# SS Meridian

One integrated game: approved Menu → Mission 1 → Mission 2 → Mission 3 → the original final Journey ending. Mission Select provides three independent entry routes. All runtime artwork and available audio retain their original bytes.

## Build and play

Use **Visual Studio 2013 Update 5**, **v120**, **Win32**. Open `SS_Meridian.sln`, select Release / Win32, then Build Solution. Or run `Build_Release.cmd` from the VS2013 x86 Native Tools Command Prompt:

```bat
msbuild SS_Meridian.sln /m /t:Build /p:Configuration=Release /p:Platform=Win32 /p:PlatformToolset=v120
```

Run `bin\Release\SS_Meridian.exe`. The build copies the supplied x86 `glut32.dll` and complete `assets` folder beside it. Keep that folder and DLL together with the EXE when distributing the built game. No absolute developer-machine paths or external mission processes are used. Debug / Win32 is also configured.

A native Windows EXE is not included: this delivery was built and tested on macOS, without VS2013. See `HEADER_REFACTOR_REPORT.md` for the current verification scope; `INTEGRATION_REPORT.md` describes the supplied baseline.

## Play

- START GAME begins Mission 1 at the ship arrival, immediately after its former standalone START screen.
- Claim the Space Stone, then click the existing **MISSION 2 - ENTER** button.
- Finish Mission 2, claim the Mind Stone, then click **ENTER** on its existing Mission 3 unlock screen.
- Finish Mission 3 and claim the Power Stone. Continue through the existing arena submissions, ship, Homecoming door, beach, reflection, sunrise and final Journey artwork.
- MISSION SELECT starts any of the three missions fresh. Prerequisite stones are initialized; the selected mission's reward must still be earned.
- ESC returns from a mission or submenu to the main menu. ESC on the main menu or EXIT closes the game. F11 toggles fullscreen. Resizing uses a centered letterboxed viewport.

| Area | Controls |
| --- | --- |
| Mission 1 | WASD movement; ENTER interactions; SPACE attack; E hide; mouse buttons |
| Mission 2 | A/D movement and dodge; SPACE fire; R reload; mouse buttons |
| Mission 3 | A/D movement; W jump; J kick; K weapon; L fire; SHIFT guard; ENTER confirm; mouse buttons |
| Ending | Hold W to walk; release and press again when entering a new walking scene; ENTER dialogue/door/reflection |

SETTINGS retains the audio toggle. The original archives contain three menu WAV files. Mission 1's optional countdown WAV was referenced by its source but was absent from the supplied archive; its optional playback hook remains, without a replacement sound.

## Source and verification

`src/iMain.cpp` owns the application callbacks and rendering dispatcher; `src/GameHost.h` preserves shared state, input routing, timing updates and mission transitions. `src/m1`, `src/m2`, and `src/m3` preserve the mission modules; the guarded `Menu.h` and `Mission2Host.h` adapters preserve the menu and Mission 2 drawing behavior. Runtime assets are namespaced in `assets/menu`, `assets/audio`, `assets/m1`, `assets/m2`, and `assets/m3`.

`tests/ValidateIntegration.py` audits the project and media hashes with Python 3. `tests/RunPortable.py` builds and runs the portable mission regressions using clang++/g++. On macOS, `--render` additionally exercises the real shared host and renders representative screens in one offline OpenGL context. Tests are excluded from both production configurations; no gameplay skips are present in Release or Debug.
