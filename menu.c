////////////////////
//Tiny simple menu//
////////////////////

//FOR 8088-86

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
#define LIST_LINE_LENGTH 110	//Line length of list.txt

unsigned short clock_ticks = 0;
unsigned short key_input = 1;
unsigned short menu_scroll = 0;
unsigned short menu_selected = 0;
unsigned char exe_params[16] = {0};
unsigned char exec_cmd[16] = {0};
unsigned char exec_fcb[24] = {0};
unsigned char Color_S = 0x0F;
unsigned char Color_N = 0x0F;
unsigned char Color_W = 0x70;
unsigned char Color_E = 0x0F;

unsigned short programs = 0;
unsigned short file_handle = 0;
unsigned char read_buffer[110] = {0};//buffer to read thumbnail or list lines
unsigned short read_lines = 16;
unsigned short read_error = 0;

//Read list variables
unsigned char LIST_FILE[] = {"LIST.TXT"};
unsigned char *list_filename = &LIST_FILE[0];
unsigned char list_error[16] = {" NO LIST FOUND  "};
unsigned char path1[33] = {0};
unsigned char exec1[17] = {0};

//Read Thumbnail variables
unsigned char THUMB_FILENAME[20] = {"menu_img.bin"};
unsigned char *thumbnail_name = &THUMB_FILENAME[0];
unsigned char start_dir_path[32] = {0};
unsigned char file_error[16] = {" FILE NOT FOUND "};
unsigned char image_error[16] = {" NO IMAGE FOUND "};

//INFO
unsigned char title[] = {"LOADER MENU   FOR PCXT"};
unsigned char info[] = {"Select using cursors [ESC to exit, ENTER to run]"};
unsigned char message0[] = {"Executing "};
unsigned short TILE_MAP = 0xB800;

//Character map "compressed" in RLE format
unsigned short map_offset = 0;
unsigned char MAP_RLE[327] = {
	0x02,0x00,0x00,0x01,0xB0,0x08,0x01,0xDB,0x08,0x02,0xDB,0x07,0x44,0xDB,0x0F,0x02,
	0xDB,0x07,0x01,0xDB,0x08,0x01,0xB0,0x08,0x55,0x00,0x00,0x01,0xDB,0x1F,0x01,0xDF,
	0x1F,0x02,0xDF,0x1E,0x02,0xDF,0x1A,0x02,0xDF,0x13,0x02,0xDF,0x19,0x17,0xDF,0x18,
	0x01,0xDB,0x08,0x2E,0x00,0x00,0x01,0xDB,0x0E,0x20,0x00,0x1F,0x01,0xDB,0x08,0x2E,
	0x00,0x00,0x01,0xDB,0x1A,0x20,0x00,0x1F,0x01,0xDB,0x08,0x2E,0x00,0x00,0x01,0xDB,
	0x03,0x20,0x00,0x1F,0x01,0xDB,0x09,0x2E,0x00,0x00,0x01,0xDB,0x09,0x20,0x00,0x1F,
	0x01,0xDB,0x03,0x2E,0x00,0x00,0x01,0xDB,0x08,0x20,0x00,0x1F,0x01,0xDB,0x0A,0x2E,
	0x00,0x00,0x01,0xDB,0x08,0x20,0x00,0x1F,0x01,0xDB,0x0E,0x2E,0x00,0x00,0x01,0xDB,
	0x08,0x20,0x00,0x1F,0x01,0xDB,0x0E,0x2E,0x00,0x00,0x01,0xDB,0x08,0x20,0x00,0x1F,
	0x01,0xDB,0x0A,0x2E,0x00,0x00,0x01,0xDB,0x08,0x20,0x00,0x1F,0x01,0xDB,0x03,0x2E,
	0x00,0x00,0x01,0xDB,0x08,0x20,0x00,0x1F,0x01,0xDB,0x09,0x2E,0x00,0x00,0x01,0xDB,
	0x08,0x20,0x00,0x1F,0x01,0xDB,0x08,0x2E,0x00,0x00,0x01,0xDB,0x08,0x20,0x00,0x1F,
	0x01,0xDB,0x08,0x2E,0x00,0x00,0x01,0xDB,0x08,0x20,0x00,0x1F,0x01,0xDB,0x09,0x2E,
	0x00,0x00,0x01,0xDB,0x08,0x20,0x00,0x1F,0x01,0xDB,0x03,0x2E,0x00,0x00,0x01,0xDB,
	0x08,0x20,0x00,0x1F,0x01,0xDB,0x0A,0x2E,0x00,0x00,0x01,0xDB,0x09,0x20,0x00,0x1F,
	0x01,0xDB,0x0E,0x2E,0x00,0x00,0x01,0xDB,0x03,0x01,0xDC,0x13,0x02,0xDC,0x19,0x14,
	0xDC,0x18,0x02,0xDC,0x19,0x02,0xDC,0x13,0x02,0xDC,0x1A,0x02,0xDC,0x1E,0x01,0xDC,
	0x1F,0x01,0xDB,0x0F,0x7C,0x00,0x00,0x01,0xDF,0x78,0x4C,0xDF,0x08,0x01,0xDF,0x78,
	0x02,0x00,0x00,0x01,0xDB,0x0F,0x4C,0x00,0x0F,0x01,0xDB,0x0F,0x02,0x00,0x00,0x01,
	0xDB,0x0F,0x4C,0x00,0x0F,0x01,0xDB,0x0F,0x02,0x00,0x00,0x01,0xDC,0x78,0x4C,0xDC,
	0x08,0x01,0xDC,0x78,0x01,0x00,0x00,
};



