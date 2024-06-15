#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Headers/main.h"

int main(int argc, char **argv) {
    
    terminal_clear();

    // Ensure proper usage
    if (argc != 3){
        if (argc != 2){
            printf("%s\n", "\033[0;31mInvalid Usage.");
            resetColor();
            printf("Check <%s -h> for more detail.\n", argv[0]);
            return 1;
        }
        else if (argc == 2){
            if (strcmp(argv[1], "-h") == 0){
                printHelp(&argv[0]);
                return 0;
            }
            else {
                printf("%s\n", "\033[0;31mInvalid Usage.");
                resetColor();
                printf("Check <%s -h> for more detail.\n", argv[0]);
                return 2;
            }
        }
    }
    // remember filename and operation
    char in[50], op[3];
    strcpy(in, "Samples/");
    strcat(in, argv[1]);
    strcpy(op, argv[2]);

    init();

    if (strcmp(op, "-c") == 0) {
        char out[50];
        strcpy(out, "Samples/");
        strcat(out, argv[1]);
        *strstr(out, ".txt") = '\0';
        strcat(out, ".compressed.txt");
        encode(in, out);
    }
    else if (strcmp(op, "-d") == 0){
        char out[50];
        strcpy(out, "Samples/");
        strcat(out, argv[1]);
        *strstr(out, ".compressed.txt") = '\0';
        strcat(out, ".decompressed.txt");
        decode(in, out);
    }
    else
        printHelp(&argv[0]);
    finalise();
    resetColor();

    return 0;
}
