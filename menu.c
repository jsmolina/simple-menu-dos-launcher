#include <stdio.h>

#define KEY_UP 0x4800
#define KEY_DOWN 0x5000
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define ESC 0x011B
#define ENTER 0x1C0D
#define MAX_ITEMS 100
#define MAX_PATH_DEPTH 75
#define MAX_EXECUTABLE_LENGTH 20 // more than enough for dos!
#define MAX_NAME_LENGTH 40 // 35 + 5
//10.482

unsigned char Color_S = 0x0F; 
unsigned char Color_N = 0x0F;
unsigned char Color_W = 0x70;
unsigned char Color_E = 0x0F;
unsigned char CARD = 0;
unsigned char check = 0;
unsigned char thumbnail[64] = {0};//Line
unsigned char start_dir_path[32] = {0};
unsigned char file_error[16] = {" FILE NOT FOUND "};
unsigned char image_error[16] = {" NO IMAGE FOUND "};
unsigned char title[] = {"LOADER MENU   FOR PCXT"};
unsigned char info[] = {"Select using cursors [ESC to exit, ENTER to run]"};
unsigned char message0[] = {"Executing "};
unsigned short TILE_MAP = 0xB800;
unsigned char MAP_RLE[] = {
	0x02,0x00,0x00,0x01,0xB0,0x08,0x01,0xDB,0x08,0x02,0xDB,0x07,0x44,0xDB,0x0F,0x02,
	0xDB,0x07,0x01,0xDB,0x08,0x01,0xB0,0x08,0x55,0x00,0x00,0x01,0xDB,0x1F,0x01,0xDF,
	0x1F,0x02,0xDF,0x1E,0x02,0xDF,
	0x1A,0x02,0xDF,0x13,0x02,0xDF,0x19,0x17,0xDF,0x18,0x01,0xDB,0x08,0x2E,0x00,0x00,
	0x01,0xDB,0x0E,0x20,0x00,0x1F,0x01,0xDB,0x08,0x2E,0x00,0x00,0x01,0xDB,0x1A,0x20,
	0x00,0x1F,0x01,0xDB,0x08,0x2E,0x00,0x00,0x01,0xDB,0x03,0x20,0x00,0x1F,0x01,0xDB,
	0x09,0x2E,0x00,0x00,0x01,0xDB,0x09,0x20,0x00,0x1F,0x01,0xDB,0x03,0x2E,0x00,0x00,
	0x01,0xDB,0x08,0x20,0x00,0x1F,0x01,0xDB,0x0A,0x2E,0x00,0x00,0x01,0xDB,0x08,0x20,
	0x00,0x1F,0x01,0xDB,0x0E,0x2E,0x00,0x00,0x01,0xDB,0x08,0x20,0x00,0x1F,0x01,0xDB,
	0x0E,0x2E,0x00,0x00,0x01,0xDB,0x08,0x20,0x00,0x1F,0x01,0xDB,0x0A,0x2E,0x00,0x00,
	0x01,0xDB,0x08,0x20,0x00,0x1F,0x01,0xDB,0x03,0x2E,0x00,0x00,0x01,0xDB,0x08,0x20,
	0x00,0x1F,0x01,0xDB,0x09,0x2E,0x00,0x00,0x01,0xDB,0x08,0x20,0x00,0x1F,0x01,0xDB,
	0x08,0x2E,0x00,0x00,0x01,0xDB,0x08,0x20,0x00,0x1F,0x01,0xDB,0x08,0x2E,0x00,0x00,
	0x01,0xDB,0x08,0x20,0x00,0x1F,0x01,0xDB,0x09,0x2E,0x00,0x00,0x01,0xDB,0x08,0x20,
	0x00,0x1F,0x01,0xDB,0x03,0x2E,0x00,0x00,0x01,0xDB,0x08,0x20,0x00,0x1F,0x01,0xDB,
	0x0A,0x2E,0x00,0x00,0x01,0xDB,0x09,0x20,0x00,0x1F,0x01,0xDB,0x0E,0x2E,0x00,0x00,
	0x01,0xDB,0x03,0x01,0xDC,0x13,0x02,0xDC,0x19,0x14,0xDC,0x18,0x02,0xDC,0x19,0x02,
	0xDC,0x13,0x02,0xDC,0x1A,0x02,0xDC,0x1E,0x01,0xDC,0x1F,0x01,0xDB,0x0F,0x7C,0x00,
	0x00,0x01,0xDF,0x78,0x4C,0xDF,0x08,0x01,0xDF,0x78,0x02,0x00,0x00,0x01,0xDB,0x0F,
	0x4C,0x00,0x0F,0x01,0xDB,0x0F,0x02,0x00,0x00,0x01,0xDB,0x0F,0x4C,0x00,0x0F,0x01,
	0xDB,0x0F,0x02,0x00,0x00,0x01,0xDC,0x78,0x4C,0xDC,0x08,0x01,0xDC,0x78,0x01,0x00,
	0x00,
};

