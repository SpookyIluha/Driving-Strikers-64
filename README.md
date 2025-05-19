# Driving Strikers 64
A port of the Driving Strikers indie game to the Nintendo 64 using Libdragon SDK, Tiny3D and a custom-made codebase in C.

![boxart](https://github.com/user-attachments/assets/a9567dbd-5887-4077-9705-ca01d82db943)

This is an indie game that was originally released on Dreamcast and was ported across various platforms. With permission from the author I've ported it onto the Nintendo 64 platform as well.
This is a 6-th gen game, that runs well on a 5-th gen system with minimal compromises and some improvements from the Dreamcast version.

This game uses advanced features, not previously seen in any N64 official title, that is a result of extensive research and optimization.

Expansion Pak is ___NOT___ needed, but recommended to play this game.

This release of the N64 port of Driving Strikers has been approved for distribution by the original developers of the game; Luke Benstead and David Reichelt (LD2K). The gameplay and style may vary from the original release. If you enjoy this port please consider supporting the developers by purchasing the original game on [Steam](https://store.steampowered.com/app/2384430/Driving_Strikers/) (PC/Linux) or [Itch.io](https://reality-jump.itch.io/driving-strikers).

# Known issues:
- EEPROM may not work on older Everdrive flashcart models, or with outdated software (libdragon incompatibility)

# Controls:
- Stick: move around
- Z trigger: nitro
- L/R triggers: jump

# Features:
- Physics-driven gameplay using the [Tiny Physics Engine](https://github.com/ESPboy-edu/ESPboy_tinyphysicsengine) open-source library
- Multilanguage support with 8 different languages using .ini configs and [inih](https://github.com/benhoyt/inih) open-source library
- Stereo CD quality music with voiced audio on a 16MB cartridge
- Full 480i TrueColor graphics at high framerates of up to 60 FPS ___without the expansion pak___, extremely rare in N64 titles
- Quality artwork and levels with antialiasing and high-res fullcolor mipmapped, bumpmapped textures
- HDR rendering and bright lighting with autoexposure, never before seen on any N64 title
- Rumble Pak and EEPROM save support

# Build the game
- Install [Libdragon SDK](https://github.com/DragonMinded/libdragon/tree/unstable) (unstable)
- Use the [texparms PR](https://github.com/DragonMinded/libdragon/pull/667) from the libdragon unstable PR's
- Install [Tiny3D](https://github.com/HailToDodongo/tiny3d/tree/no_light) library (no_light branch or later)
- Clone this repository
- Add your Driving Strikers assets from the Release to the "filesystem" folder
- Build it with make command

# Screenshots

![drivingstrikers64 2025-05-12 01-32-02](https://github.com/user-attachments/assets/ecf9522f-bc18-4e72-a50b-384b89b180b2)
![drivingstrikers64 2025-05-10 16-11-51](https://github.com/user-attachments/assets/29db1ff2-2a90-40e1-8186-97666a1df0ab)
![drivingstrikers64 2025-05-10 00-40-28](https://github.com/user-attachments/assets/77c22f7c-128f-4236-bc38-674b3cc71fe5)
![drivingstrikers64 2025-05-10 00-40-13](https://github.com/user-attachments/assets/17ef8e04-420f-444e-971c-691ea16f2e78)
![drivingstrikers64 2025-05-09 16-35-00](https://github.com/user-attachments/assets/29ac863e-2b88-450e-bd9b-33cc7be8c240)
![drivingstrikers64 2025-05-06 11-48-03](https://github.com/user-attachments/assets/22263558-a7ee-4a37-a985-d111f4401768)
![drivingstrikers64 2025-05-06 11-47-37](https://github.com/user-attachments/assets/769c4fdb-0a27-4bb6-b2ab-78419cca0e37)
![drivingstrikers64 2025-04-30 01-07-46](https://github.com/user-attachments/assets/d8161f4f-937b-401c-ad47-a2efbe1cbe2e)

# Visual comparison between N64 (left) with DC (right) version

![image](https://github.com/user-attachments/assets/6e5c92e9-08d2-48fa-8a41-99df41b86029)
![image](https://github.com/user-attachments/assets/3c6f979f-eb54-44b4-bff4-16a6dc151897)
![image](https://github.com/user-attachments/assets/892c1f4b-818f-4f1d-9d1b-a147288e3bad)
![image](https://github.com/user-attachments/assets/53b9b09f-0a60-4b50-aaa3-3732de8748ca)
![image](https://github.com/user-attachments/assets/ea007ff9-b710-4e11-967a-1c55f5f47e84)

