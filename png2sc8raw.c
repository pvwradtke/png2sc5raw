/*

Copyright 2025, Paulo Vinicius Radtke

This file is distributed under the MIT license.

To compile, install SDL2 and SDL2 Image and run:

	gcc png2sc8raw.c -lSDL2 -lSDL2_image -o png2sc8raw

Or:

    make
    make install

*/



#include <SDL2/SDL_surface.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include "screen8pal.h"

#define ZOOM    8

typedef struct Color{
    uint8_t r, g, b;
    double   rn, gn, bn, light;
}Color;

uint8_t palcolorsrg[8]={0, 39, 82, 115, 155, 186, 224, 255};
uint8_t palcolorsb[4]={0, 82, 155, 255};

Color   palette[256];
enum METHODS { DIVISION, RGBDIST, RGBLDIST, RGBNDIST, RGBNLDIST, MAXMETHODS };
char methods[MAXMETHODS][12]={
    "DIVISION", "RGBDIST", "RGBLDIST", "RGBNDIST", "RGBNLDIST"
};

uint8_t picture[65536];
uint8_t picturePC[65536*3];

Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp)
    {
        case 1:
            return *p;
            break;

        case 2:
            return *(Uint16 *)p;
            break;

        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;
        break;

        case 4:
            return *(Uint32 *)p;
            break;

        default:
            return 0;       /* shouldn't happen, but avoids warnings */
    }
}

