// VIDEO.C

void paleta(unsigned char pal){
  switch(pal){

	case 0: {
		asm{
			cli
			// bit 4 del colour control register a 1, paleta de alta intensidad
			// bit 5 del CCR a 1, paleta Magenta
			mov  dx,03D9h
			mov  al,48
			out  dx,al
			sti
		}
		break;
	  }
	case 1: {
		asm{
			cli
			// bit 4 del colour control register a 1, paleta de alta intensidad
			// bit 5 del CCR a 0, paleta Amarilla
			mov  dx,03D9h
			mov  al,16
			out  dx,al
			sti
		}
	}
	break;
 }

}

void modo_cga(int modo){
  switch (modo){
	case 160: asm{
		   // forzamos el modo 03h, 80x25 cga color
		mov ax,0003h
		int 10h

		// desactivamos el video mientras reprogramamos el CRTC
		mov  dx,03D8h
		mov  al,1
		out  dx,al

		// desactivamos las interrupciones
		cli

		// y las NMI
		in   al,70h
		and  al,7Fh
		out  70h,al

		// reprogramamos el CRTC
		mov  dx,03D4h   // El puerto

		mov  ax,7F04h  // Cuidadin con el byte menos significativo ;)
		out  dx,ax

		mov  ax,6406h
		out  dx,ax

		mov  ax,7007h
		out  dx,ax

		mov  ax,0109h  // Tama€o de fuente 2 pixeles de alto
		out  dx,ax

		//       Ahora llenamos la pantalla con caracteres DDh
		//       ya que en este modo de video lo unico que tocamos es el byte de color

		mov  ax,0B800h
		mov  es,ax
		xor  di,di
		mov  ax,00DDh
		mov  cx,1F40h
		rep  stosw

		// activamos NMI
		in   al,70h
		or   al,80h
		out  70h,al

		// y las interrupciones
		sti

		// y activamos el video
		mov  dx,3D8h
		mov  al,9
		out  dx,al

		}
	  break;
	  case 320: asm{
		mov ax,0x0004
		int 10h
	  }
	  break;
	}
}

void putpixel_160(unsigned char x, unsigned char y,unsigned char c){
  asm{
	mov  ax,0xB800                  // OPTIMIZAR
	mov  es,ax

	xor  al,al
	mov  ah,y              // Y

	cmp ah,0
	jb nopintar

	cmp ah,99
	ja nopintar

	shr  ax,1
	mov  di,ax
	shr  ax,1
	shr  ax,1
	add  di,ax

	xor  dh,dh
	mov  dl,x              // X

	cmp  dl,0
	jb nopintar

	cmp dl,159
	ja nopintar

	add  di,dx
	or   di,1

	mov  al,c               // Color
	mov  bl,es:[di]

	and  dx,1
	jnz  xOdd

	and  bl,0xF0
	jmp  xDone
  }
xOdd:
  asm{
	and  bl,0xF
	mov  cl,4
	shl  al,cl
  }
xDone:
  asm{
	or   al,bl
	mov  es:[di],al
  }
nopintar:

}

char getpixel_160 (unsigned char x, unsigned char y){
  unsigned char pixel;
  asm{
	mov ax,0xB800                   // OPTIMIZAR
	mov es,ax

	xor al,al
	mov ah,y
	shr ax,1
	mov di,ax
	shr ax,1
	shr ax,1
	add di,ax

	xor dh,dh
	mov dl,x
	add di,dx
	or di,1

	mov al,es:[di]

	and dx,0x01
	jz xEven

	mov cl,4
	shr al,cl


  }
xEven:
  asm{
	and  al,0x0F
	mov  pixel,al
  }
  return (pixel);

}



void modo_texto(){
  asm{
	push ax
	mov ah,00h
	mov al,03h
	int 10h
	pop ax
  }
}

