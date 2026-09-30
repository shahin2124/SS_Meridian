# SS MERIDIAN — MISSION 2 — CODEX ASSET MAP

## READ THIS FIRST

This ZIP is already arranged by **game state / time of use**.  
Do **not** roam the package looking for alternatives. Use the canonical paths below.

There are no source sheets, duplicate sprite sets, `__MACOSX`, or `.DS_Store` files in this package.

## Technical target

- Game display: **1280×720**
- Visual Studio 2013 Update 5
- Win32 / x86
- legacy OpenGL / existing iGraphics-style structure
- PNG assets are pre-rendered; transparent sprites/VFX use RGBA
- Dynamic text/HP/timer/ammo should be rendered by code

## Authoritative controls for Mission 2

- **A** = move Issac left
- **D** = move Issac right
- **SPACE** = fire arrow
- **R** = reload after 3 arrows
- No free mouse aiming
- No separate dodge button
- A/D movement is also the dodge system
- Issac keeps the bow aimed toward the dragon while strafing

## Authoritative combat rules

- Issac HP: **175**
- LEFT dragon head: **50 HP**
- CENTER dragon head: **50 HP**
- RIGHT dragon head: **50 HP**
- Arrow damage default: **10**
- Only valid HEAD hits reduce boss victory HP
- Battle timer: **03:00**
- Dragon is one grounded static approved boss image; attacks are built with separate VFX
- A head at 0 HP stops initiating new attacks
- All three heads at 0 -> dragon death sequence -> Mind Stone reward
- Mind Stone claim -> Mission 3 unlock screen directly
- Do NOT return to Main Arena after Mission 2

---

## Folder map

### `01_ENTRY`
Use only for the Mission 2 entry state.

- `background/mission2_enter_screen.png`
- `ui/mission2_enter_button_normal.png`
- `ui/mission2_enter_button_hover.png`
- `ui/mission2_enter_button_pressed.png`

### `02_DRAGON_INTRO`
Use immediately after entry to establish the dragon challenge.

- `background/mission2_dragon_intro_background.png`

### `03_FIGHT_GUIDE`
Use before combat. The 03:00 timer must NOT run here.

- `background/mission2_fight_guide_background.png`
- `ui/start_fight_button_normal.png`
- `ui/start_fight_button_hover.png`
- `ui/start_fight_button_pressed.png`

Render guide text by code.

### `04_BATTLE`
This folder contains everything used during active combat.

#### Draw order

1. `environment/mission2_dragon_battle_background.png`
2. `boss/mission2_dragon_boss_idle.png`
3. active dragon VFX/projectiles
4. Issac sprite + Issac arrow/VFX
5. `environment/mission2_dragon_battle_foreground.png`
6. `ui/mission2_combat_hud.png`
7. aim reticle when appropriate
8. dynamic values/text

#### Player

Default ready:
`player/issac_bow_ready.png`

Hold A:
`player/strafe_left/issac_bow_strafe_left_01.png` → `_06.png`

Hold D:
`player/strafe_right/issac_bow_strafe_right_01.png` → `_06.png`

SPACE:
`player/projectiles/issac_arrow_projectile.png`

At arrow release:
`player/vfx/bow_release/bow_release_vfx_01.png` → `_06.png`

Valid head collision only:
`player/vfx/arrow_head_hit/arrow_head_hit_vfx_01.png` → `_06.png`

#### Dragon — IMPORTANT

The dragon itself is:
`boss/mission2_dragon_boss_idle.png`

Do NOT search for left/center/right full-body attack dragon sprites. They do not exist and are not required.

For whichever living head is selected to attack:

1. Place `boss/vfx/mouth_charge/dragon_mouth_charge_vfx_01..06` on that head's mouth.
2. Show `boss/vfx/ground_warning/dragon_ground_warning_vfx_01..06` at predicted impact location.
3. Launch `boss/projectiles/dragon_fireball_projectile.png` from that head mouth.
4. At impact play `boss/vfx/fire_impact/dragon_fire_impact_vfx_01..06`.
5. Damage Issac only if his hitbox is still inside the valid impact region.

When one head reaches 0 HP:
loop `boss/vfx/defeated_head_loop/dragon_head_defeated_loop_01..06` over that head and prevent that head from starting new attacks.

When all three heads reach 0:
play `boss/vfx/death/dragon_death_vfx_01..06`, stop timer/attacks/damage, then transition to reward.

#### Target reticle

`ui/mission2_aim_reticle.png`

Only show it over a **living head** when Issac's fixed projectile trajectory currently aligns with that head.

#### Loss / retry

There is deliberately no separate loss background in this package.

On loss:
- freeze battle
- reuse the current battle scene
- render a dark translucent overlay in code
- render `MISSION FAILED`
- use `ui/retry/retry_button_normal|hover|pressed.png`

### `05_MIND_STONE_REWARD`

After dragon death:

- background: `background/mind_stone_reward_bg.png`
- stone: `stone/mind_stone.png`
- looping aura: `stone/aura/mind_stone_aura_01.png` → `_06.png`
- CLAIM states: `ui/claim_button_normal|hover|pressed.png`

The Mind Stone is yellow/gold.

Only mark Mission 2 completed / Mission 3 unlocked when the player performs the claim action.

### `06_MISSION3_UNLOCK`

Immediately after successful Mind Stone claim:

- `background/mission3_unlock_background.png`
- `ui/enter_button_normal.png`
- `ui/enter_button_hover.png`
- `ui/enter_button_pressed.png`

Do NOT return to Main Arena.

ENTER -> existing Mission 3 startup flow.

---

## Animation convention

Every `01..06` folder is ordered:

`01 -> 02 -> 03 -> 04 -> 05 -> 06`

For looping effects, return to `01`.

Left/right strafe frames already use identical canvases inside each sequence.

## Asset safety changes made during packaging

No artwork was regenerated.

Only these safe packaging operations were performed:

- removed Mac metadata / junk files;
- moved left-strafe frames out of an incorrect source folder into the player folder;
- renamed `dragon_master_reference.png` to `mission2_dragon_boss_idle.png`;
- renamed `arrow_projectile.png` to `issac_arrow_projectile.png`;
- normalized the generic blank action-button states onto one identical transparent canvas without resizing artwork;
- duplicated that normalized blank button under explicit START FIGHT and RETRY names so Codex does not need to infer reuse;
- added transparent safety padding to edge-touching single-sprite assets without resizing/repainting their pixels.

See `VALIDATION_REPORT.txt` and `ASSET_INVENTORY.csv` for exact verification.