int main(int argc, char* argv[]) {
    int method=0;
    if(argc<3 || argc>4){
    	printf("Usage: png2scr8raw <pngfilename> <raw filename> [METHOD]\n");
        printf("\nWhere METHOD indicates how to convert a true color picture: DIVISION (default), RGBDIST, RGBLDIST, RGBNDIST, RGBNLDIST\n");
    	return 1;
    }
    if(argc==4){
        method=-1;
        for(int i=0; i<MAXMETHODS;i++)
            if(strcmp(argv[3], methods[i])==0){
                method=i;
                break;
            }
        if(method==-1){
            printf("Invalid method %s. Please use one of the followinf: DIVISION, RGBDIST, RGBLDIST, RGBNDIST, RGBNLDIST", argv[3]);
            return(1);
        }
    }
    // Prepares the palette data
    for(int color=0;color<256;color++){
        palette[color].g = screen8Pal[color][1]; //(color&0b11100000) >> 5;
        palette[color].r = screen8Pal[color][0]; //(color&0b00011100) >> 2;
        palette[color].b = screen8Pal[color][2]; //(color&0b00000011);
        palette[color].gn = palette[color].g * 0.7152;
        palette[color].rn = palette[color].r * 0.2126;
        palette[color].bn = palette[color].b * 0.0722;
        palette[color].light=palette[color].rn + palette[color].gn + palette[color].bn;
    }
    // Initialize SDL and SDL_image
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS )){
        printf("Failed to init video library.\n");
        return(1);
    }
    IMG_Init(IMG_INIT_PNG); // Initialize for PNG and JPG support

    // Load an image
    SDL_Surface* msx = IMG_Load(argv[1]);
    if (!msx) {
        printf("Can't load the file\n");
        return 1;
    }
    if(msx->w != 256){
        printf("PNG image needs to be 256 pixels wide.\n");
        return 1;
    }
    if(msx->h > 256){
        printf("PNG image height is larger than 256 (single screen 8 page).\n");
        return 1;
    }
    printf("File %s - Width: %d, Height: %d\n",argv[1], msx->w, msx->h);
    // Checks if the picture is and indexed color format
    if(SDL_ISPIXELFORMAT_INDEXED(msx->format->format)){
        if(msx->format->palette->ncolors!=256){
            printf("The PNG doesn't have 256 indexed colors. Colors: %d\n", msx->format->palette->ncolors);
            return 1;
        } else{
            printf("PNG has 256 indexed colors. Colors will be converted into a 1:1 relation between color index and MSX2 Screen 8 equivalent palette -  picture NEEDS to use the equivalent MSX2 Screen 8 palette in the 8 bits RGB space.\n");
            // Converts the pixels
            for(int i=0;i<msx->h;i++)
                for(int j=0;j<msx->w;j++)
                    picture[i*256+j]=((uint8_t*)msx->pixels)[i*msx->w+j];
        }
    }
    // The pixel format is an RGB format (likely true color)
    else{
        printf("PNG is true color. Pitch is %d\n", msx->pitch);
        uint8_t r,g,b;
        Uint32 data;
        for(int i=0;i<msx->h;i++){
            for(int j=0;j<msx->w;j++){
                data = getpixel(msx, j, i);
                SDL_GetRGB(data, msx->format, &r, &g, &b);
                switch(method){
                    // DIVISION
                    case 0:
                    {
                        picture[i*msx->w+j]=(g/32) << 5 | (r/32) << 2 | b/64;
                        break;
                    }
                    //RGBDIST
                    case 1:{
                        double distance;
                        //double minimum=sqrt(pow(r-screen8Pal[0][0],2)+pow(g-screen8Pal[0][1],2)+pow(b-screen8Pal[0][2],2) );
                        double minimum=sqrt(pow(r-palette[0].r,2)+pow(g-palette[0].g,2)+pow(b-palette[0].b,2) );
                        int index=0;
                        for(int i=1;i<256;i++){
                            distance=sqrt(pow(r-palette[i].r,2)+pow(g-palette[i].g,2)+pow(b-palette[i].b,2) );
                            if(distance < minimum){
                                minimum=distance;
                                index=i;
                            }
                        }
                        picture[i*msx->w+j]=index;
                        break;
                    }
                    case 2:{
                        double distance;
                        //double minimum=sqrt(pow(r-screen8Pal[0][0],2)+pow(g-screen8Pal[0][1],2)+pow(b-screen8Pal[0][2],2) );
                        double minimum=sqrt(pow(r-palette[0].r,2)+pow(g-palette[0].g,2)+pow(b-palette[0].b,2) + pow(g*0.7152+r*0.2126+b*0.0722 -palette[0].light,2));
                        int index=0;
                        for(int i=1;i<256;i++){
                            distance=sqrt(pow(r-palette[i].r,2)+pow(g-palette[i].g,2)+pow(b-palette[i].b,2) + pow(g*0.7152+r*0.2126+b*0.0722 -palette[i].light,2));
                            if(distance < minimum){
                                minimum=distance;
                                index=i;
                            }
                        }
                        picture[i*msx->w+j]=index;
                        break;
                    }
                    case 3:{
                        double distance;
                        //double minimum=sqrt(pow(r-screen8Pal[0][0],2)+pow(g-screen8Pal[0][1],2)+pow(b-screen8Pal[0][2],2) );
                        double minimum=sqrt(pow(r*0.2126-palette[0].rn,2)+pow(g*0.7152-palette[0].gn,2)+pow(b*0.0722-palette[0].bn,2));// + pow(g*0.7152+r*0.2126+b*0.0722 -palette[0].light,2));
                        int index=0;
                        for(int i=1;i<256;i++){
                            distance=sqrt(pow(r*0.2126-palette[i].rn,2)+pow(g*0.7152-palette[i].gn,2)+pow(b*0.0722-palette[i].bn,2));// + pow(g*0.7152+r*0.2126+b*0.0722 -palette[0].light,2));
                            if(distance < minimum){
                                minimum=distance;
                                index=i;
                            }
                        }
                        picture[i*msx->w+j]=index;
                        break;
                    }
                    case 4:
                    default:{
                        double distance;
                        //double minimum=sqrt(pow(r-screen8Pal[0][0],2)+pow(g-screen8Pal[0][1],2)+pow(b-screen8Pal[0][2],2) );
                        double minimum=sqrt(pow(r*0.2126-palette[0].rn,2)+pow(g*0.7152-palette[0].gn,2)+pow(b*0.0722-palette[0].bn,2) + pow(g*0.7152+r*0.2126+b*0.0722 -palette[0].light,2));
                        int index=0;
                        for(int i=1;i<256;i++){
                            distance=sqrt(pow(r*0.2126-palette[i].rn,2)+pow(g*0.7152-palette[i].gn,2)+pow(b*0.0722-palette[i].bn,2) + pow(g*0.7152+r*0.2126+b*0.0722 -palette[i].light,2));
                            if(distance < minimum){
                                minimum=distance;
                                index=i;
                            }
                        }
                        picture[i*msx->w+j]=index;
                        break;
                    }



                }
            }
        }
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
    SDL_Window *window = SDL_CreateWindow("Converted Screen 8 Picture", SDL_WINDOWPOS_UNDEFINED,  SDL_WINDOWPOS_UNDEFINED, msx->w*ZOOM, msx->h*ZOOM, 0);
    if(!window){
        printf("Can't create a window.\n");
        return(1);
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);

    SDL_RenderSetLogicalSize(renderer, msx->w*ZOOM, msx->h*ZOOM);

    SDL_Texture *alvo = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, msx->w, msx->h);



    //SDL_Texture *texture=SDL_CreateTextureFromSurface(renderer, msx);
    SDL_Rect clip, destino;
    // O próximo desenho vai ser na textura alvo
    SDL_SetRenderTarget(renderer, alvo);
    clip.x = 0;
    clip.y= 0;
    clip.w = msx->w;
    clip.h = msx->h;

    destino.x=0;
    destino.y=0;
    destino.w=msx->w;
    destino.h=msx->h;
 //   SDL_RenderCopy(renderer, texture, &clip, &destino);
    // Converts the picture back to a PC format to display the transformed picture
    for(int i=0;i<msx->h;i++)
        for(int j=0;j<msx->w;j++){
            SDL_SetRenderDrawColor(renderer, screen8Pal[picture[i*msx->w+j]][0], screen8Pal[picture[i*msx->w+j]][1], screen8Pal[picture[i*msx->w+j]][2], 255);
            SDL_RenderDrawPoint(renderer, j, i);
        }
        // Shows the converted picture

    SDL_SetRenderTarget(renderer, 0);
    SDL_RenderClear(renderer);
    // Desenha o alvo na tela
    SDL_RenderCopy(renderer, alvo, 0, 0);

    SDL_RenderPresent(renderer);
    SDL_SetRenderTarget(renderer, alvo);
    /*SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(picturePC, msx->w, msx->h, 24, msx->w*3, SDL_PIXELFORMAT_RGB888);
    if(!surface){
        printf("Can't create a surface.\n");
        return(1);
    }*/

    // Event loop to keep the window open
    SDL_Event e;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }
    }

   // Clean up
   //SDL_FreeSurface(surface);
   SDL_DestroyWindow(window);
    SDL_FreeSurface(msx);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
