Copyright 2025, Paulo Vinicius Radtke

This file is distributed under the MIT license.

This will convert a 16 colors indexed PNG file to a RAW MSX2 Screen 5 picture. It generates both a RAW image (ready to use in the VRAM) and the associated color palette, already normalized for the MSX RGB format.

There's no constraints in the palette, and you can use color 0 (border/transparency) to draw. Just keep in mind that if you use color 0, it'll changer the screen border. The tool was developped to take a 256xn pixels image, where n is the number of lines, up to 1024 (all four screen 5 pages). 

If the width is larger than 256, the tool will abort, same for a picture taller than 1024 lines. Finally, you may also convert images with a width smaller than 256, but given this is a RAW file, there's no information about the dimension and it's up to you to ensure that the proper dimensions are used in your game.

Usage:

   png2scr5raw <pngfilename> <raw filename> <palette filename>

Example:

   png2scr5raw background.png background.raw background.pal
