#include <conio.h>
#include <dos.h>
#include <stdio.h>


int
main() {
    // gotoxy de conio y luego ir modificando el rango que ves
    char str[150], path[100][150], executable[100][150], name[100][150];
    char ignore[100];
    int i=0,j=0,value[100];
    FILE* fp;
    fp = fopen("LIST.TXT", "r");
    while (fgets(str, 150, fp)) {
        /** if you want to split value and string*/
        sscanf(str,"%s\t%s\t%*s\t%*s\t%s", &path[j], &executable[j], &name[j]);
        printf("path: %s \n", path[j]);
        printf("executable: %s \n", executable[j]);
        printf("name: %s \n", name[j]);
        j++;
    }
    fclose(fp);
    return 0;
}


