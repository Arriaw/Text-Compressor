#include "huff.h"

// Reads characters from the input file and updates the frequency array.
void determineFreq(FILE *f) {
    int c;
    while ((c = fgetc(f)) != EOF) {
        ++frequency[c];
        ++all;
    }
    for (c = 0; c < chars; ++c)
        if (frequency[c] > 0)
            ++unique;
}

// Allocates memory for the frequency array and sets up pointers for leaf and parent indices.
void init() {
    frequency = (int *) calloc(2 * chars, sizeof(int));
    leafIndex = frequency + chars - 1;
}

void allocateTree() {
    nodes = (node_t *) calloc(2 * unique, sizeof(node_t));
    parentIndex = (int *) calloc(unique, sizeof(int));
}

void finalise() {
    free(parentIndex);
    free(frequency);
    free(nodes);
}

int addNode(int index, int weight) {

    int i = nodeCount++;
    
    // Performs an insertion sort by shifting nodes to the right until the correct position for the new node is found.
    while (i > 0 && nodes[i].weight > weight) {
        memcpy(&nodes[i + 1], &nodes[i], sizeof(node_t)); // Move nodes to the right to make space for the new nodes
        if (nodes[i].index < 0) // If its a leaf
            ++leafIndex[-nodes[i].index];
        else
            ++parentIndex[nodes[i].index];
        --i;
    }

    // Making new node
    ++i;
    nodes[i].index = index;
    nodes[i].weight = weight;
    if (index < 0)
        leafIndex[-index] = i;
    else
        parentIndex[index] = i;

    return i; // Index of the adding node
}

void addLeaves() {
    int i, freq;
    for (i = 0; i < chars; ++i) {
        freq = frequency[i];
        if (freq > 0)
            addNode(-(i + 1), freq);
    }
}

void buildTree() {
    int left, right, index; // These indices point to nodes with the smallest weights.
    while (free_index < nodeCount) {
        left = free_index++;
        right = free_index++;
        index = addNode(right / 2, nodes[left].weight + nodes[right].weight);
        parentIndex[right / 2] = index;
    }
}


int encode(const char* infile, const char *outfile) {
    FILE *fin, *fout;
    if ((fin = fopen(infile, "rb")) == NULL) {
        printf("%s\n", "    \033[0;31mFile Failed to Compress");
        return -1;
    }
    if ((fout = fopen(outfile, "wb")) == NULL) {
        printf("%s\n", "    \033[0;31mFile Failed to Compress");
        fclose(fin);
        return -1;
    }

    determineFreq(fin);
    stack = (int *) calloc(unique - 1, sizeof(int));
    allocateTree();

    addLeaves();
    writeHeader(fout);
    buildTree();
    fseek(fin, 0, SEEK_SET);
    int c;
    while ((c = fgetc(fin)) != EOF)
        encodeAlpha(fout, c);
    flushBuffer(fout);
    free(stack);
    fclose(fin);
    fclose(fout);

    printf("%s\n", "    \033[0;32mFile Compressed Successfully.");
    
    return 0;
}

void encodeAlpha(FILE *fout, int character) {
    int node_index;
    stack_top = 0;
    node_index = leafIndex[character + 1];

    while (node_index < nodeCount) { // It stores the bit (0 or 1) in the stack array and moves to the parent node until reach root
        stack[stack_top++] = node_index % 2;
        node_index = parentIndex[(node_index + 1) / 2];
    }
    
    // Write Encoded Bits to Output File
    while (--stack_top > -1)
        writeBit(fout, stack[stack_top]);
}
        
int decode(const char* ifile, const char *ofile) {
    FILE *fin, *fout;
    if ((fin = fopen(ifile, "rb")) == NULL) {
        printf("    \033[0;31mFailed to decompress the file.\n");
        return -1;
    }
    if ((fout = fopen(ofile, "wb")) == NULL) {
        printf("    \033[0;31mFailed to decompress the file.\n");
        fclose(fin);
        return -1;
    }

    if (readHeader(fin) == 0) {
        buildTree();
        decodeBitInStream(fin, fout);
    }
    fclose(fin);
    fclose(fout);

    printf("%s\n", "    \033[0;32mFile Decompressed Successfully.");

    return 0;
}

