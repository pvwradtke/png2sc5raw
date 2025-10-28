/*

Copyright 2025, Paulo Vinicius Radtke

This file is distributed under the MIT license.

To compile, install SDL2 and SDL2 Image and run:

	gcc png2sc8raw.c -lSDL2 -lSDL2_image -o png2sc8raw

Or:

    make
    make install

*/

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

uint8_t picture[65536];
uint8_t palette[256];

int main(int argc, char* argv[]) {
    if(argc<3){
    	printf("Usage: png2scr8raw <pngfilename> <raw filename>\n");
    	return 1;
    }
    // Initialize SDL and SDL_image
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG); // Initialize for PNG and JPG support

    // Load an image
    SDL_Surface* msx = IMG_Load(argv[1]);
    if (!msx) {
        printf("Can't load the file\n");
        return 1;
    }
    if(msx->format->palette->ncolors>256){
        printf("PNG doesn't have 256 indexed colors. Colors: %d\n", msx->format->palette->ncolors);
        return 1;
    }
    printf("File %s - Width: %d, Height: %d, %d colors\n",
    	argv[1], msx->w, msx->h, msx->format->palette->ncolors);
    if(msx->w != 256){
        printf("PNG image needs to be 256 pixels wide.\n");
        return 1;
    }
    if(msx->h > 256){
        printf("PNG image height is larger than 256 (single screen 5 page).\n");
        return 1;
    }
 
    for(int i=0;i<msx->format->palette->ncolors;i++)
	palette[i] = (msx->format->palette->colors[i].g/32)<<5 | (msx->format->palette->colors[i].r/32) << 2 | (msx->format->palette->colors[i].b/64);
    // Converts the pixels
    for(int i=0;i<msx->h;i++){
    	for(int j=0;j<msx->w;j++)
    		picture[i*256+j]=palette[((char*)msx->pixels)[i*msx->w+j]];
    }
    FILE *out = fopen(argv[2], "wb");
    if (!out) {
        printf("Can't open raw image at %s\n", argv[2]);
        return 1;
    }
    int written = fwrite(picture, 1, msx->h*msx->w, out);
    fclose(out);
    if (written!=msx->h*msx->w) {
        printf("Can't write raw image at %s\n", argv[2]);
        return 1;
    }


   // Clean up
    SDL_FreeSurface(msx);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