/////////////
//Functions//
/////////////

void Wait_1s(){
	asm mov clock_ticks,0
	asm mov ah,1
	asm mov cx,0
	asm mov dx,0
	asm int 0x1A	//Reset clock

	_wait:
		asm mov ah,0
		asm int 0x1A	//Read clock
		asm cmp clock_ticks,15
		asm jg _stop
		asm mov clock_ticks,dx	//dx (clock ticks) to variable
		asm loop _wait
	_stop:
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
	asm{
		mov Color_S,0x70; mov Color_N,0x10;mov Color_E,0x10;mov TILE_MAP,0xB000;
		jmp ENDCARD
	}
	CARD1://CGA TANDY EGA VGA
	asm {
		mov Color_S,0x3F; mov Color_N,0x1F; mov Color_E,0x4F; mov TILE_MAP,0xB800;
	}
	ENDCARD:
}

void Count_programs(){
	// Read TXT
	asm mov ax,0x3D00				//open file, Read only
	asm mov dx,list_filename		//filename to open
	asm int 21h
	asm mov file_handle,ax

	asm cmp ax,0x02
	asm jz _no_list_file
		_loop_read:
			//this reads 110 bytes (one line) and stores them in buffer
			asm mov ah,0x3F
			asm mov cx,110
			asm mov bx,file_handle
			asm mov dx,offset read_buffer
			asm int 21h

			asm cmp ax,110	//If line != 110, end of file
			asm jnz _end_read
			asm inc programs
			asm jmp _loop_read

		_end_read:
		//Close file
		asm mov ah,0x3E
		asm mov bx,file_handle
		asm int 21h
		asm jmp _end_count

	_no_list_file:
		//print message at list position
		asm mov di,(3*160)+8
		asm mov si,offset list_error
		asm mov	ah,Color_S
		asm mov cx,16
		_loop_line:
			asm lodsb
			asm stosw
			asm loop _loop_line

		Wait_1s();
		ClearScreen();
		asm mov ax,4C00h
		asm int 21h
	_end_count:
	asm dec programs

}

