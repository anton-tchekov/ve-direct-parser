# VE-Direct Parser GUI

Simple GUI for Victron VE-Direct Serial Protocol.
Uses SDL2 and SDL_ttf. Works on Linux only.

Test on BlueSolar MPPT 100|20 48 V:

![Screenshot](images/screenshot.png)

Next Day:

![Screenshot](images/screenshot2.png)

## TODO

- Detect if, device is MPPT or other
- HEX Protocol: Display Past Data
- Use poll(): Communication FDs for main thread<->Serial Thread
- Properly join thread and close file on exit