void decodeBitInStream(FILE *fin, FILE *fout) {
    int i = 0, bit, node_index = nodes[nodeCount].index;
    while (1) {
        bit = readBit(fin);
        if (bit == -1)
            break;
        node_index = nodes[node_index * 2 - bit].index;
        if (node_index < 0) {
            char c = -node_index - 1;
            fwrite(&c, 1, 1, fout);
            if (++i == all)
                break;
            node_index = nodes[nodeCount].index;
        }
    }
}

int writeBit(FILE *f, int bit) {
    if (bits_in_buffer == MAXBUFFERSIZE << 3) { // if buffer is full
        size_t bytes_written = fwrite(buffer, 1, MAXBUFFERSIZE, f); // writes the entire buffer to the output file
        if (bytes_written < MAXBUFFERSIZE && ferror(f))
            return -2;
        bits_in_buffer = 0;
        memset(buffer, 0, MAXBUFFERSIZE); // clearing buffer
    }
    if (bit)
        buffer[bits_in_buffer >> 3] |= (0x1 << (7 - bits_in_buffer % 8));
    ++bits_in_buffer;

    return 0;
}

// Ensures that any remaining bits in the buffer are written to the output file 
int flushBuffer(FILE *f) {
    if (bits_in_buffer) { // if any remaining bit in buffer
        size_t bytes_written = fwrite(buffer, 1, (bits_in_buffer + 7) >> 3, f);
        if (bytes_written < MAXBUFFERSIZE && ferror(f))
            return -1;
        bits_in_buffer = 0;
    }
    return 0;
}

int readBit(FILE *f) {
    if (current_bit == bits_in_buffer) {
        if (eof_input)
            return -3;
        else {
            size_t bytes_read =
                fread(buffer, 1, MAXBUFFERSIZE, f);
            if (bytes_read < MAXBUFFERSIZE) {
                if (feof(f))
                    eof_input = 1;
            }
            bits_in_buffer = bytes_read << 3;
            current_bit = 0;
        }
    }

    if (bits_in_buffer == 0)
        return -3;
    int bit = (buffer[current_bit >> 3] >> (7 - current_bit % 8)) & 0x1;
    ++current_bit;
    return bit;
}

int writeHeader(FILE *f) {
    int i, j, byte = 0, size = sizeof(unsigned int) + 1 + unique * (1 + sizeof(int));
    unsigned int weight;

    char *buffer = (char *) calloc(size, 1);
    if (buffer == NULL)
        return -4;

    j = sizeof(int);
    while (j--)
        buffer[byte++] = (all >> (j << 3)) & 0xff; // Write an integer value to a byte array 

    buffer[byte++] = (char) unique;

    // Writes information for each active character to the header
    for (i = 1; i <= unique; ++i) {
        weight = nodes[i].weight;
        buffer[byte++] = (char) (-nodes[i].index - 1);
        j = sizeof(int);
        while (j--)
            buffer[byte++] = (weight >> (j << 3)) & 0xff;
    }
    fwrite(buffer, 1, size, f);
    free(buffer);
    return 0;
}

int readHeader(FILE *f) {
    int i, j, byte = 0, size;
    size_t bytes_read;
    unsigned char buff[4];

    // Read the original size of the file from the header
    bytes_read = fread(&buff, 1, sizeof(int), f);
    if (bytes_read < 1)
        return -3;
    
    // Decode original size
    byte = 0;
    all = buff[byte++];
    while (byte < sizeof(int))
        all = (all << (1 << 3)) | buff[byte++];

    // Read the number of active characters from the header
    bytes_read = fread(&unique, 1, 1, f);
    if (bytes_read < 1)
        return -3;

    allocateTree();

    size = unique * (1 + sizeof(int));
    unsigned int weight;
    char *buffer = (char *) calloc(size, 1);
    if (buffer == NULL)
        return -4;
    
    // Read information for each active character from the header
    fread(buffer, 1, size, f);
    byte = 0;
    for (i = 1; i <= unique; ++i) {
        nodes[i].index = -(buffer[byte++] + 1);
        j = 0;
        weight = (unsigned char) buffer[byte++];
        while (++j < sizeof(int)) 
            weight = (weight << (1 << 3)) | (unsigned char) buffer[byte++];
        
        nodes[i].weight = weight;
    }
    nodeCount = (int) unique;
    free(buffer);
    return 0;
}