char path[MAX_ITEMS][MAX_PATH_DEPTH], executable[MAX_ITEMS][MAX_EXECUTABLE_LENGTH], name[MAX_ITEMS][MAX_NAME_LENGTH];


//Functions

void Wait_1s(){
	asm {
		mov ah,0x86
		mov cx,0x0F
		mov dx,0x4240
		int 15h
	}
}

void ClearScreen(){
	asm{
	mov ah, 0x00
	mov al, 0x03  //text mode 80x25 16 colours
	int 0x10
	}
}

void Clearkb(){
	asm mov ah,00ch
	asm mov al,0
	asm int 21h
}

void Check_Graphics(){
	int i;
	asm mov ax,0x1A00
	asm mov bl,0x32
	asm int 0x10
	asm cmp al,0x1A
	asm jz CARD1;			//if (al == 0x1A) CADR = 1;
		asm mov ah,0x12		//else
		asm mov bx,0x1010
		asm int 0x10
		asm cmp bh,0
		asm jz CARD1;		//if (bh == 0) CADR = 1;
			asm mov ah,0x0F	//else
			asm mov bl,0
			asm int 0x10
			asm cmp al,0x07
			asm jnz CARD1;	//if (al != 0x07) CADR = 1;

	//else MDA
	Color_S = 0x70; Color_N = 0x10; Color_E = 0x10; TILE_MAP = 0xB000;
	asm jmp ENDCARD
	CARD1://CGA TANDY EGA VGA
	Color_S = 0x3F; Color_N = 0x1F; Color_E = 0x4F; TILE_MAP = 0xB800;
	ENDCARD:
}

int Load_List() {
	char str[150];
	int j=0;
	FILE* fp;
	fp = fopen("LIST.TXT", "r");
	while (fgets(str, 150, fp)) {
		// skip comments
		if(str[0] == '#') {
			continue;
		}
		sscanf(str,"%s\t%s\t%*s\t%*s\t%[^\n]", &path[j], &executable[j], &name[j]);
		j++;
	}
	fclose(fp);
	return j;
	
	// Read TXT
	// if char == 0x20 (#), skip line (read until 0x0D, 0x0A or \r\n)
	// else, read names: 
	//		0x09 = tab (separates fields)
}

