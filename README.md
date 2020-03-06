# disruptor_ripper
An archive extractor tool for the PlayStation (PSX) game Disruptor (1996).
 
## Disclaimer
This project is a total mess, partly because the game archive itself is very poorly structured (it's designed around reducing CD's load times), and partly because
it's the result of several tweaks to make the final extractor extract all of the graphics with little attention paid towards proper form.</br>
This definitely needs some major polishing/refactoring, but since it's been rotting on the HDD since 4 years as of this writing I figured I might as well host it
and fix it sometime in the future (which most likely won't be anytime soon).</br>
I've also included IDA's database in the misc/ folder where I named some routines of potential interest, but I advise to take it with a grain of salt.
 
Keep in mind that this works only with the PAL version (SLES_005.35); it will crash with the others, so make sure to get the correct one.
 
 
## Usage
Either open a command prompt and type

    disruptor_ripper <WAD.IN's path>
, or (on Windows) drag-and-drop the archive WAD.IN over the executable; you will then be prompted to choose what you want to extract, which is pretty straightforward
and doesn't need to be explained here.
 
Stuff will be extracted in the same path as the WAD.IN archive, so be sure to place it in a directory where you have write permissions before passing it to the exe.
 
 
## Extracted material
- Sprites (Enemies, HUD, weapons, psyonics)
- Sounds (In-game, loading screen speeches, psyonics, weapons)
 
All picture-related material is extracted as .TGA pictures, which can be viewed fine with
IrfanView (albeit without transparency), SLADE3, MtPaint, and possibly other image viewers I'm unaware of;
if that's still a problem for you, Ken Silverman's PNGOUT is your friend :)