void putpixel_320 (unsigned int x, unsigned int y, unsigned char c){
  asm {
	mov  ax,0xB800                  // OPTIMIZAR
	mov  es,ax
							// Rehacer con lookup table
	mov  ax,y
	xor  bl,bl
	mov  bh,al
	and  bh,0x01
	shl  bh,1
	add  bh,0xB8
	mov  es,bx
	and  al,0xFE
	shl  ax,1
	shl  ax,1
	shl  ax,1
	mov  di,ax
	shl  ax,1
	shl  ax,1
	add  di,ax
	mov  ax,x
	mov  cl,al
	not  cl
	and  cl,0x03
	shl  cl,1
	shr  ax,1
	shr  ax,1
	add  di,ax
	mov  al,0xFC
	rol  al,cl
	and  al,es:[di]
	mov  bh,c
	shl  bh,cl
	or   al,bh
	mov  es:[di],al
  }
}

void line_320(int startx, int starty, int endx, int endy, int color) {
	register int t,xerr=0, yerr=0;
	int delta_x, delta_y, distance;
	int incx, incy;

	/* compute the distances in both directions */
	delta_x=endx-startx;
	delta_y=endy-starty;

	/* Compute the direction of the increment,
	  an increment of 0 means either a horizontal or vertical
	  line.
	*/
	if(delta_x>0) incx=1;
	else if(delta_x==0) incx=0;
	else incx=-1;

	if(delta_y>0) incy=1;
	else if(delta_y==0) incy=0;
	else incy=-1;

	/* determine which distance is greater */
	delta_x=abs(delta_x);
	delta_y=abs(delta_y);
	if(delta_x>delta_y) distance=delta_x;
	else distance=delta_y;

	/* draw the line */
	for(t=0; t<=distance+1; t++) {
		putpixel_320(startx, starty, color);

		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance) {
			xerr-=distance;
			startx+=incx;
		}
		if(yerr>distance) {
			yerr-=distance;
			starty+=incy;
		}
	}
}

/*
void line_320_asm(int startx, int starty, int endx, int endy, int color){
  asm{
	push bp
	mov bp,sp
	push si
	push di
	push es

	mov ax,0xb800
	mov es,ax

	// Calcula la pendiente de la linea

	mov di,1
	mov dx,endy
	mov ax,starty
	sub dx,ax
	jge salvar_y
	neg di
	neg dx
  }

salvar_y:

  asm{
	push di
	mov cx,endx
	mov ax,startx
	mov si,1
	sub cx,ax
	jge salvar_x
	neg si
	neg cx
  }

salvar_x:

  asm{
	push si
	cmp cx,dx
	jge diag_c
	mov si,0
	xchg cx,dx
	jmp salvar_delsxy
  }

diag_c:

  asm{
	mov di,0
  }

salvar_delsxy:

  asm{
	push cx
	push dx
	push si
	push di

	mov ax,[bp-14]
	sal ax,1
	push ax
	sub ax,cx
	mov bx,ax
	sub ax,cx
	push ax
	inc cx
	mov dx,startx
	mov ax,00000011b
	push ax
	mov ax,00001100b
	push ax
	mov ax,11000000b
	push ax
	mov ax,starty
  }
	// Bucle para dibujar los puntos sobre la linea
buclelinea:

  asm{
	jmp dibujar
  }
vueltadibujar:
  asm{
	cmp bx,0
	jge diagonal
  }

recta:
  asm{
	add dx,[bp-16]
	add ax,[bp-18]
	add bx,[bp-20]
	loop buclelinea
	jmp finlinea
  }

diagonal:
  asm{
	add dx,[bp-10]
	add ax,[bp-8]
	add bx,[bp-22]
	loop buclelinea
	jmp finlinea
  }

dibujar:
  asm{
	push ax
	push dx
	push cx
	shr al,1
	jc impar
	mov di,0
	jmp comun
  }
impar:
  asm{
	mov di,2000h
  }
comun:
  asm{
	mov cx,80
	push dx
	mul cx
	add di,ax
	pop dx
	mov si,dx
	shr dx,1
	shr dx,1
	add di,dx
	and si,03h
	shl si,1
	mov ax,[bp-24+si]
	mov dh,[bp+12]
	and dh,al
	not al
	mov ah,es:[di]
	and ah,al
	or ah,dh
	mov es:[di],ah

	pop cx
	pop dx
	pop ax
	jmp vueltadibujar
  }


finlinea:

  asm{
	mov ax,bp
	mov sp,ax
  }





} */