void Draw_Menu(){
	unsigned short _offset = 0;
	asm mov ax,TILE_MAP
	asm mov es,ax
	asm xor di,di		//ES:DI = VRAM
	asm xor bx,bx
	_loop:
		//if ((CARD == 0) && ( (col&0x0F) == 8)) col = 0x10;
		asm mov cl,byte ptr MAP_RLE[bx  ]
		asm mov	ax,word ptr MAP_RLE[bx+1]
		_loop0:
			asm mov	word ptr es:[di],ax
			asm add di,2
			asm inc _offset
			asm dec cl
			asm jnz _loop0
			
		asm add bx,3
		asm cmp _offset,160*25
		asm jng _loop
	
	asm mov di,29*2
	asm mov cx,22
	asm mov si,offset title 
	asm mov	ah,Color_W
	_loop1:	
		asm lodsb			//load al from ds:[si], increment si
		asm stosw			//store ax to es:[di], increment di
		asm loop _loop1
	
	asm mov di,(22*160)+32
	asm mov cx,48
	asm mov si,offset info 
	asm mov	ah,0x0F
	_loop2:	
		asm lodsb			//load al from ds:[si], increment si
		asm stosw			//store ax to es:[di], increment di
		asm loop _loop2
}

void Update_List(unsigned short scroll, unsigned short selected){
	asm mov ax,TILE_MAP
	asm mov es,ax
	asm mov di,(3*160)+8	//First name position in screen
	asm mov si,offset name	//Names array
	asm mov ax,scroll		//Scroll position
	asm mov cx,40
	asm mul cx
	asm add si,ax
	asm mov bx,0
	
	_loopc:					//Write 16 names
		asm mov	ah,Color_N	//Color not selected
		asm cmp bx,selected
		asm jnz not_selected
		asm mov	ah,Color_S	//Color selected
		not_selected:
		asm mov cx,32
		_loopb:	
			asm lodsb
			asm stosw		//store ax to es:[di], increment di
			asm loop _loopb
		asm add si,8
		asm add di,96
		asm inc bx
		asm cmp bx,16
		asm jnz _loopc
}

//32x32 image
void Get_Image(unsigned char *filename){
	unsigned short handle = 0;
	unsigned char lines = 16;
	unsigned char *name = &filename[0];

	//This function opens a file
	asm mov ah,0x3D					//open file
	asm mov al,0					//Read only
	asm mov dx,name					//filename to open
	asm int 21h
	asm mov handle,ax
	
	//if handle == 0x02, file not found				
	asm cmp ax,0x02					
	asm jz _no_file
		//else read file
		//Point to video ram offset were the image is being displayed
		asm mov ax,TILE_MAP
		asm mov es,ax
		asm mov di,(3*160) + 84			//ES:DI = VRAM
		
		_loop2:
			//this reads 64 bytes and stores them in thumbnail
			asm mov ah,0x3F
			asm mov cx,64
			asm mov bx,handle
			asm mov dx,offset thumbnail
			asm int 21h
			
			//This moves 64 bytes from thumbnail (ds:si) to vram E(S:DI)
			asm mov si,offset thumbnail
			asm mov cx,32
			_loop3:
				asm lodsw			//lds:[si] => ax, increment si
				asm stosw			//ax => es:[di], increment di
				asm loop _loop3
			
			//Jump to next line in video ram
			asm add di,96
			asm dec lines
			asm jnz _loop2
		
		//Close file
		asm mov ah,0x3E					
		asm mov bx,handle				
		asm int 21h
		asm jmp _end_get_image
		
	_no_file:
		//print message at image position
		asm mov di,(11*160) + 94
		asm mov si,offset image_error
		asm mov	ah,Color_S
		asm mov cx,16
		_loop_line:	
			asm lodsb
			asm stosw
			asm loop _loop_line
	
	_end_get_image:
}


