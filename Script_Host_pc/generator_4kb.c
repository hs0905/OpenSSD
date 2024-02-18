#include <stdio.h>
#include <stdlib.h>

int main(void){
    FILE *fp;
    for(int i = 0; i < 1024; i++){
        char temp[100];
        int n = i;
        sprintf(temp, "write_4kb_file/%d.bin", i);
        fp = fopen(temp, "wb");
        for(int j = 0; j < 1024; j++){
            fwrite(&n, sizeof(int), 1, fp);
        }
        fclose(fp);
    }

    return 0;
}