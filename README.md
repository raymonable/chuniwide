# Chuniwide

This is a fork of Chuniwide with cleaned up source and support for newer game versions.<br>
Versions starting from 2.40 and up (including future versions) should be compatible.

### Installation

1. Download the appropriate DLL (25inch or 27inch) from the [Releases](https://github.com/raymonable/chuniwide/releases) tab.
2. Place the DLL next to your `chusanApp.exe` (found in the `bin` folder). Make sure you note it's name.
3. Open your `launch.bat` (or `start.bat`) in Notepad, and adjust the following line:
```
inject_x86 -d -k chusanhook_x86.dll chusanApp.exe
```
After `chusanhook_x86.dll` add either `-k chuniwide_27inch.dll` or `-k chuniwide_25inch.dll`. Save the file.
4. If your game isn't set to windowed mode, ensure it is (this is managed in `segatools.ini`).
5. Launch the game.