void Update_List() {
	// Read TXT
	//This function opens a file
	asm mov cx,0//a bug in c?
	asm mov read_lines,0

	asm mov ax,0x3D00				//open file, Read only
	asm mov dx,list_filename					//filename to open
	asm int 21h
	asm mov file_handle,ax

	//We now know there is a list.txt, read file
	asm mov ax,TILE_MAP
	asm mov es,ax
	asm mov di,(3*160)+8			//ES:DI = VRAM


	//Seek list.txt
	asm mov ax,menu_scroll
	asm inc ax
	asm mov bx,LIST_LINE_LENGTH
	asm mul bx			//ax = scroll*line_length
	asm mov dx,ax		//File position
	asm mov ax,4201h	//Move pointer from current location
	asm mov bx,file_handle
	asm int 21h			//

	//Write 16 names
	_loop_read:
		//this reads 110 bytes (one line) and stores them in buffer
		asm mov ah,0x3F
		asm mov cx,110
		asm mov bx,file_handle
		asm mov dx,offset read_buffer
		asm int 21h

		asm cmp ax,110	//If line != 110, end of file
		asm jnz _end_read

		//Check selected item
		asm mov	ah,Color_N	//Color not selected
		asm mov cx,menu_selected;
		asm cmp read_lines,cx; asm jnz not_selected //If selected
			asm mov si,offset read_buffer
			asm mov	ah,Color_S	//Color selected
			//Store path
			asm mov bx,0
			asm mov cx,16
			_loopA:
				asm lodsw			//ds:[si] => ax, increment si
				asm mov word ptr path1[bx],ax
				asm add bx,2
				asm loop _loopA
			//store exe
			asm inc si	//skip tab
			asm mov bx,0
			asm mov cx,8
			_loopB:
				asm lodsw			//ds:[si] => ax, increment si
				asm mov word ptr exec1[bx],ax
				asm add bx,2
				asm loop _loopB
		not_selected:
		//Write names to VRAM
		asm mov si,offset read_buffer+76
		//This moves 16 bytes from buffer (ds:si) to vram (ES:DI)
		asm mov cx,32
		_loop0:
			asm lodsb			//ds:[si] => al, increment si
			asm stosw			//ax => es:[di], increment di
			asm loop _loop0

		//Jump to next line in video ram
		asm add di,128-32
		asm inc read_lines
		asm cmp read_lines,16
		asm jnz _loop_read

	_end_read:
	//Close file
	asm mov ah,0x3E
	asm mov bx,file_handle
	asm int 21h
}

