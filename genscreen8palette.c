#include <stdio.h>
#include <stdint.h>

uint8_t palcolorsrg[8]={0, 39, 82, 115, 155, 186, 224, 255};
uint8_t palcolorsb[4]={0, 82, 155, 255};

void main(void){
    printf("GIMP Palette\nName: MSX2 Screen 8\n#\n");
    for(int color=0;color<256;color++){
        printf("%03d %03d %03d\n",
            palcolorsrg[((uint8_t)color & 0b00011100)>>2],
            palcolorsrg[((uint8_t)color & 0b11100000)>>5],
            palcolorsb[((uint8_t)color & 0b00000011)]
        );
    }
}
