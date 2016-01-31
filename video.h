// VIDEO.H

#include "video.c"

void modo_cga(int modo);
void modo_texto(void);
void putpixel_160(unsigned char x, unsigned char y,unsigned char c);
char getpixel_160 (unsigned char x, unsigned char y);
void putpixel_320 (unsigned int x, unsigned int y, unsigned char c);
void line_320(int startx, int starty, int endx, int endy, int color);
void line_320_asm(int startx, int starty, int endx, int endy, int color);
void paleta(unsigned char pal);

