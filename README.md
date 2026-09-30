# SS Meridian

SS Meridian is a single-player, story-driven 2D action game developed in C++ with OpenGL/GLUT and an iGraphics-style menu.

Term Project, Software Development I [CSE 1200]

Department of Computer Science and Engineering
Ahsanullah University of Science and Technology (AUST), Bangladesh

## Story

Issac wakes up on the rusted, abandoned ship SS Meridian. In a dark arena, a mysterious Guide tells him the only way out is to collect three stones: **Space**, **Mind** and **Power**. Each stone is earned by completing one mission, and every mission plays differently.

## Missions

| Mission | Gameplay |
| --- | --- |
| **Mission 1: The Space Stone** | Ship exploration, zombie shooting, stealth against a monster (Patrol / Search / Chase / Catch AI), and a sword fight with ghosts on a train |
| **Mission 2: The Three-Headed Dragon** | Archery boss fight with dodging, reloading and a 3-minute time limit |
| **Mission 3: The Arena** | Fight three different monsters one after another, each with its own attack patterns |

After the Power Stone, the game ends with a short cinematic sequence, *The Journey of Issac*.

## Features

- Animated main menu with mission select, controls, settings and credits
- Three missions with three different gameplay styles
- Enemy AI, boss fights, collision and hit detection
- Health, timers and a retry option in every mission
- Stone progression carried across missions and into the ending
- Dialogue and story scenes
- Resizable window and full screen (F11) with a fixed aspect ratio

## Screenshots

<img width="960" height="540" alt="image" src="https://github.com/user-attachments/assets/d9154ffe-57bb-45ed-9a26-ee31f930005a" />


<img width="960" height="540" alt="image" src="https://github.com/user-attachments/assets/fa0e3e69-0ab1-4a23-bee3-41a0bcbe49fb" />


<img width="960" height="540" alt="image" src="https://github.com/user-attachments/assets/8a993829-c321-41cf-9745-403dfca1afb1" />


<img width="960" height="540" alt="image" src="https://github.com/user-attachments/assets/0d8d667d-903f-4b26-986e-687995d41be3" />


<img width="960" height="540" alt="image" src="https://github.com/user-attachments/assets/4d09918c-609a-43a2-a0f3-fbd57f1607cf" />


<img width="960" height="540" alt="image" src="https://github.com/user-attachments/assets/174beba5-b48a-4677-9eb0-418f037f3240" />


<img width="960" height="540" alt="image" src="https://github.com/user-attachments/assets/de044f26-2dd6-483e-8d11-4f7cd304af52" />


<img width="960" height="540" alt="image" src="https://github.com/user-attachments/assets/c99cc4fc-cde8-4279-87a3-0764d66fbeae" />


<img width="960" height="540" alt="image" src="https://github.com/user-attachments/assets/fe4d6b65-a16d-4c9f-9d57-d5441a1fa3bc" />


<img width="960" height="540" alt="image" src="https://github.com/user-attachments/assets/6bdbd8b8-f9c4-44a0-bdf2-2e8fb1c3d13a" />


<img width="960" height="540" alt="image" src="https://github.com/user-attachments/assets/573bcc64-416e-49b5-aaf0-e6cbacf8bcb2" />


## Installation

1. Download Visual Studio 2013 (Update 5). Link below:
   https://www.junian.dev/downloads/visual-studio/#download-visual-studio-2013
2. Download this repository as a ZIP file and unzip it.
3. Open the `SS_Meridian_Final 2` folder and open `SS_Meridian.sln` with Visual Studio 2013.
4. Select **Release** and **Win32** in the toolbar.
5. Press **Ctrl + Shift + B** to build, then **Ctrl + F5** to run.
6. Wait a few seconds. The game will start at the main menu.

To run the game without Visual Studio, use `bin\Release\SS_Meridian.exe`. Keep the `assets` folder and `glut32.dll` in the same folder as the EXE.

## Controls

| Area | Controls |
| --- | --- |
| Mission 1 | **W A S D** move, **ENTER** interact, **SPACE** attack, **E** hide, mouse click |
| Mission 2 | **A / D** move and dodge, **SPACE** fire, **R** reload, mouse click |
| Mission 3 | **A / D** move, **W** jump, **J** kick, **K** Space weapon, **L** Mind fire, **SHIFT** guard, **ENTER** confirm |
| Ending | Hold **W** to walk, **ENTER** to continue |
| Anywhere | **F11** full screen, **ESC** back to the menu |

## Project Structure

| Path | Description |
| --- | --- |
| `src/iMain.cpp` | Entry point, window setup and rendering |
| `src/GameHost.h` | Input routing, timing and mission transitions |
| `src/Menu.h` | Main menu, mission select, controls, settings and credits |
| `src/Integration.h` | Progress tracking (which stones Issac owns) |
| `src/m1`, `src/m2`, `src/m3` | Mission 1, Mission 2 and Mission 3 |
| `assets/` | Artwork and audio (menu, audio, m1, m2, m3) |
| `tests/` | Test programs (not part of the game build) |

## Tech Stack

- Language: C++
- Tools: OpenGL / GLUT, iGraphics-style menu, stb_image
- IDE: Visual Studio 2013
- Platform: Windows
- Version Control: Git and GitHub

## Authors

This game is developed by

- Shariyar Islam
- Shahin Ahmed
- Fahim Hossain Jim

## Supervisors

- Saha Reno, Assistant Professor, Department of CSE, AUST
- Md. Zahid Hossain, Lecturer, Department of CSE, AUST

## License

This project was developed for academic purposes as part of CSE 1200 at AUST.
