#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "read.h"
#include "ard_fmt.h"
#include "rewrite.h"

int main(int argc, char **argv){
    if(argc < 2){
        fprintf(stderr, "No file adress given.\n");
        exit(1);
    }

    FILE *input;
    struct midi *midi;

    input = fopen(argv[1], "r");
    if (!input) {
        perror("Error at file opening");
        exit(1);
    }

    midi = malloc(sizeof(struct midi));

    read_midi(midi, input);

    fclose(input);
    /* rewriting for debugging purposes */
    // 

    convert_to_ard(midi);

    return 0;
}
/* 
variable length encoding
80

1000000

1  0000000

1 0000001 0 0000000

10000001 00000000

81 00
*/
