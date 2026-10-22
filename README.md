<a id="english"></a>
**English** | [Русский](#russian)

# DOD13-Client [![Build Status](https://github.com/whamemer/dod13-client/actions/workflows/build.yml/badge.svg)](https://github.com/whamemer/dod13-client/actions) <img align="right" width="128" height="128" src="android/app/src/main/ic_launcher-playstore.png" alt="DOD13-Client" />
Reverse-engineered client library of **Day of Defeat 1.3** for [Xash3D FWGS](https://github.com/FWGS/xash3d-fwgs).

The code base is [hlsdk-portable](https://github.com/FWGS/hlsdk-portable) with the Day of Defeat client restored on top of it,
function by function, from the Steam version of the game. It replaces `client.dll` / `client.so` of the original game and works
with the original server library.

> **Status.** The client is playable and considered ready for testing. It is developed and tested on **Windows x86** with the
> latest developer build of Xash3D FWGS, and it is checked on an **Android** phone (the debug APK: touch controls, HUD, menus).
> Windows x64 and Linux are built by the CI, but nobody has played them yet. The CI also has a macOS build (Apple Silicon and
> Intel) that has not been run on a Mac yet. Bug reports are welcome.

## What is restored
* **Weapons.** All the weapons of the game with client-side prediction (bipods, sandbags, the mortar, scopes), weapon events,
  muzzle flashes, shells, tracers, surface hits, explosions, blood and corpses.
* **HUD.** Health and stamina, ammo, objective icons and capture progress, the map (small and large) with markers and
  grenades, crosshair, kill notices, chat, hint messages, spectator interface.
* **Effects.** The particle manager of the original game (built into the client), rain and snow, fog, muzzle smoke,
  rubble, static map props (`env_model`), particle shooters and the cinematic camera, all read from the entity lump of the map.
* **Movement and rendering.** Player movement shared with the server (prone, ladders, stamina jump), the model renderer with the
  poses of the bipod and the sandbag, view bob and recoil.
* **Menus.** Team and class menus, MOTD, scoreboard, spectator panel. The original uses VGUI2, which Xash3D does not have, so the
  panels are rebuilt on [freevgui](https://github.com/FWGS/freevgui) (VGUI1) from the layouts of the game (`resource/UI/*.res`).
* **Bug fixes of the original client**, optional and switchable at build time (see [Building](#building)).

## Download
Builds are published on the [Releases](https://github.com/whamemer/dod13-client/releases) page: `windows` (32-bit), `windows_x64`,
`linux` and, if their build passed, `macos_arm64` and `macos_x64`, each archive holds the `dod` folder that goes next to the engine, with the client (`dod/cl_dlls/client.dll`, or
`client.so`), the main menu (`dod/cl_dlls/menu.dll`, or `menu.so`) and the extras (`dod/extras.pk3`).

## Installation
1. Get the [latest developer build of Xash3D FWGS](https://github.com/FWGS/xash3d-fwgs/releases/tag/continuous) and unpack it.
2. You have to own [Day of Defeat on Steam](https://store.steampowered.com/app/30/Day_of_Defeat/). Copy the `valve` and `dod`
   folders from your Steam installation into the Xash3D FWGS directory. The game files are not distributed with this project.
3. Unpack the build into the same directory, so that `client.dll` and `menu.dll` land in `dod/cl_dlls/` and `extras.pk3` in `dod/`.
4. Run `xash3d.exe -game dod`.

On **Android** install the APK next to the Xash3D FWGS app: it declares itself as a mod of that app (`su.xash.engine.MOD`). The
client, the menu and the extras are inside the APK, the game files of the original game (`valve`, `dod`) are not.

### Notes
* **The view does not tilt when strafing.** The tilt comes from the server cvar `sv_rollangle`. The default of GoldSrc is `2`, the
  default of Xash3D is `0`. `userconfig.cfg` sets `sv_rollangle 2` and `sv_rollspeed 200` for a server that you start from the
  game, a dedicated server needs them in its own config.
* The menus look slightly different from the original, see the description of VGUI1 in [cl_dll/readme.md](cl_dll/readme.md#vgui1-instead-of-vgui2).

## Documentation
Everything about the client is in [cl_dll/readme.md](cl_dll/readme.md):
* [Layout](cl_dll/readme.md#layout) of the source code;
* [Console variables](cl_dll/readme.md#console-variables) and [console commands](cl_dll/readme.md#console-commands-of-the-client)
  of the client, with defaults;
* [Build options](cl_dll/readme.md#build-options) and the [fixes of the bugs of the original game](cl_dll/readme.md#fixes-of-the-bugs-of-the-original-game);
* [How the engine talks to the client](cl_dll/readme.md#how-the-engine-talks-to-the-client) and the
  [user messages](cl_dll/readme.md#user-messages-and-console-commands);
* [Mechanics that exist only in Day of Defeat](cl_dll/readme.md#mechanics-that-exist-only-in-day-of-defeat) and
  [weapon prediction](cl_dll/readme.md#weapon-prediction);
* [VGUI1 instead of VGUI2](cl_dll/readme.md#vgui1-instead-of-vgui2), the
  [extras and the main menu](cl_dll/readme.md#extras-and-the-main-menu) and the
  [differences from hlsdk-portable](cl_dll/readme.md#differences-from-hlsdk-portable).

## Building
Clone the source code with the submodules:
```
git clone https://github.com/whamemer/dod13-client --recursive
```
The build is 32-bit by default. The result is installed as `dod/cl_dlls/client.dll`. You need a
C99 and C++11 compiler (MSVC, GCC or Clang) and either Python 3 (waf) or CMake 3.15 or newer.

### Options
`mod_options.txt` holds the options of the build, both build systems read it. `GAMEDIR` and `CLIENT_INSTALL_DIR` say where the
client is installed. Every fix of a bug of the original game has its own macro (`FOG_MINIMAP_FIX`, `SCOPE_WIDESCREEN_FIX`,
`MINIMAP_SPECTATOR_FIX`), all are `ON`. Set one to `OFF` to get the behavior of the original. They are described in
[cl_dll/readme.md](cl_dll/readme.md#fixes-of-the-bugs-of-the-original-game).

CMake also builds the main menu (`menu.dll`, option `BUILD_MAINUI`, `-DBUILD_MAINUI=OFF` turns it off) and packs
`3rdparty/dod13client-extras` into `extras.pk3`; both end up in the game directory with the client. waf builds the client and
`extras.pk3` only.

### waf
```
./waf configure
./waf build
./waf install --destdir=dist
```
On Windows use `waf.bat`. `-T debug` makes a debug build, `-8` allows a 64-bit build, `--disable-werror` turns off the strict
warnings of GCC and Clang.

### CMake
Windows x86:
```
cmake -A Win32 -S . -B build
cmake --build build --config Release
cmake --install build --config Release --prefix dist
```
Windows x64 (`-A x64 -D64BIT=ON`), Linux and macOS (`cmake -S . -B build`) use the same commands; on Apple Silicon
`-DCMAKE_OSX_ARCHITECTURES=x86_64` makes the Intel build. For Linux the CI builds in the
[Steam Runtime](https://github.com/ValveSoftware/steam-runtime) i386 environment, see
[.github/workflows/build.yml](.github/workflows/build.yml).

### Android
```
cd android
./gradlew assembleDebug
```
It needs JDK 17, Android SDK 34, NDK 26.1.10909125 and CMake 3.22.1 (see `android/app/build.gradle`). The APK is
`android/app/build/outputs/apk/debug/app-debug.apk`. It holds the client and the menu for arm64-v8a and armeabi-v7a, and the
extras as its assets (the engine mounts them as the files of the game). `assembleRelease` makes an APK that is not signed, so the
debug one is what the CI publishes. The APK is checked on a phone.

## Credits
* [FWGS](https://github.com/FWGS) for the engine, [Xash3D FWGS](https://github.com/FWGS/xash3d-fwgs), and for the SDK this code
  is based on, [hlsdk-portable](https://github.com/FWGS/hlsdk-portable).
* **Valve** for Day of Defeat.

## License
The code is based on the Half-Life SDK and is distributed under its license, see [LICENSE](LICENSE): it allows free,
non-commercial modifications of Valve games. Third-party parts keep their own licenses.

Day of Defeat and Half-Life are products of Valve Corporation. This project is not affiliated with or endorsed by Valve. It
contains no assets of the game, you need the original game to play.

---

<a id="russian"></a>
[English](#english) | **Русский**

# DOD13-Client [![Build Status](https://github.com/whamemer/dod13-client/actions/workflows/build.yml/badge.svg)](https://github.com/whamemer/dod13-client/actions) <img align="right" width="128" height="128" src="android/app/src/main/ic_launcher-playstore.png" alt="DOD13-Client" />
Клиентская библиотека **Day of Defeat 1.3**, восстановленная реверс-инжинирингом, для [Xash3D FWGS](https://github.com/FWGS/xash3d-fwgs).

За основу взят [hlsdk-portable](https://github.com/FWGS/hlsdk-portable), поверх которого функция за функцией восстановлен клиент
Day of Defeat из Steam-версии игры. Он заменяет `client.dll` / `client.so` оригинальной игры и работает с оригинальной серверной
библиотекой.

> **Состояние.** Клиент играбелен и готов к тестированию. Он разрабатывается и проверяется на **Windows x86** с последней
> developer-сборкой Xash3D FWGS, а также проверен на телефоне с **Android** (debug APK: сенсорное управление, HUD, меню).
> Windows x64 и Linux собирает CI, но в них пока никто не играл. В CI есть и сборка для macOS (Apple Silicon и Intel), её на
> Mac пока ни разу не запускали. Сообщения об ошибках приветствуются.

## Что восстановлено
* **Оружие.** Всё оружие игры с клиентским предсказанием (сошки, мешки с песком, миномёт, прицелы), события оружия,
  вспышки выстрелов, гильзы, трассеры, попадания в поверхности, взрывы, кровь и трупы.
* **HUD.** Здоровье и выносливость, патроны, значки объектов и прогресс захвата, карта (малая и большая) с метками и
  гранатами, перекрестие, сообщения об убийствах, чат, подсказки, интерфейс наблюдателя.
* **Эффекты.** Менеджер частиц оригинальной игры (встроен в клиент), дождь и снег, туман, дым выстрелов, обломки,
  статические пропсы карты (`env_model`), генераторы частиц и кинематографическая камера, всё это читается из entity lump карты.
* **Движение и рендер.** Движение игрока, общее с сервером (лёжа, лестницы, прыжок на выносливости), рендер моделей с позами
  сошек и мешка с песком, покачивание вида и отдача.
* **Меню.** Меню команд и классов, MOTD, таблица счёта, панель наблюдателя. Оригинал использует VGUI2, которого нет в Xash3D,
  поэтому панели перестроены на [freevgui](https://github.com/FWGS/freevgui) (VGUI1) по раскладкам игры (`resource/UI/*.res`).
* **Исправления багов оригинального клиента**, необязательные и включаемые при сборке (см. [Сборка](#сборка)).

## Загрузка
Сборки публикуются на странице [Releases](https://github.com/whamemer/dod13-client/releases): `windows` (32 бита), `windows_x64`,
`linux` и, если их сборка прошла, `macos_arm64` и `macos_x64`. В каждом архиве лежит папка `dod`, которая кладётся рядом с движком, с клиентом (`dod/cl_dlls/client.dll` или
`client.so`), главным меню (`dod/cl_dlls/menu.dll` или `menu.so`) и extras (`dod/extras.pk3`).

## Установка
1. Скачайте [последнюю developer-сборку Xash3D FWGS](https://github.com/FWGS/xash3d-fwgs/releases/tag/continuous) и распакуйте её.
2. У вас должна быть [Day of Defeat в Steam](https://store.steampowered.com/app/30/Day_of_Defeat/). Скопируйте папки `valve` и
   `dod` из установки Steam в каталог Xash3D FWGS. Файлы игры вместе с проектом не распространяются.
3. Распакуйте сборку в тот же каталог так, чтобы `client.dll` и `menu.dll` оказались в `dod/cl_dlls/`, а `extras.pk3` в `dod/`.
4. Запустите `xash3d.exe -game dod`.

На **Android** установите APK рядом с приложением Xash3D FWGS: он объявляет себя модом этого приложения (`su.xash.engine.MOD`).
Клиент, меню и extras находятся внутри APK, файлов оригинальной игры (`valve`, `dod`) в нём нет.

### Примечания
* **Вид не наклоняется при стрейфе.** Наклон задаёт серверная переменная `sv_rollangle`. Умолчание GoldSrc равно `2`,
  умолчание Xash3D равно `0`. `userconfig.cfg` ставит `sv_rollangle 2` и `sv_rollspeed 200` для сервера, который вы запускаете из
  игры, выделенному серверу они нужны в его собственном конфиге.
* Меню выглядят немного иначе, чем в оригинале, см. описание VGUI1 в [cl_dll/readme.md](cl_dll/readme.md#vgui1-вместо-vgui2).

## Документация
Всё о клиенте описано в [cl_dll/readme.md](cl_dll/readme.md#russian):
* [Структура](cl_dll/readme.md#структура) исходного кода;
* [Консольные переменные](cl_dll/readme.md#консольные-переменные) и [консольные команды](cl_dll/readme.md#консольные-команды-клиента)
  клиента, с умолчаниями;
* [Параметры сборки](cl_dll/readme.md#параметры-сборки) и [исправления багов оригинальной игры](cl_dll/readme.md#исправления-багов-оригинальной-игры);
* [Как движок общается с клиентом](cl_dll/readme.md#как-движок-общается-с-клиентом) и
  [пользовательские сообщения](cl_dll/readme.md#пользовательские-сообщения-и-консольные-команды);
* [Механики, которые есть только в Day of Defeat](cl_dll/readme.md#механики-которые-есть-только-в-day-of-defeat) и
  [предсказание оружия](cl_dll/readme.md#предсказание-оружия);
* [VGUI1 вместо VGUI2](cl_dll/readme.md#vgui1-вместо-vgui2),
  [extras и главное меню](cl_dll/readme.md#extras-и-главное-меню) и
  [отличия от hlsdk-portable](cl_dll/readme.md#отличия-от-hlsdk-portable).

## Сборка
Склонируйте исходный код вместе с подмодулями:
```
git clone https://github.com/whamemer/dod13-client --recursive
```
Сборка по умолчанию 32-битная. Результат устанавливается как `dod/cl_dlls/client.dll`. Нужен компилятор C99 и C++11 (MSVC, GCC
или Clang) и либо Python 3 (waf), либо CMake 3.15 и новее.

### Параметры
`mod_options.txt` содержит параметры сборки, его читают обе системы сборки. `GAMEDIR` и `CLIENT_INSTALL_DIR` говорят, куда
устанавливается клиент. У каждого исправления бага оригинальной игры есть свой макрос (`FOG_MINIMAP_FIX`, `SCOPE_WIDESCREEN_FIX`,
`MINIMAP_SPECTATOR_FIX`), все они `ON`. Поставьте `OFF`, чтобы получить поведение оригинала. Они описаны в
[cl_dll/readme.md](cl_dll/readme.md#исправления-багов-оригинальной-игры).

CMake также собирает главное меню (`menu.dll`, параметр `BUILD_MAINUI`, `-DBUILD_MAINUI=OFF` отключает его) и упаковывает
`3rdparty/dod13client-extras` в `extras.pk3`; то и другое попадает в каталог игры вместе с клиентом. waf собирает только клиент и
`extras.pk3`.

### waf
```
./waf configure
./waf build
./waf install --destdir=dist
```
В Windows используйте `waf.bat`. `-T debug` делает отладочную сборку, `-8` разрешает 64-битную сборку, `--disable-werror`
отключает строгие предупреждения GCC и Clang.

### CMake
Windows x86:
```
cmake -A Win32 -S . -B build
cmake --build build --config Release
cmake --install build --config Release --prefix dist
```
Windows x64 (`-A x64 -D64BIT=ON`), Linux и macOS (`cmake -S . -B build`) используют те же команды; на Apple Silicon
`-DCMAKE_OSX_ARCHITECTURES=x86_64` делает сборку для Intel. Для Linux CI собирает в окружении
[Steam Runtime](https://github.com/ValveSoftware/steam-runtime) i386, см.
[.github/workflows/build.yml](.github/workflows/build.yml).

### Android
```
cd android
./gradlew assembleDebug
```
Нужны JDK 17, Android SDK 34, NDK 26.1.10909125 и CMake 3.22.1 (см. `android/app/build.gradle`). APK лежит в
`android/app/build/outputs/apk/debug/app-debug.apk`. В нём клиент и меню для arm64-v8a и armeabi-v7a, а extras это его assets
(движок монтирует их как файлы игры). `assembleRelease` делает неподписанный APK, поэтому CI публикует debug-вариант. APK
проверен на телефоне.

## Благодарности
* [FWGS](https://github.com/FWGS) за движок, [Xash3D FWGS](https://github.com/FWGS/xash3d-fwgs), и за SDK, на котором основан
  этот код, [hlsdk-portable](https://github.com/FWGS/hlsdk-portable).
* **Valve** за Day of Defeat.

## Лицензия
Код основан на Half-Life SDK и распространяется под его лицензией, см. [LICENSE](LICENSE): она разрешает свободные
некоммерческие модификации игр Valve. Сторонние части сохраняют свои лицензии.