///MAIN FUNCTION
int main() {
	unsigned short error = 0;
	unsigned short programs;
	unsigned short scroll = 0;
	unsigned short selected = 0;
	unsigned short input = 1;
	getcwd(start_dir_path,32);
	ClearScreen();
	Check_Graphics();
	Draw_Menu();
	programs = Load_List();
	
	//MAIN LOOP
	_main_loop:
		if (input){
			Update_List(scroll, selected);
			Get_Image("menu_img.bin");
			input = 0;
			Clearkb();
		}
		_wait_again:
		//Wait for key and read
		asm mov ax, 00h
		asm int 16h
		asm mov input,ax //input = getch();
		
		//this is a switch case: switch(input)
		//case ESC
		asm cmp input,ESC
		asm jnz _end_input_ESC
			asm jmp _exit
		_end_input_ESC:
		
		//case KEY_DOWN
		asm cmp input,KEY_DOWN; asm jnz _end_input_KEY_DOWN
				//if(selected == 15)
				asm cmp selected,15; asm jnz _sel_not_15
					asm mov ax,scroll
					asm add ax,15		//scroll + 15
					asm mov bx,programs
					asm dec bx			//programs-1
					//if (scroll + 15 < programs-1)
					asm cmp ax,bx
					asm jnb _is_not_below		
						asm inc scroll	//scroll++;
					_is_not_below:; asm jmp _end_main_loop
				//else
				_sel_not_15:; asm inc selected	//selected++;
			asm jmp _end_main_loop
		_end_input_KEY_DOWN:
		
		//case == KEY_UP
		asm cmp input,KEY_UP; asm jnz _end_input_KEY_UP
				//if(selected == 0)
				asm cmp selected,0  
				asm jnz _sel_not_0
					//if(scroll > 0)
					asm cmp scroll,0
					asm jng _is_not_greater		
						asm dec scroll	//scroll--;
					_is_not_greater:
					asm jmp _end_main_loop
				//else
				_sel_not_0: 			
					asm dec selected	//selected--;
			asm jmp _end_main_loop
		_end_input_KEY_UP:
		
		//case == ENTER
		asm cmp input,ENTER; asm jnz _main_loop
				//print message at xy 3,22 color 0x0F
				asm mov di,(22*160)+6
				asm mov si,offset message0 
				asm mov	ah,0x0F
				_loop2:	
					asm lodsb
					asm or	al,al	//if al == 0
					asm jz	end_of_string
					asm stosw
					asm jmp _loop2
				end_of_string:
				
				//print executable[selected + scroll) at xy 13,22 color 0x0F
				asm mov di,(22*160)+26
				asm mov si,offset executable
				asm mov ax,selected
				asm add ax,scroll
				asm mov cx,20
				asm mul cx
				asm add si,ax
				asm mov	ah,0x0F
				_loop3:	
					asm lodsb
					asm or	al,al	//if al == 0
					asm jz	end_of_string2
					asm stosw
					asm jmp _loop3
				end_of_string2:
				
				Wait_1s();
				
				//chdir(path[selected+scroll]);
				asm mov si,offset path
				asm mov ax,selected
				asm add ax,scroll
				asm mov cx,75
				asm mul cx
				asm add si,ax
				asm mov ah,0x3B
				asm mov dx,si	//ASCIIZ path name
				asm int 21h
				
				ClearScreen();
				//system(executable[selected + scroll]);
				//SS and SP should be preserved in code segment before call
					//since a bug in DOS version 2.x destroys these
				asm mov si,offset executable
				asm mov ax,selected
				asm add ax,scroll
				asm mov cx,20
				asm mul cx
				asm add si,ax
				asm mov ah,0x4B
				asm mov al,0x00 //Load and run program
				asm mov dx,si	//file path
				asm int 21h
				
				///Return from program
				asm cmp ax,0x02		//if error code == 2 (file not found)
				asm jnz _no_error
					//printf("File not found");
					asm mov di,(11*160) + 60
					asm mov si,offset file_error
					asm mov	ah,Color_S
					asm mov cx,16
					_loop_line:	
						asm lodsb
						asm stosw
						asm loop _loop_line
					Wait_1s();
				_no_error:
				ClearScreen();
				Clearkb();
				
				//chdir(start_dir_path);
				asm mov ah,0x3B
				asm mov dx,offset start_dir_path	//ASCIIZ path name
				asm int 21h
				
				Draw_Menu();
		_end_input_ENTER:
		
		//end of switch case
	
		// Loop again
		_end_main_loop:
		asm jmp _main_loop:
	// End program
	_exit:
	
	ClearScreen();
	return 0;
}
