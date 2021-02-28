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
    uint8_t event; // the 4 upper bits are the event type and the 4 lower the channel
    uint8_t parameter1;
    uint8_t parameter2;
};

struct track {
    char chunk_ID[4];
    uint32_t chunk_size;
    struct track_events *track_events; //many track events might happen
};

struct time_info{
        uint8_t format;
        uint16_t ticks_per_beat;
        uint8_t SMPTE_frames;
        uint8_t ticks_per_frame;
    };

struct midi {
    struct header header;
    struct track *tracks; //since a midi might contain multiple tracks
    struct time_info time_info;
};

// set of functions that convert little endian to big endian, byte by byte
// not necessary for a single byte
uint16_t le2be16 (uint16_t number){
    return((number<<8)|    // move byte 0 to byte 1
           (number>>8));   // move byte 1 to byte 0
}

uint32_t le2be32 (uint32_t number){
    return(((number>>24)&0xff)       | // move byte 3 to byte 0
           ((number<<8) &0xff0000)   | // move byte 1 to byte 2
           ((number>>8) &0xff00)     | // move byte 2 to byte 1
           ((number<<24)&0xff000000)); // byte 0 to byte 3
} 

void print_main_header(struct midi *midi){

    printf("chunk_ID:\t%.4s \n"
            "chunk_size:\t%x\n"
            "format:\t\t%d\n"
            "tracks:\t\t%d\n"
            "time format:\t%d\n"
            "time_div:\t%x\n\n",
            midi->header.chunk_ID,
            midi->header.chunk_size,
            midi->header.format_type,
            midi->header.number_of_tracks,
            midi->time_info.format,
            midi->header.time_division
            );
    
}

int main(void){
    FILE *input;
    struct midi *midi;
    uint32_t test, tracks, swap; 
    uint8_t time_format;

    input = fopen("midis/joel.mid", "r");
    if (!input) {
        perror("Error at file opening");
        exit(1);
    }

    // reads the main header of the file
    midi = malloc(sizeof(struct midi));
    // need to read in a buffer to correct the endianess
    if (fread(midi, 1, HEADER_SIZE, input) != HEADER_SIZE) {
        perror("Error at reading header");
        exit(1);
    }
    // no need to convert the ID to big endian
    midi->header.chunk_size = le2be32(midi->header.chunk_size);
    midi->header.format_type = le2be16(midi->header.format_type);
    midi->header.number_of_tracks = le2be16(midi->header.number_of_tracks);
    midi->header.time_division = le2be16(midi->header.time_division);

    // gets the type of time division(first bit), then the useful part(other 15 bits)
    midi->time_info.format = midi->header.time_division & 0x8000;
    if(midi->time_info.format == 0){
        midi->time_info.ticks_per_beat &= 0x7FFF;
    } else {
        midi->time_info.SMPTE_frames &= 0x7F00;
        midi->time_info.ticks_per_frame &= 0x00FF;;
    }

    //print_main_header(midi);

    // track by track, reading its header then allocating sufficient space for the read
    // maybe check if format 0 dont do loop, although it's not that severe
    tracks = midi->header.number_of_tracks;
    for(int i = 0; i < tracks; i++){
        printf("track # %d\n", i);
        // maybe test malloc?
        midi->tracks = malloc(sizeof(struct track)*tracks);

        if(fread(&midi->tracks[i],1, TRACK_HDR_SIZE, input) != TRACK_HDR_SIZE){
            perror("Error reading file");
            exit(1);
        }
        
        midi->tracks[i].chunk_size = le2be32(midi->tracks[i].chunk_size);

        /*
        printf("chunk_ID:\t%.4s\n"
               "chunk_size:\t%d\n"
               , midi->tracks[i].chunk_ID,
               midi->tracks[i].chunk_size);
        */

        // uses the chunk size to allocate sufficient memory
        
        midi->tracks[i].track_events = malloc(sizeof(struct track_events)*(midi->tracks[i].chunk_size));

        // variable lenght delta time, every event has this
        int count = 0;
        uint32_t value;
        uint8_t c;
        // is the if needed?
        if ( (value = getc(input)) & 0x80 ){
            value &= 0x7F;
            do
            {
               value = (value << 7) + ((c = getc(input)) & 0x7F);
            } while (c & 0x80);
        }
        midi->tracks[i].track_events[0].delta_time = value;
        
        // now to checking which kind of event it is
        fread(&midi->tracks[i].track_events[0].event, 1, 8, input);
        
        if (midi->tracks[i].track_events[0].event == 0xff){ // meta event
            uint8_t type;
            fread(&type, 1, 8, input);
            switch (type)
            {
            case 0x00:
                /* code */
                break;
            case 0x01:
                /* code */
                break;
            case 0x02:
                /* code */
                break;
            case 0x03:
                /* code */
                break;
            case 0x04:
                /* code */
                break;
            case 0x05:
                /* code */
                break;
            case 0x06:
                /* code */
                break;
            case 0x07:
                /* code */
                break;
            case 0x20:
                /* code */
                break;
            case 0x2f:
                /* code */
                break;
            case 0x51:
                /* code */
                break;
            case 0x54:
                /* code */
                break;
            case 0x58:
                /* code */
                break;
            case 0x59:
                /* code */
                break;
            case 0xff:
                /* code */
                break;
            default:
                exit(1);
                break;
            }
        
        
        } else if (midi->tracks[i].track_events[0].event == 0xf0){// sysex event
            // not sure what to do yet
        
        } else {// midi event
            uint8_t type, channel;
            type = midi->tracks[i].track_events[0].event & 0xf0;
            channel = midi->tracks[i].track_events[0].event & 0x0f;
            switch (type)
            {
            case 0x08:
                /* code */
                break;
            case 0x09:
                /* code */
                break;
            case 0x0a:
                /* code */
                break;
            case 0x0b:
                /* code */
                break;
            case 0x0c:
                /* code */
                break;
            case 0x0d:
                /* code */
                break;
            case 0x0e:
                /* code */
                break;
    
            default:
                exit(1);
            }
        }
        


        /*
        printf("event_type:\t%x\n"
               "event_type:\t%x\n"
               "parameter1:\t%d\n"
               "parameter2:\t%d\n"
               , 
               midi->tracks[i].track_events[0].type >> 4,
               midi->tracks[i].track_events[0].type & 0x0F,
               midi->tracks[i].track_events[0].parameter1,
               midi->tracks[i].track_events[0].parameter2);
        */
    }
    
    return 0;
}
/*
80

1  0000000

1 0000001 0 0000000

10000001 00000000

81 00
*/