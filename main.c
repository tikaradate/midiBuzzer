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
// set of functions that convert le to be, byte by byte
// not necessary for a single byte ?
uint32_t le2be16 (uint32_t number){
    return(((number<<8) &0xff) |   // move byte 0 to byte 1
           ((number>>8) &0x00ff));      // move byte 1 to byte 0
} 

uint32_t le2be32 (uint32_t number){
    return(((number>>24)&0xff)       | // move byte 3 to byte 0
           ((number<<8) &0xff0000)   | // move byte 1 to byte 2
           ((number>>8) &0xff00)     | // move byte 2 to byte 1
           ((number<<24)&0xff000000)); // byte 0 to byte 3
} 

int main(void){
    FILE *input;
    struct midi *midi;
    uint32_t test, tracks, swap; 
    uint8_t buf[HEADER_SIZE];

    input = fopen("midis/joel.mid", "r");
    if (!input) {
        perror("Error at file opening");
        exit(1);
    }

    // reads the main header of the file
    midi = malloc(sizeof(struct midi));
    // need to read in a buffer to correct the endianess
    test = fread(midi, 1, HEADER_SIZE, input);
    if (test != HEADER_SIZE) {
        perror("Error at header");
        exit(1);
    }

    midi->header.chunk_size = le2be32(midi->header.chunk_size);
    midi->header.format_type = le2be16(midi->header.format_type);
    midi->header.number_of_tracks = le2be16(midi->header.number_of_tracks);

    printf("chunk_ID:\t%.4s \n"
            "chunk_size:\t%x\n"
            "format:\t\t%d\n"
            "tracks:\t\t%d\n",
            midi->header.chunk_ID,
            midi->header.chunk_size,
            midi->header.format_type,
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