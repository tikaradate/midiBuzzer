#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#define HEADER_SIZE 14
#define TRACK_HDR_SIZE 8

struct header {
    char chunk_ID[4];
    uint32_t chunk_size;
    uint16_t format_type;
    uint16_t number_of_tracks;
    uint16_t time_division;
};

struct track_events {
    uint32_t delta_time; // it can use up to 4 bytes
    uint8_t type; // the 4 upper bits are for the event type and the 4 lower to the channel
    uint8_t parameter1;
    uint8_t parameter2;
};

struct track {
    char chunk_ID[4];
    uint32_t chunk_size;
    struct track_events *track_events; //many track events might happen
};

struct midi {
    struct header header;
    struct track *tracks; //since a midi might contain multiple tracks
};
// convert be to le, byte by byte


int main(void){
    FILE *input;
    struct midi *midi;
    int test, tracks; 
    uint8_t buf[HEADER_SIZE];

    input = fopen("joel.mid", "r");
    if (!input) {
        perror("Error at file opening");
        exit(1);
    }

    // reads the main header of the file
    midi = malloc(sizeof(struct midi));
    // need to read in a buffer to correct the endianess
    test = fread(buf, 1, HEADER_SIZE, input);
    if (test != HEADER_SIZE) {
        perror("Error at header");
        exit(1);
    }
    for(int i = 0; i < HEADER_SIZE; i+=2){
        midi[i] = ((uint8_t)buf[i+1]) | (((uint8_t)buf[i]) << 4);
    }
    printf("chunk_ID:\t%.4s \n"
            "chunk_size:\t%x\n"
            "format:\t\t%d\n"
            "track number:\t%d\n",
            midi->header.chunk_ID,    // wrong
            midi->header.chunk_size,  // wrong
            midi->header.format_type, // wrong
            midi->header.number_of_tracks);
    
    // track by track, reading its header then allocating sufficient space for the read
    // maybe check if format 0 dont do loop, although it's not that severe
    tracks = midi->header.number_of_tracks;
    for(int i = 0; i < tracks; i++){
        //midi->tracks = malloc(sizeof(struct track)*tracks);
        //test = fread(midi->tracks[0],1, TRACK_HDR_SIZE, input);
        // looks at chunk_size to read it nicely(?)

    }
    
    return 0;
}