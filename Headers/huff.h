#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXBUFFERSIZE 256

int readHeader(FILE *f);
int writeHeader(FILE *f);
int readBit(FILE *f);
int writeBit(FILE *f, int bit);
int flushBuffer(FILE *f);
void decodeBitInStream(FILE *fin, FILE *fout);
int decode(const char* ifile, const char *ofile);
void encodeAlpha(FILE *fout, int character);
int encode(const char* ifile, const char *ofile);
void buildTree();
void addLeaves();
int addNode(int index, int weight);
void finalise();
void init();

int chars = 256;
int unique = 0;
int *frequency = NULL;
unsigned int all = 0;

typedef struct {
    int index;
    unsigned int weight;
} node_t;

node_t *nodes = NULL;
int nodeCount = 0;
int *leafIndex = NULL;
int *parentIndex = NULL;

int free_index = 1;

int *stack;
int stack_top;

unsigned char buffer[MAXBUFFERSIZE];
int bits_in_buffer = 0;
int current_bit = 0;

int eof_input = 0;