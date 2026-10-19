# Day of Defeat client DLL

This document describes the structure of the Day of Defeat 1.3 client library (`client.dll` / `client.so`), how it
talks to the engine, and what differs from [hlsdk-portable](https://github.com/FWGS/hlsdk-portable), the SDK this
code base is built on. It is written for people who want to read, build or modify the client.

The client targets **Xash3D FWGS**. GoldSrc support is kept behind `GOLDSOURCE_SUPPORT` (x86 Windows/Linux/macOS only).

* [Layout](#layout)
* [Build options](#build-options)
* [Fixes of the bugs of the original game](#fixes-of-the-bugs-of-the-original-game)
* [How the engine talks to the client](#how-the-engine-talks-to-the-client)
* [User messages and console commands](#user-messages-and-console-commands)
* [Console variables](#console-variables)
* [Console commands of the client](#console-commands-of-the-client)
* [Mechanics that exist only in Day of Defeat](#mechanics-that-exist-only-in-day-of-defeat)
* [Weapon prediction](#weapon-prediction)
* [VGUI1 instead of VGUI2](#vgui1-instead-of-vgui2)
* [Differences from hlsdk-portable](#differences-from-hlsdk-portable)

## Layout

| Path | What is there |
|------|---------------|
| `cl_dll/cdll_int.cpp` | The exported functions the engine looks up by name (`Initialize`, `HUD_*`). Owns `gEngfuncs`, `gHUD` and `g_pParticleMan`. |
| `cl_dll/hud.cpp`, `hud.h` | `CHud`, the owner of all HUD elements, the cvars and the user message hooks. |
| `cl_dll/dod_*.cpp`, `mortarhud.cpp`, `scope.cpp`, `weather.cpp`, `ParticleShooter.cpp`, `CClientEnvModel.cpp` | Day of Defeat HUD elements and systems, see below. |
| `cl_dll/ev_dod.cpp`, `ev_common.cpp` | Client side of the events: weapon fire effects, explosions, surface hits, blood, corpses, voices. |
| `cl_dll/dod/` | The glue that lets the server weapon code run inside the client (prediction). |
| `cl_dll/vgui/` | The viewport and all menus, written on freevgui (VGUI1). |
| `cl_dll/StudioModelRenderer.cpp`, `GameStudioModelRenderer.cpp` | Model rendering: bone cache, player animation, bipod and sandbag poses, env_model drawing. |
| `cl_dll/view.cpp`, `input.cpp`, `input_xash3d.cpp`, `in_camera.cpp` | View calculation, input and the third person camera. |
| `dlls/` | Headers shared with the server (`cbase.h`, `player.h`, `weapons.h`, ...), `dod_shared.cpp` (data tables) and `wpn_shared/` (one class per weapon). There is no server code. |
| `pm_shared/` | Player movement, in C, shared with the server. |
| `particleman/`, `public/` | The particle manager of the original game, compiled into the client. |
| `game_shared/` | Voice and a few helpers shared with the server. |

## Build options

`mod_options.txt` in the root of the repository holds the options of the build, one per line: `NAME=VALUE # description`.
Both waf (`wscript`) and CMake read it. `ON` defines the macro `NAME` for the whole client, `OFF` defines nothing, any other
value becomes a variable of the build.

| Option | Value | Meaning |
|--------|-------|---------|
| `GAMEDIR` | `dod` | Directory of the game; the install target puts the client into `<GAMEDIR>/<CLIENT_INSTALL_DIR>`. |
| `CLIENT_INSTALL_DIR` | `cl_dlls` | Where the client library goes (must match `liblist.gam` / `gameinfo.txt`). |
| `FOG_MINIMAP_FIX` | `ON` | A fix of a bug of the original game, see [below](#fixes-of-the-bugs-of-the-original-game). |
| `SCOPE_WIDESCREEN_FIX` | `ON` | A fix of a bug of the original game, see [below](#fixes-of-the-bugs-of-the-original-game). |
| `MINIMAP_SPECTATOR_FIX` | `ON` | A fix of a bug of the original game, see [below](#fixes-of-the-bugs-of-the-original-game). |

The options of Half-Life (crowbar, gauss, satchel fixes and the like) are removed. The fixes for the bugs of the original
game are added here, each under its own macro, so they can be turned off.

## Fixes of the bugs of the original game

The original client has bugs that this code base reproduces on purpose, because the reverse follows the binary. A fix of such a
bug is **optional**: it sits behind a macro that is switched in `mod_options.txt` (see [Build options](#build-options)), as
the fixes of the Half-Life weapons are in hlsdk-portable. With the option `OFF` the client behaves exactly like the original.
All the fixes below are `ON` by default.

| Option | The bug of the original | The fix |
|--------|-------------------------|---------|
| `FOG_MINIMAP_FIX` | The fog of `cl_fog` is turned off by the map: `RenderDoDFog` forces the level 0 while the small or the large map is shown (`_cl_minimap` 1 or 2), so the fog works only with the map hidden completely (`_cl_minimap 0`). The map is the second view of the frame and the fog that stays on would gray it, the original avoids that by turning the fog off for the whole frame. | The fog follows `cl_fog` in every mode of the map. `SuspendDoDFog` (`dod_fog.cpp`) turns it off only while the map is drawn in `HUD_DrawTransparentTriangles`, and `RenderDoDFog` turns it on again right after, so the next frame is fogged. |
| `SCOPE_WIDESCREEN_FIX` | On a widescreen resolution the frame of the sniper scope (and of the binoculars) does not cover the screen: at 1920x1200 the world is seen in strips of 80 pixels at the left and the right and of 50 pixels at the top and the bottom. The engine renders a widescreen with the vertical FOV of 4:3 and widens the horizontal one, the original sizes the frame with the FOV of the screen itself, so it is about 8% too small. | `AdjustScopeFov` in `scope.cpp` repeats the calculation of the engine (`V_AdjustFov` of `cl_view.c`), so the frame is sized for the FOV that is really rendered. It does nothing at 4:3 and 5:4, and in an engine that has no `r_adjust_fov` cvar. |
| `MINIMAP_SPECTATOR_FIX` | While a player spectates, the map shows the icon of his own position of the Allies whatever the team of the watched player is, and none of the team mates, because a spectator has no team. | `GetMapTeam` in `dod_map.cpp` takes the team of the watched player (`g_iUser2`), the icon of the camera is the Axis, British or Allies one for that team, and the players of that team are drawn. Without a watched player (free roaming) the map behaves as before. |

### Adding a fix

1. Add a line to `mod_options.txt`: `NAME=ON # what the fix does`.
2. Wrap the changed code in `#if NAME` ... `#else` (the original code) ... `#endif`. Use `#if`, not `#ifdef`: a switched off option
   defines nothing.
3. Run `waf configure` again (CMake reads the file at configure time too). The build log lists the option and its state.

### Differences from the Steam version that are not in the client

* **Tilting the view when strafing.** The client code (`V_CalcViewRoll`, `PM_CalcRoll`) is the same as in the original, the tilt
  comes from the server cvar `sv_rollangle`. The default of GoldSrc is `2.0` (`sv_rollspeed 200`), the default of Xash3D is `0`,
  so without `sv_rollangle 2` in the config of the server there is no tilt. The setting is saved to `config.cfg` (`FCVAR_ARCHIVE`).

## How the engine talks to the client

### Exported functions

The engine finds these by name in `cdll_int.cpp`, `entity.cpp`, `view.cpp`, `input.cpp`, `tri.cpp`, `hud_spectator.cpp`,
`demo.cpp` and `StudioModelRenderer`: `Initialize`, `HUD_Init`, `HUD_VidInit`, `HUD_Redraw`, `HUD_UpdateClientData`,
`HUD_Reset`, `HUD_PlayerMove`, `HUD_PlayerMoveInit`, `HUD_PlayerMoveTexture`, `HUD_PostRunCmd`, `HUD_Frame`, `HUD_Key_Event`,
`HUD_AddEntity`, `HUD_CreateEntities`, `HUD_StudioEvent`, `HUD_TempEntUpdate`, `HUD_DrawNormalTriangles`,
`HUD_DrawTransparentTriangles`, `HUD_GetStudioModelInterface`, `HUD_MobilityInterface`, `V_CalcRefdef`, `CL_CreateMove`, `CL_IsThirdPerson`,
`IN_ActivateMouse`, `IN_Accumulate`, `IN_MouseEvent`, `Demo_ReadBuffer`, `HUD_ChatInputPosition`, `HUD_Shutdown` and others.
Xash3D never calls the legacy GoldSrc `F(void*)` table, so the original `cldll_func_t` hooks are not used.

### Drawing

The calls go through `gEngfuncs`; `cl_util.h` wraps them in macros of the same name as in the original SDK.

* `HSPRITE SPR_Load( char *picname )`: load a sprite, returns a handle.
* `int SPR_Frames( HSPRITE )`, `SPR_Height( HSPRITE, int frame )`, `SPR_Width( HSPRITE, int frame )`: sprite sizes (0 for a bad handle or frame).
* `SPR_Set( HSPRITE, int r, int g, int b )`: select the sprite and its color; required before drawing.
* `SPR_Draw( frame, x, y )`, `SPR_DrawHoles( frame, x, y )` (color index 255 is transparent), `SPR_DrawAdditive( frame, x, y )`.
* `SPR_EnableScissor( x, y, width, height )` / `SPR_DisableScissor()`: clipping rectangle.
* `ScreenWidth` / `ScreenHeight`: macros for the size of the screen in pixels (`gHUD.m_scrinfo`). Use them instead of reaching into `gHUD`.
* `XRES( x )` / `YRES( y )`: convert coordinates given in 640x480 space.

### Sound and communication

* `PlaySound( char *name, int volume )` plays a sound by name; `PlaySound( int index, int volume )` plays a precached one.
* `SendClientCmd( char *cmd )` sends a command to the server as if it was typed in the console.
* `GetPlayerName( int entity_number )` returns the name of a client or NULL.

## User messages and console commands

The server sends user messages, the client binds them to `CHud` members. Four macros from `cl_util.h` do the binding:

```cpp
// in CHud::Init(): hook the message "Health" into the engine
HOOK_MESSAGE( Health );

// once per message: a free function that forwards the message to the HUD element m_Icons
DECLARE_MESSAGE( m_Icons, Health );

// the same for console commands
HOOK_COMMAND( "+showscores", ShowScores );
DECLARE_COMMAND( m_Scoreboard, ShowScores );
```

`DECLARE_MESSAGE( y, x )` expects a member `y` of `CHud` with a method `MsgFunc_x`, and `DECLARE_COMMAND( y, x )` a method
`UserCmd_x`. The methods are declared in the headers with the macros of `hud.h`, which keep the signatures in one place:

```cpp
class CHudDodIcons : public CHudBase
{
public:
	CHudMsgFunc( Health );		// int MsgFunc_Health( const char *pszName, int iSize, void *pbuf )
	CHudMsgFuncV( InitHUD );	// void MsgFunc_InitHUD( const char *pszName, int iSize, void *pbuf )
	CHudUserCmd( Slot1 );		// void UserCmd_Slot1()
};
```

Read the message with `BEGIN_READ( pbuf, iSize )` and `READ_BYTE()`, `READ_SHORT()`, `READ_LONG()`, `READ_COORD()`,
`READ_STRING()`. Return 1 when the message was handled.

### Messages the client hooks

`AllowSpec`, `AmmoPickup`, `AmmoShort`, `AmmoX`, `BloodPuff`, `CameraView`, `CancelProg`, `CapMsg`, `ClCorpse`, `ClanTimer`,
`ClientAreas`, `CurMarker`, `CurWeapon`, `Damage`, `DeathMsg`, `Frags`, `GameRules`, `GameTitle`, `HLTV`, `HandSignal`, `Health`,
`HideWeapon`, `HudText`, `InitHUD`, `InitObj`, `ItemPickup`, `Logo`, `MOTD`, `MapMarker`, `ObjScore`, `Object`, `PClass`, `PShoot`,
`PStatus`, `PTeam`, `PlayersIn`, `ProgUpdate`, `RandomPC`, `ReloadDone`, `ReqState`, `ResetHUD`, `ResetSens`, `RoundState`,
`SayText`, `Scope`, `ScoreInfo`, `ScoreInfoLong`, `ScoreShort`, `ServerName`, `SetFOV`, `SetObj`, `ShowMenu`, `Spectator`,
`StartProg`, `StartProgF`, `StatusIcon`, `StatusValue`, `TeamNames`, `TeamScore`, `TextMsg`, `TimeLeft`, `TimerStatus`, `Train`,
`UseSound`, `VGUIMenu`, `VoiceMask`, `WaveStatus`, `WaveTime`, `WeapPickup`, `WeaponList`, `WideScreen`, `YouDied`.

Several of them (`ScoreInfo`, `ScoreInfoLong`, `ScoreShort`, `Frags`, `ObjScore`, `PStatus`, `PTeam`, `PClass`, `TeamScore`,
`WaveTime`, `WaveStatus`, `CurMarker`, `MapMarker`, `WideScreen`, `ForceCam`, `VGUIMenu`) go through `DoDViewport` in
`cl_dll/vgui/`, see [VGUI1 instead of VGUI2](#vgui1-instead-of-vgui2).

## Console variables

All values are the defaults at registration. `A` = saved to `config.cfg` (`FCVAR_ARCHIVE`), `U` = sent to the server as
userinfo (`FCVAR_USERINFO`).

### HUD

| Cvar | Default | Flags | Effect |
|------|---------|-------|--------|
| `cl_hud_health` | 1 | A | Show the health and stamina icons. |
| `cl_hud_ammo` | 1 | A | Show the ammo counters. |
| `cl_hud_objectives` | 1 | A | Show the objective icons (capture points). |
| `cl_hud_objtimer` | 1 | A | Show the timer of an objective. |
| `cl_hud_reinforcements` | 1 | A | Show the reinforcements counter when the lives are infinite. |
| `cl_hud_msgs` | 2 | A | Chat-style server messages: 0 none, 1 only "has joined/left the game", 2 all. |
| `cl_identiconmode` | 2 | A | Team icon over players: 0 off, 1 over the player you look at, 2 over the whole team. |
| `hud_centerid` | 1 | A | Center the name of the target in the status bar. |
| `hud_deathnotice_time` | 6 | A | Seconds a death notice stays on the screen. |
| `hud_saytext_time` | 5 | | Seconds a chat line stays on the screen. |
| `hud_saytext_internal` | 1 | | Internal state of the `hud_saytext` command: chat on/off. |
| `hud_textmode` | 0 | A | 2 draws HUD strings with the console font. |
| `hud_fastswitch` | 0 | A | Select a weapon with one keypress. |
| `hud_draw` | 1 | A | Draw the HUD. |
| `hud_capturemouse` | 1 | A | The HUD may capture the mouse. |
| `hud_takesshots` | 0 | A | Take a screenshot at the end of every round. |
| `cl_autoreload` | 1 | A, U | Reload automatically when the clip is empty. |
| `cl_hudfont` | 1 | A | Registered as in the original; not read by the client. |
| `cl_scoreboard_bg` | 1 | A | Background of the plain text scoreboard. |
| `cl_showpacketloss` | 0 | A | Show the packet loss column in the scoreboard. |
| `dodcredits` | 0 | A | Registered as in the original; the `credits` command shows the credits on the screen. |

### Map and spectator

| Cvar | Default | Flags | Effect |
|------|---------|-------|--------|
| `_cl_minimap` | 2 | A, U | State of the map: 0 hidden, 1 large map in the middle of the screen, 2 small map in the corner. Changed with `+map`. The map is only drawn with a hardware renderer and FOV 90. |
| `_cl_minimapzoom` | 1 | A | Zoom step of the small map (1, 3, 8 times). Changed with `+mapzoom`. |
| `cl_dmsmallmap` | 1 | A | Allow the small map; 0 makes `+map` toggle only the large one. |
| `cl_dmshowmarkers` | 1 | A | Draw map markers (posted by players). |
| `cl_dmshowplayers` | 1 | A | Draw players on the map. |
| `cl_dmshowflags` | 1 | A | Draw the flags on the map. |
| `cl_dmshowobjects` | 1 | A | Draw the objectives on the map. |
| `cl_dmshowgrenades` | 1 | A | Draw grenades, dynamite and the like on the map. |
| `spec_pip` | 1 | | Picture-in-picture of the spectator. `+use` resets it to 0, as in the original. |
| `spec_scoreboard` | 0 | | Show the scoreboard to a spectator. |
| `spec_drawnames_internal`, `spec_drawcone_internal`, `spec_drawstatus_internal`, `spec_autodirector_internal` | 1 | | State of the commands `spec_drawnames`, `spec_drawcone`, `spec_drawstatus`, `spec_autodirector`. |

### Effects and rendering

| Cvar | Default | Flags | Effect |
|------|---------|-------|--------|
| `cl_particlefx` | 2 | A | 0 no particles, 1 particles, 2 also rain, snow and particle shooters of the map. Needs a hardware renderer. |
| `cl_weatherdis` | 1700 | A | Distance in units up to which rain and snow particles are created around you (at least 100). |
| `max_rubble` | 240 | | Maximum number of rubble pieces alive at the same time. |
| `cl_numshotrubble` | 5 | A | Maximum number of rubble pieces a bullet hit throws (a random number from 0 to this). |
| `cl_corpsestay` | 10 | A | Seconds a corpse stays after a death. |
| `cl_dynamiclights` | 1 | A | Dynamic lights of muzzle flashes and explosions. |
| `cl_bulletejects` | 1 | A | Eject shells from the weapons. |
| `cl_drawmodels`, `cl_drawplayermodels` | 1 | A | Draw models / player models. |
| `cl_fog` | 1 | A | 0 off, 1 gray fog from `cl_fog_start` to `cl_fog_end`, 2 fog with the color of `cl_fog_red/green/blue`. |
| `cl_fog_start`, `cl_fog_end` | 10, 3500 | A | Distances of the fog. |
| `cl_fog_red`, `cl_fog_green`, `cl_fog_blue` | 192 | A | Color of the fog for `cl_fog 2`. |
| `cl_fog_density` | 0.0 | A | Registered as in the original; the client does not use it, the density is set only by the engine. |
| `cl_cutscenes` | 1 | A | Registered as in the original; not read by the client. |

The fog is drawn through `TriAPI->Fog`, not with direct OpenGL calls as in the original, so it works with any Xash3D
renderer. It is off under water. The gray sky and the lighter textures are the fog itself: a linear fog from `cl_fog_start`
to `cl_fog_end` (10 and 3500 by default) blends the far surfaces and the sky with the gray. In the original `cl_fog 1` is an
OpenGL fog (gray 0.5, linear) and only it gives a visible fog; here `cl_fog 1` is the same gray fog through `TriAPI->Fog`
and `cl_fog 2` makes a fog of the color `cl_fog_red/green/blue`, which works in Xash3D. The original turns the fog off while
the map is shown, see `FOG_MINIMAP_FIX` in [Fixes of the bugs of the original game](#fixes-of-the-bugs-of-the-original-game).

### Crosshair, view and input

| Cvar | Default | Flags | Effect |
|------|---------|-------|--------|
| `cl_dynamic_xhair` | 1 | A | The crosshair grows with movement and recoil. 0 keeps it fixed. |
| `cl_xhair_style` | 0 | A | 0 draws `sprites/crosshairs.spr`, other values pick a crosshair from `sprites/customXHair.spr`. |
| `cl_vsmoothing` | 0.05 | A | Seconds of view smoothing on trains and lifts. |
| `cl_bob`, `cl_bobcycle`, `cl_bobup` | 0.005, 0.8, 0.5 | A (`cl_bob`) | Weapon and view bob. `cl_bob` is saved to `config.cfg`: a value left by another client (Half-Life uses 0.01) doubles the sway of the weapon, in the original it is 0.005. |
| `cl_chasedist`, `cl_waterdist` | 112, 4 | | Distance of the chase camera, offset of the view under the water line. |
| `zoom_sensitivity_ratio` | 1.2 | A | Mouse sensitivity multiplier while zoomed. |
| `cl_pitchup`, `cl_pitchdown` | 89 | | Limits of the view pitch. |
| `cl_forwardspeed`, `cl_backspeed`, `cl_sidespeed` | 400 | A (first two) | Speeds of the keys. The real speed comes from the stamina (`DoDGetSpeed`). |
| `cl_laddermode` | 2 | A | Xash3D ladder behavior. |
| `sensitivity`, `m_pitch`, `m_yaw`, `m_forward`, `m_side` | 3, 0.022, 0.022, 1, 0.8 | A | Mouse. |

### VGUI

| Cvar | Default | Flags | Effect |
|------|---------|-------|--------|
| `cl_motd_vgui` | 1 | A | 1 shows the MOTD as a VGUI window, 0 as plain text in the HUD. |
| `cl_scoreboard_vgui` | 1 | A | 1 shows the VGUI scoreboard, 0 the text one of the HUD. |
| `_ah` | 1 | A, U | Registered as in the original (userinfo); the client does not read it. |

The engine cvars `vgui_utf8` and `con_fontscale` / `hud_fontscale` affect the menus; see the VGUI section.

## Console commands of the client

| Command | What it does |
|---------|--------------|
| `+map` / `-map` | Cycle the state of the map (`_cl_minimap`). |
| `+mapzoom` / `-mapzoom` | Change the zoom of the small map. |
| `+special` / `-special` | Registered as in the original; the client only keeps the key state. |
| `+score` `+showscores` / `-score` | Show the scoreboard. |
| `+commandmenu` / `-commandmenu` | Open the command menu (`commandmenu.txt`). |
| `prone` | Go prone (sends `sprone` to the server). |
| `centerview` | Reset the view pitch. |
| `credits` | Show the credits on the screen. |
| `slot1` ... `slot10`, `invnext`, `invprev`, `cancelselect` | Weapon selection. |
| `hud_saytext` | Turn the chat on or off. |
| `spec_mode`, `spec_decal`, `spec_menu`, `spec_help`, `spec_autodirector`, `spec_drawnames`, `spec_drawcone`, `spec_drawstatus` | Spectator. |
| `thirdperson`, `firstperson`, `+camin`, `+camout`, `+camdistance`, ... | Third person camera. |
| `voice_showbanned` | Voice status of the players. |

## Mechanics that exist only in Day of Defeat

### Entity lump parser (`dod_camera.cpp`)

The game does not send everything a map needs, so the client reads the **entity lump of the BSP file itself**.
`DoD_LoadClientEnts( map )` is called at the end of `CHud::MsgFunc_InitHUD`. It loads the lump, parses it into
key/value pairs (`DoD_ProcessEnts`) and acts on five entity classes:

| Entity | What the client does |
|--------|----------------------|
| `dod_camera`, `path_corner` | Stored in a list; the cinematic camera moves along them (`DoD_FindTarget`, `DoD_ReadCameraSettings`). |
| `particle_shooter` | Registered in `CParticleShooter` (up to `MAX_SHOOTERS`); the flags are in `game_shared/fx_flags.h`. |
| `info_doddetect` | `detect_wind_velocity_x/y` set the global wind of the particles, `detect_weather_type` selects rain (1) or snow (2), `detect_allies_country` tells whether the Allies are British. |
| `env_model` | Static props without a targetname are drawn only by the client, see [Static props](#static-props-cclientenvmodel). |

### Cinematic camera

The `CameraView` message and the `EV_DoDCamera` event switch the view to a `dod_camera` (the FOV of the camera is applied, 90 when it has none) and play
the screen fades (`DoD_ScreenFades`). The `WideScreen` message turns the widescreen state on and off (the original draws no bars).

### The map (`dod_map.cpp`, `view.cpp`, `tri.cpp`)

The map is **not a sprite**. The engine renders the world a second time from above: `V_CalcNormalRefdef` requests a second
view (`nextView`), sets `g_iOnlyClientDraw` and a viewport from `GetMapBounds()`; `HUD_DrawTransparentTriangles` then draws
the objective icons and the entities (`CHudDoDMap::DrawOverview`) instead of the fog and the particles. The engine must
honor `nextView` for the map to appear. The overview data (zoom, origin, height layers) comes from `overviews/<map>.txt`, the same file the spectator map uses.

### Objectives (`dod_objectives.cpp`)

`CObjectiveIcons` draws the capture points over the world and on the HUD: owner, capture progress (`StartProg`,
`ProgUpdate`, `CancelProg`), number of players inside (`PlayersIn`), timers (`TimerStatus`, `WaveTime`). Icons are placed by
projecting the origin of the point onto the screen (`CalcIconLocations`, `ScreenToWorld`).

### Static props (`CClientEnvModel`)

Maps place props (trees, hay, vehicles, flak guns) with `env_model` entities. In Day of Defeat the **static ones are never
sent by the server**: an `env_model` without a `targetname` that is not "initially off" exists only in the entity lump of the
BSP file, and the client draws it by itself (`dod_merderet` has 46 of them). An `env_model` with a `targetname` is an
ordinary server entity and is not touched. The models have no collision, mappers put clip brushes around them.

1. **Parse.** `Parse_env_model` (called from `DoD_ProcessEnts`, see above) reads `model`, `body`, `origin`, `angles`,
   `spawnflags`, `m_iAction_On` (greater than 0 makes the model animated) and `m_iszSequence_On` (the name of the sequence).
2. **Store.** `CClientEnvModel::AddEnvModel` keeps up to `MAX_ENV_MODELS` (192) models: loads the model, finds the sequence by
   its name (`GetSequence`, sequence 0 when the name is unknown). `SF_ENVMODEL_DROPTOFLOOR` (2) traces down from the origin
   to -8000 with hull 2 and puts the model on the floor, `SF_ENVMODEL_ANIMATION` (0x10) makes the sequence loop. The list is
   cleared when a map is loaded.
3. **Draw.** The renderer is asked to draw entities, not a list, so `CClientEnvModel::Think` keeps one **proxy temp entity**
   (`models/p_stick.mdl`, flags `FTENT_PERSIST | FTENT_CLIENTCUSTOM | FTENT_SCALE`, `entityType = 4`). Its callback
   `EV_EnvModelCallback` moves it to the local player (or to the camera of a spectator) every frame, so it is never culled.
   When `CStudioModelRenderer::StudioDrawModel` gets an entity with `entityType & 4` it draws **all the models of the list
   instead**: the box of the sequence (`bbmin`/`bbmax` moved to the origin) is tested with `BoxInPVS`, a visible model is
   drawn with `StudioDrawCurrentEntity`, and the entity is restored at the end.
4. **Xash3D.** The engine frees every temp entity when its `die` time passes (0.75 seconds after the allocation), even with
   `FTENT_PERSIST`, and gives the slot to the next effect. The callback therefore sets `die` one second ahead each frame, and
   `Think` makes a new proxy when the old one is gone or the slot now belongs to another effect (the callback pointer tells).
   Without this the props disappear after about a second.

To add a prop the client draws: place an `env_model` without a `targetname`, set `model` and, for an animated one, the sequence
and `m_iAction_On`.

### Environment

* **Fog**: `dod_fog.cpp`, see the cvars above.
* **Rain and snow**: `weather.cpp`. `CWeatherManager` creates particles around the player on a timer from
  `HUD_DrawTransparentTriangles` (`g_iWeatherType` comes from `info_doddetect`).
* **Particles**: `particleman/` is the original particle manager. It is built **into** the client (no separate library) and
  loaded through `CL_LoadParticleMan`. The DoD particle classes are in `tri.cpp` / `tri.h` (`CDoDParticle`, `CDoDRocketTrail`,
  `CDoDDirtExploDust`, `CDoDSnowFlake`, ...).
* **Corpses**: `EV_RoundReset`, `CreateCorpse`, `HitBody`, `RemoveBody` in `ev_dod.cpp` keep the dead bodies
  (`g_DeadPlayerModels[MAX_PLAYERS]`) as temp entities with the flags `FTENT_CORPSE` and `FTENT_BODYSTATIC`.

### Events

The client registers 49 events in `Game_HookEvents` (`cl_dll/dod/dod_events.cpp`), in the order of the original:
`events/weapons/*.sc` for every weapon (colt, luger, garand, thompson, mp44, ..., mortar, melee), and
`events/misc/*.sc` / `events/effects/*.sc` for train, pain, whizz, voices (`usvoice`, `gervoice`), round restart/reset,
camera, smoke, blood, bloodstream, tracers, bubbles, explosion, sparkshower, bodydamage, helmet, overheat, rockettrail and
mortarshell. The handlers are in `ev_dod.cpp`. The surface hit effects (dirt, sand, grass, wood, glass, foliage, snow,
tile, metal, water) pick the material from `pm_shared/pm_materials.h`.

### Stance and deploy state

The server sends the state of the player in `entvars_t::iuser3` (`g_iUser3` on the client) and the first component of
`vuser1` (`g_iVuser1x`). The values have names in `cl_dll/dod_shared.h`:

| Name | Value | Meaning |
|------|-------|---------|
| `STANCE_PRONE` | 1 | Prone. |
| `STANCE_PRONE_DEPLOYED` | 2 | Prone with the bipod deployed. |
| `STANCE_MORTAR` | 3 | The mortar is deployed. |
| `SANDBAG_DEPLOYED` | 2 | The weapon is deployed on a sandbag (`vuser1.x`). |

### Model rendering

DoD does not use the stock HL renderer. `CStudioModelRenderer` keeps a **bone cache** (a hash table by the name of the
bone) so weapons and `MOVETYPE_FOLLOW` models can be attached to a bone of another model. `CGameStudioModelRenderer`
animates the players: the gait (`gaitsequence`), the torso and leg turn, the nine direction pitch blend, and the poses
for the bipod and sandbag (`bipod_*`, `sandbag_*` sequences). The p-model of the weapon in the hands is swapped on the
events 7000 / 7002 (`leftreload`). The first person model is not drawn while the player is zoomed (FOV 89 or less), the
scope sprite is drawn by `CHudScope` instead. `StudioDrawModel` also draws the static `env_model` props of the map, see
[Static props](#static-props-cclientenvmodel).

### Other systems

* **Scope** (`scope.cpp`): the sniper scope is drawn with the Tri API in four quads.
* **Mortar** (`mortarhud.cpp`): the HUD of the mortar with a ballistic trajectory cache. The class never shipped in the
  game; the code is kept as in the original.
* **Crosshair** (`dod_crosshair.cpp`): the crosshair of the weapon, with the accuracy from `P_WpnInfo` (`dlls/dod_shared.cpp`).
* **Icons** (`dod_icons.cpp`): health, stamina, objective and area icons; `ShowHideWeapons`.
* **`CHudDoDCommon`** (`dod_common.cpp`): `GameRules`, `CameraView`, `prone`, and the gun lowering when you fall or leave the screen.
* **Localization** (`vgui/dod_localize.cpp`): reads `resource/dod_english.txt` and `valve_english.txt` and replaces `#Tokens`.
* **Server tables** (`dlls/dod_shared.cpp`): weapon names, class info, voice files, map marker icons.

## Weapon prediction

The server weapon code (`dlls/wpn_shared/*.cpp`) is compiled into the client and runs for the local player. The glue is
in `cl_dll/dod/`:

* `dod_baseentity.cpp`: empty bodies for the server methods the weapon classes link against.
* `dod_weapons.cpp`: the prediction loop (`HUD_InitClientWeapons`, `HUD_WeaponsPostThink`): a static pool of weapon
  objects and one local `CBasePlayer`.
* `dod_events.cpp`: `Game_HookEvents` and the declarations of the `EV_*` handlers.
* `dod_objects.cpp`: client rays and objects (`Game_AddObjects`).

The server-style files include `extdll.h`, which brings `dlls/vector.h`, and must **not** include `hud.h` (it brings
`cl_dll/util_vector.h`, a second `Vector` class). They reach the HUD through the functions declared in `com_weapons.h`
and defined in `com_weapons.cpp`: `HUD_GetRoundState`, `HUD_SetRecoil`, `HUD_SetScope`, `HUD_SendWeaponAnim`, `HUD_PlaySound`
and so on. Add a function there when prediction needs one more piece of the HUD.

A weapon class returns its group from `Classify()`: `CLASS_MELEE`, `CLASS_GRENADE`, `CLASS_GRENADE_EX`, `CLASS_PISTOL`,
`CLASS_RIFLE`, `CLASS_SCOPED_RIFLE`, `CLASS_SMG`, `CLASS_MACHINEGUN` (the weapons with a bipod), `CLASS_ROCKET`
(`dod_shared.h`).

## VGUI1 instead of VGUI2

Day of Defeat 1.3 draws its menus with **VGUI2**: `vgui2::Panel`, layouts in KeyValues `.res` files, `ILocalize`,
`ISurface`, an HTML control and the `IViewPort` of `game_controls`. Xash3D FWGS has no VGUI2 and there are no sources of the
original panels. So everything under `cl_dll/vgui/` is **not a 1:1 reverse** of the original code. It is a replacement of
VGUI2 built on **freevgui**, the VGUI1 compatible library that comes with hlsdk-portable (`vgui.dll` / `libvgui.so`).

What the original gives and this code keeps: the layouts (the game ships the `.res` files and the client reads them), the
flow (which message opens which panel, which command a button sends), the logic seen in IDA (sorting of the scoreboard,
availability of the classes, the three states of the spectator GUI), the colors and the fonts of `ClientScheme.res`,
and the look measured on the screenshots of the original. What it does not keep: any VGUI2 code. Every VGUI2 service the
panels rely on has a small stand-in here.

### What replaces what

| In the original (VGUI2) | Here | Files |
|-------------------------|------|-------|
| `IViewPort` / `IDoDViewPort` of game_controls: the root of all panels and the receiver of user messages | `DoDViewport`, derived from `TeamFortressViewport` of VGUI1, plus the interfaces `IDoDViewPort` and `IDoDViewPortMsgs`. The HUD calls it through `gDoDViewPortInterface`; the order of the methods in `idodviewport.h` is the order of the vtable of the original. | `dodviewport.*`, `idodviewport.h`, `idodviewportmsgs.h`, `dodviewport_interface.cpp` |
| `LoadControlSettings( "Resource/UI/X.res" )` | `CDoDKeyValues` parses the file, `CDoDResPage` creates the controls | `dodkeyvalues.*`, `dodrespage.*` |
| `ImagePanel`, `ProgressBar`, `RichText`, `Label`, `MouseOverPanelButton` | `CDoDImagePanel`, `CDoDProgressBar`, the text control with a scroll bar, `vgui::Label`, `CDoDButton` | `dodrespage.*`, `dodbutton.*` |
| `ScrollBar` and the scroll panel | `CDoDScrollPanel` with the arrow buttons of the original (no slider), mouse wheel | `dodscrollpanel.*` |
| HTML control (a browser behind the MOTD) | `dodhtml`, a small HTML and CSS engine | `dodhtml.*` |
| `SectionedListPanel` of the scoreboard | `ScorePanel` draws the header, the team rows and the player rows itself | `dodclientscoreboard.*` |
| `ClientScheme.res` (fonts, colors) | Constants and a font table | `dodmenu.*` |
| `vgui2::localize()` (`ILocalize`) | `DoD_FindLocalized`, `DoD_ConstructLocalized` | `dod_localize.*` |
| `vgui2::surface()` text drawing in the HUD | `CHudVGUI2Print`, drawn with the console font of the engine | `../hud_vgui2print.cpp` |

### The panels

| Panel | Layout of the game | Notes |
|-------|--------------------|-------|
| Team menu `CDoDTeamMenu` | `Teammenu.res` | Five buttons with the `jointeam` commands, player lists on mouse over, map description with a scroll bar. |
| Class menus `CDoDClassMenu` | `Classmenu_Allies`, `_Allies_Brit`, `_Allies_Para`, `_Axis`, `_Axis_Para` and `Classes/*.res` | Buttons follow the class mask from the server, a full class is dim. The page of a class (picture, five bars, description) opens on mouse over. |
| MOTD `CDoDMOTD` | `MOTD.res` | The page of the server goes through `dodhtml`. |
| Scoreboard `ScorePanel` | `ScoreBoard.res` | Window 444x360, teams and spectators in sections, colors of the teams. |
| Spectator GUI `CDoDSpectatorGUI` | `Spectator.res`, `BottomSpectator.res` | Three states (hidden, name of the player, bottom bar with the lists). Lists: options, camera, players. |
| Text window `CDoDTextWindow` | `TextWindow.res` | The help of the spectator and the description of the map. |
| Hint panel `CHintPanel` | `HintPanel.res` | Messages of `HudText` with the hint flag. |
| Mortar panel `CMortarPanel` | `MortarPanel.res` | The class is not in the game, restored from the binary for completeness. |
| Widescreen | | Deferred: the original draws nothing for it. |

A menu is opened the same way as in the original. The server sends `VGUIMenu` with an id (2 team, 4 briefing, 5 intro,
10..14 the class menus, 15 the mortar), `DoDViewport::ShowVGUIMenu` creates the panel from its layout, and the buttons send
the commands written in the layout (`jointeam`, `cls_*`, `vguicancel`).

### The layout loader

The `.res` files are **read from the game directory at runtime**, the same files VGUI2 reads (`dod/resource/UI/*.res`,
`dod/resource/classes/*.res`). Change a position, a color or a command in a file and the panel follows; the code only
creates the controls. `CDoDResPage` knows `Label`, `ImagePanel`, `ProgressBar`, `RichText`, `Button`, `DoDButton` and
`MouseOverPanelButton`; other controls (`Frame`, `Panel`, `Menu`) only keep their rectangle.

The layouts are in 640x480. They are scaled by the **height** of the screen on both axes (`YRES`), as the original does,
and the panel is centered, so a 16:9 screen gets side margins and no stretching.

### Fonts, colors and text encoding

* The fonts are the ones of `ClientScheme.res` (`ClassMenuDefault`, `ClassMenuTitleFont`, `ClassMenuBoldFont`,
  `ClassMenuBodyFont`, `Default`, `DefaultVerySmall`), created with freevgui at a size scaled with the height of the screen.
  VGUI2 draws text about 12% smaller than the same tall in VGUI1, so the size is multiplied by `DODMENU_FONT_SCALE`
  (0.88; 0.84 for `Default`), measured on screenshots at 1920x1200.
* The colors are the constants `DODCOLOR_*` in `dodmenu.h`. Note that the alpha of VGUI1 is a transparency: 0 is opaque.
* VGUI1 draws a string byte by byte in a one-byte charset. The localization is UTF-8, so a text is converted to
  Latin-1 for the menus (`DoD_CodePointToLatin1`) unless the engine cvar `vgui_utf8` is 1, and then it stays UTF-8.

### Localization

`dod_localize.cpp` is the stand-in for `ILocalize`. It reads the UTF-16LE KeyValues files `resource/valve_english.txt`
(the base game) and then `resource/dod_english.txt` (a token of the later file replaces the same token of the earlier
one), falls back to `titles.txt`, and replaces `%s1` .. `%s4` in a format with the arguments of a message. Both the panels
and the HUD use it: `DoD_FindLocalized( "#Token" )`, `DoD_ConstructLocalized( ... )`.

### HUD text: `CHudVGUI2Print`

`CHudVGUI2Print` is **not an original implementation**. In the original it draws localized text with VGUI2 fonts through
`vgui2::surface()` and `vgui2::localize()`. The HUD code of the game calls it (center messages, chat, text menus, timers), so
the class keeps the **interface of the original** and the callers stay as they were reversed, but the body is new: the text
is drawn with the **console font of the engine** (`pfnDrawConsoleString`).

| Function | What it does here |
|----------|-------------------|
| `DrawVGUI2String( token, x, y, r, g, b )` | Looks the token up (`DoD_FindLocalized`, the token itself when unknown), draws the line, returns the x where it ended. `x == -1` centers it horizontally, `y == -1` vertically. |
| `DrawVGUI2StringReverse( token, x, y, ... )` | The same, but the line ends at `x` (right aligned). Returns the x where it starts. |
| `DrawLocalizedString`, `DrawLocalizedStringReverse` | Draw a line that is already localized (the original takes a `wchar_t` string). |
| `VGUI2HudPrint( token, x, y, r, g, b )` | Shows a token for 4 seconds from `Draw`. Unknown tokens are not shown. |
| `VGUI2HudPrintArgs( token, s1, s2, s3, s4, x, y, r, g, b )` | The same, with `%s1` .. `%s4` filled in. Unknown tokens are shown as they are. |
| `GetHudFontHeight`, `GetStringSize` | Metrics of the console font. |

The text has a one pixel black shadow. It is used by `TextMsg` (`print_center`), the chat lines (`CHudSayText`), the text menus
(`CHudMenu`), the clan match countdown (`#Clan_time_remaining`, `#Clan_match_live`), the current map marker, the clan warm-up label and the
"MG reload" hint. Because it is the console font, the size and the look follow the font settings of the engine (for example
`con_fontscale`), not the fonts of `ClientScheme.res`, and the widths differ from the original. `hud_textmode 2` makes the
older `DrawHudString` use the same font.

To print a new localized message from the HUD, add the token to `dod_english.txt` and call `VGUI2HudPrint` with it.

### Fallbacks for a build or a user without the VGUI panels

`cl_motd_vgui 0` shows the MOTD as plain text in the HUD (`MOTD.cpp`) and `cl_scoreboard_vgui 0` uses the text scoreboard of the
HUD (`scoreboard.cpp`). Both are the hlsdk-portable classes, kept for this.

### What is not restored

* Right click on a row of the scoreboard (spectate this player) and the voice icon next to a name.
* The HLTV banner picture (it is empty until a HLTV server sends `DRC_CMD_BANNER`) and the `inset` window.
* The exact highlight of the entry under the mouse in the spectator lists (the color is taken from the scheme, no screenshot of the original).
* HTML in the MOTD: images, links and scripts. VGUI1 has no browser, `dodhtml` lays out text, headings, lists, tables and `<style>`.

### Notes for mod makers

* To change the look of a menu, edit its `.res` first.
* A new panel: a class in `cl_dll/vgui/` that builds a `CDoDResPage` from its layout, a case in `DoDViewport::ShowVGUIMenu`, and, if the server opens it, a new id in the `VGUIMenu` enum of `dodviewport.h`.
* `vgui/vgui_*.cpp` is the part of freevgui that has to stay inside the client (scheme manager, custom objects, `vgui_int.cpp` that creates the viewport).

## Differences from hlsdk-portable

* The HL weapons, monsters and the VGUI1 TF menus are gone; there is no server code.
* `CHudHealth` is removed; `CHudDodIcons` draws the health, and the `Damage` message goes to `CHud::MsgFunc_Damage` like in the original.
* `CHudBattery`, `CHudFlashlight`, `CHudGeiger`, `CHudAmmoSecondary` and `HistoryResource` are removed.
* `pm_shared` is the one of Day of Defeat: prone, bipod, mortar, stamina jump, DoD materials and step sounds.
* The view is the DoD one: lateral bob only, a second view for the map, the death camera, mortar pitch.
* The input has the DoD button checks (`DoDHandleButtons`) and the speed from the stamina (`DoDGetSpeed`).
* `input_xash3d.cpp` limits the view of a deployed weapon to +-45 degrees yaw and +-20 degrees pitch.