void Draw_Menu(){
	asm mov map_offset,0
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
			asm inc map_offset
			asm dec cl
			asm jnz _loop0

		asm add bx,3
		asm cmp map_offset,160*25
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

//32x32 image
void Get_Image(){
	asm mov read_lines,16
	//This function opens a file
	asm mov ax,0x3D00				//open file, read only
	asm mov dx,thumbnail_name					//filename to open
	asm int 21h
	asm mov file_handle,ax

	//if handle == 0x02, file not found
	asm cmp ax,0x02
	asm jz _no_file
		//else read file
		//Point to video ram offset were the image is being displayed
		asm mov ax,TILE_MAP
		asm mov es,ax
		asm mov di,(3*160) + 84			//ES:DI = VRAM

		_loop2:
			//this reads 64 bytes and stores them in read_buffer
			asm mov ah,0x3F
			asm mov cx,64
			asm mov bx,file_handle
			asm mov dx,offset read_buffer
			asm int 21h

			//This moves 64 bytes from read_buffer (ds:si) to vram E(S:DI)
			asm mov si,offset read_buffer
			asm mov cx,32
			_loop3:
				asm lodsw			//ds:[si] => ax, increment si
				asm stosw			//ax => es:[di], increment di
				asm loop _loop3

			//Jump to next line in video ram
			asm add di,96
			asm dec read_lines
			asm jnz _loop2

		//Close file
		asm mov ah,0x3E
		asm mov bx,file_handle
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
	//setblock(_segment,5*1024);

	asm mov si, offset exe_params
	asm mov word ptr [si+2],offset exec_cmd		//dword pointer to command line parameter string
	asm mov word ptr [si+4],ds
	asm mov word ptr [si+6],offset exec_fcb		//dword pointer to default FCB at address 5Ch (0)
	asm mov word ptr [si+8],ds
	asm mov word ptr [si+10],offset exec_fcb		//dword pointer to default FCB at address 6Ch (0)
	asm mov word ptr [si+12],ds

	//get drive
	asm mov ah,19h
	asm int 21h

	asm add al,65	//al = drive = A B C... (0 1 2 + 64)
	asm mov si,offset start_dir_path
	asm mov ds:[si],al
	asm inc si
	asm mov ax,0x5C3A	// ':\'
	asm mov ds:[si],ax		//store 'A B C..',':','\' at the start of path
	asm add si,2

	//getcwd(start_dir_path,32);
	asm mov ah,47h
	asm mov dl,0	//drive number (0 = default, 1 = A:)
	asm int 21h

	ClearScreen();
	Check_Graphics();
	Draw_Menu();
	Count_programs();

	//MAIN LOOP
	_main_loop:
		//if (input)
		asm cmp key_input,0; asm jng _no_input;
			Update_List();
			Get_Image();
			asm mov key_input,0
			Clearkb();
			//printf("SEG %04X ",_segment);
		_no_input:

		//Wait for key and read
		asm mov ax, 00h
		asm int 16h
		asm mov key_input,ax //key_input = getch();

		//this is a switch case: switch(key_input)
		//case ESC
		asm cmp key_input,ESC
		asm jnz _end_input_ESC
			asm jmp _exit
		_end_input_ESC:

		//case KEY_DOWN
		asm cmp key_input,KEY_DOWN; asm jnz _end_input_KEY_DOWN
			//if(menu_selected == 15)
			asm cmp menu_selected,15; asm jnz _sel_not_15
				asm mov ax,menu_scroll
				asm add ax,15		//menu_scroll + 15
				asm mov bx,programs
				asm dec bx			//programs-1
				//if (menu_scroll + 15 < programs-1)
				asm cmp ax,bx; asm jnb _is_not_below
					asm inc menu_scroll	//menu_scroll++;
				_is_not_below:; asm jmp _main_loop
				//else
				_sel_not_15:; asm inc menu_selected	//menu_selected++;
			asm jmp _main_loop
		_end_input_KEY_DOWN:

		//case == KEY_UP
		asm cmp key_input,KEY_UP; asm jnz _end_input_KEY_UP
			//if(menu_selected == 0)
			asm cmp menu_selected,0; asm jnz _sel_not_0
				//if(menu_scroll > 0)
				asm cmp menu_scroll,0; asm jng _is_not_greater
						asm dec menu_scroll	//menu_scroll--;
				_is_not_greater:; asm jmp _main_loop
				//else
				_sel_not_0:
					asm dec menu_selected	//menu_selected--;
			asm jmp _main_loop
		_end_input_KEY_UP:

		//case == ENTER
		asm cmp key_input,ENTER; asm jnz _main_loop
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

			//print executable[menu_selected + scroll) at xy 13,22 color 0x0F
			asm mov di,(22*160)+26
			asm mov si,offset exec1
			asm mov	ah,0x0F
			asm mov	cx,16
			_loop3:
				asm lodsb
				asm stosw
				asm loop _loop3
			end_of_string2:

			Wait_1s();

			//chdir(path[menu_selected+scroll]);
			asm mov si,offset path1
			asm mov ah,0x3B
			asm mov dx,si	//ASCIIZ path name
			asm int 21h

			ClearScreen();
			// system(executable[menu_selected + menu_scroll]);
			//SS and SP should be preserved in code segment before call
				//since a bug in DOS version 2.x destroys these
			asm mov si,offset exec1
			asm mov ah,0x4B
			asm mov al,0x00 //Load and run program
			asm mov dx,si	//file name
			asm mov bx, offset exe_params
			asm int 21h
			asm mov read_error,ax
			Wait_1s();
			///Return from program
			asm cmp read_error,0x02		//if error code == 2 (file not found)
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
		asm jmp _main_loop:
	// End program
	_exit:
	
	ClearScreen();
	return 0;
}
