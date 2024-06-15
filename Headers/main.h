#include "huffman.h"


#ifdef _WIN32
    #include <windows.h>

    void terminal_clear() {
        system("cls");
    }

#elif __linux__
    #include <unistd.h>

    void terminal_clear() {
        system("clear");
    }
#endif


void resetColor(){
    printf("\033[0m");
}

void printHelp(char *argv[]){
    printf("\n");
    printf("                %s\n\n", "\033[1;35mWelcome to the File Compression");
    printf("                           %s\n", "\033[0;30musing");
    printf("                       %s\n\n\n", "\033[0;36mHuffman coding");
    resetColor();
    printf("Usage: %s <input-file>  <operation>\n", argv[0]);
    printf("    %s\n", "<filename>.txt for <input-file> in compression");
    printf("    %s\n", "<filename>.txt.huffman for <input-file> in decompression");
    printf("    %s        %s\n", "-c", "for compress file");
    printf("    %s        %s\n", "-d", "for decompress file");
} 