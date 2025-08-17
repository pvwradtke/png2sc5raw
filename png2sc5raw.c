/*

Copyright 2025, Paulo Vinicius Radtke

This file is distributed under the MIT license.

To compile, install SDL2 and SDL2 Image and run:

	gcc png2xpm.c -lSDL2 -lSDL2_image -o png2xp

*/

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

uint16_t palette[16];
uint8_t picture[131072];

int main(int argc, char* argv[]) {
    if(argc<4){
    	printf("Usage: png2scr5raw <pngfilename> <raw filename> <palette filename>\n");
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
    if(msx->format->palette->ncolors!=16){
        printf("PNG doesn't have 16 indexed colors\n");
        return 1;    
    }
    printf("File %s - Width: %d, Height: %d, %d colors\n", 
    	argv[1], msx->w, msx->h, msx->format->palette->ncolors);
    if(msx->w > 256){
        printf("PNG image width is larger than 256.\n");
        return 1;    
    }
    if(msx->w > 1024){
        printf("PNG image height is larger than 1024 (all for screen 5 pages).\n");
        return 1;    
    }
    
    for(int i=0;i<msx->format->palette->ncolors;i++){
	palette[i] = (msx->format->palette->colors[i].g/32)<<8 | (msx->format->palette->colors[i].r/32) << 4 | (msx->format->palette->colors[i].b/32);
    	//printf("Color: %d - %02X, %02X %02X\n", i, msx->format->palette->colors[i].r/32,msx->format->palette->colors[i].g/32, msx->format->palette->colors[i].b/32);
    	//printf("   Color MSX: %04X\n", palette[i]);
    }
    FILE *out = fopen(argv[3], "wb");
    if (!out) {
        printf("Can't open palette file at %s\n", argv[3]);
        return 1;
    }
    int written = fwrite(palette, 2, 16, out);
    fclose(out);
    if (written!=16) {
        printf("Can't write palette in %s\n", argv[3]);
        return 1;
    }
    // Converts the pixels
    for(int i=0;i<msx->h;i++){
    	for(int j=0;j<msx->w/2;j++)
    		picture[i*128+j]=((char*)msx->pixels)[i*msx->w+j*2]<<4 | ((char*)msx->pixels)[i*msx->w+j*2+1];
    }
    out = fopen(argv[2], "wb");
    if (!out) {
        printf("Can't open raw image at %s\n", argv[2]);
        return 1;
    }
    written = fwrite(picture, 1, msx->h*msx->w/2, out);
    fclose(out);
    if (written!=msx->h*msx->w/2) {
        printf("Can't write raw image at %s\n", argv[2]);
        return 1;
    }
    	
    
   // Clean up
    SDL_FreeSurface(msx);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
