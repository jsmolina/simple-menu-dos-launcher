#include <conio.h>
#include <dos.h>
#include <stdio.h>

#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define ESC 27
#define ENTER 13


char path[100][150], executable[100][150], name[100][150];

int load() {
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
}

int main() {
    // gotoxy de conio y luego ir modificando el rango que ves
    int programs;
    int i = 0, scroll = 0;
    int selected = 0;
    int exit = 0;
    char input;


    programs = load();
    while(!exit) {
        clrscr();
        gotoxy(1, selected + 5);
        printf("->");
        for (i = 0; i < 16; i++) {
            gotoxy(5, i + 5);
            if (i + scroll < programs) {
                printf(name[i + scroll]);
            }
        }
        printf("\nSelect using cursors [ESC to exit, ENTER to run]: ");
        input = getch();
        if (input == ESC) {
            exit = 1;
        } else if (input == KEY_DOWN) {

            if(selected == 15) {
                if (scroll + 15 < programs) {
                    scroll++;
                }
            } else {
                selected++;
            }
        } else if (input == KEY_UP) {
            if(selected == 0) {
                if(scroll > 0) {
                    scroll--;
                }
            } else {
                selected--;
            }
        } else if (input == ENTER) {
            printf("\nExecuting %s", executable[selected + scroll]);
            chdir(path[selected+scroll]);
            system(executable[selected + scroll]);
        }
    }

    return 0;
}


