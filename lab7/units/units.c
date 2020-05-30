#include <stdio.h>
#include <stdlib.h>

#include <libgen.h> // Prototype for basename() function

#define KB(i) ((i)*1<<10)
#define INT sizeof(int)
#define SHORT sizeof(short)
#define BYTE sizeof(char)
int unit_size = INT;

char* unit_to_format(int unit) {
    static char* formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
    return formats[unit_size-1];
    /* If the above is too confusing, this can also work:
    switch (unit_size) {
        case 1:
            return "%#hhx\n";
        case 2:
            return "%#hx\n";
        case 4:
            return "%#hhx\n";
        default:
            return "Unknown unit";
    }
    */
}  


/************ These are the functions that actually do the work ***************/
/* Reads units from file */
void read_units_to_memory(FILE* input, char* buffer, int count) {
    fread(buffer, unit_size, count, input);
}

/* Prints the buffer to screen by converting it to text with printf */
void print_units(FILE* output, char* buffer, int count) {
    char* end = buffer + unit_size*count;
    while (buffer < end) {
        //print ints
        int var = *((int*)(buffer));
        fprintf(output, unit_to_format(unit_size), var);
        buffer += unit_size;
    }
}

/* Writes buffer to file without converting it to text with write */
void write_units(FILE* output, char* buffer, int count) {
    fwrite(buffer, unit_size, count, output);
}
/*****************************************************************************/




int main(int argc, char* argv[]) {
    char buffer[KB(10)] = {};
    FILE* input = fopen("input", "r");   
    FILE* output = fopen("output", "w");
    if (!input || !output)
    {
        perror("Opening files");
        fprintf(stderr, "input = %p, output = %p\n", input, output);
        exit (-1);
    } 
    
    
    // Reading command line arguments
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == 'i' && argv[i][1] == '\0') { // ints
            unit_size = INT;
        }
        else if (argv[i][0] == 's' && argv[i][1] == '\0') {  // shorts
            unit_size = SHORT;
        }   
        else if (argv[i][0] == 'b' && argv[i][1] == '\0') {  // bytes 
            unit_size = BYTE;
        }
        else {
            fprintf(stderr, "%s: Unknown argument: %s\n", basename(argv[0]), argv[1]);
            fprintf(stderr, "Valid commands line arguments: \"i\", \"s\", or \"b\"\n");
            return -1;
        }
        
    }
    int unit_count = 4;
    printf("Reading data from \"input\", and writing it to stdout and \"output\"\n");
    // Read 4 units from "input" file
    read_units_to_memory(input, buffer, unit_count);
    // Print the 4 unites read to stdout with printf (convert data to text)
    print_units(stdout, buffer, unit_count);
    // Print the 4 unites read to "output" file as is (no conversion)
    write_units(output, buffer, unit_count);
    
    return 0;
}

