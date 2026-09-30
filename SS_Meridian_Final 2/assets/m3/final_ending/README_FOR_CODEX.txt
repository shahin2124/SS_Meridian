SS MERIDIAN — FINAL ENDING ASSETS

CODEX: USE THIS FOLDER ONLY FOR THE NEW ENDING
===============================================

Recommended project destination:
    assets/final_ending/

START POINT
-----------
The new ending begins immediately after the existing Mission 3 POWER STONE CLAIMED state.

ENDING ORDER
------------
1. arena/
   Return Issac to the Main Arena. Use the locked arena background/foreground, Issac, Guide, eye assets, and final dialogue panel.

2. hand/
   Submission order is EXACTLY:
      Space (BLUE)
      Mind  (YELLOW/GOLD)
      Power (PURPLE)

   Hand states:
      hand_three_empty.png
      hand_three_space_only.png
      hand_three_space_mind.png
      hand_three_complete.png

   Use matching VFX folders under hand/vfx/. After Power submission, play hand/vfx/final_glow/.

3. BLACK COUNTDOWN
   Render 1, 2, 3 in code over black. Do not look for image assets for this.

4. ship/
   Use ship_interior_b.png + ship_foreground_b.png.
   Use issac_walk/ for Issac's ship walk.
   Use homecoming_door/ for the six opening frames.
   Render the word HOMECOMING and the ENTER prompt in code.

5. beach/
   homecoming_beach_background.png = static base
   homecoming_sun.png = move upward slowly in code to create the living sunrise
   reflection/ = looping sunlight reflection
   waves/ = looping shoreline animation
   issac_walk/ = corrected 6-frame beach walk
   issac_watching_sunrise.png = final standing pose

   Let Issac walk near the ocean for about 10 seconds, then replace the walk animation with the standing sunrise pose.

6. FINAL FADE
   Render 1, 2, 3 in code, gradually fade to black.

7. finale/
   Show journey_of_issac_final.png.
   Render THE JOURNEY OF ISSAC in code if a title is required.

ABSOLUTE RULES
--------------
- Mission 3 is the final mission. No Mission 4 gameplay.
- Exactly three stones: Space=BLUE, Mind=YELLOW/GOLD, Power=PURPLE.
- No red stone, green stone, Reality Stone, Time Stone, fourth socket, or fifth socket.
- Do not use similarly named assets elsewhere in the project.
- Do not modify these PNG files.
- Old incorrect files HAND AFTER SPACE STONE.png and HAND AFTER SPACE + MIND.png are intentionally NOT included.
- macOS __MACOSX and .DS_Store files are intentionally NOT included.

HAND RENDERING NOTE
-------------------
Some locked Hand/VFX source canvases differ by one pixel in height. Do not regenerate them. Render every Hand state and Hand VFX into the SAME fixed destination rectangle in-game so the Hand never moves between states.

For exact file dimensions and hashes, see asset_manifest.json.
