#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "read.h"

// set of functions that convert little endian to big endian,
// or the other way around, byte by byte
static uint16_t le2be16 (uint16_t number){
    return((number<<8)|    // move byte 0 to byte 1
           (number>>8));   // move byte 1 to byte 0
}

static uint32_t le2be32 (uint32_t number){
    return(((number>>24)&0xff)       | // move byte 3 to byte 0
           ((number<<8) &0xff0000)   | // move byte 1 to byte 2
           ((number>>8) &0xff00)     | // move byte 2 to byte 1
           ((number<<24)&0xff000000)); // byte 0 to byte 3
} 


void read_midi(struct midi *midi, FILE *input){
    read_header(midi, input);
    read_tracks(midi, input);
}

void read_header(struct midi *midi, FILE *input){
    if (fread(midi, 1, HEADER_SIZE, input) != HEADER_SIZE) {
        perror("Error reading the file header");
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

}

void read_tracks(struct midi *midi, FILE *input){
    int tracks;
    tracks = midi->header.number_of_tracks;
    
    midi->tracks = malloc(sizeof(struct track)*tracks);
    for(int i = 0; i < tracks; i++){

        //printf("\n\ntrack # %d\n\n", i);
        
        fread(&midi->tracks[i].chunk_ID, 1, 4, input);
        fread(&midi->tracks[i].chunk_size, 1, 4, input);

        midi->tracks[i].chunk_size = le2be32(midi->tracks[i].chunk_size);

        int j, bytes;
        midi->tracks[i].track_events = malloc(sizeof(struct track_events)*(midi->tracks[i].chunk_size));
        bytes = 0;
        j = 0;

        // since many data have variable size, a while loop should suffice
        while(bytes < midi->tracks[i].chunk_size){

            // variable lenght delta time, every event has this
            uint32_t value;
            uint8_t c;

            bytes++;
            if ( (value = getc(input)) & 0x80 ){
                value &= 0x7F;
                do
                {
                    bytes++;
                    value = (value << 7) + ((c = getc(input)) & 0x7F);
                } while (c & 0x80);
            }
            midi->tracks[i].track_events[j].delta_time = value;
                        
            // now to checking which kind of event it is
            fread(&midi->tracks[i].track_events[j].event, 1, 1, input);
            bytes++;
            if (midi->tracks[i].track_events[j].event == 0xff){ // meta event
                uint8_t lenght;

                fread(&midi->tracks[i].track_events[j].meta.type, 1, 1, input);
                bytes++;

                fread(&midi->tracks[i].track_events[j].meta.lenght,1,1,input);
                bytes++;
                lenght = midi->tracks[i].track_events[j].meta.lenght;
                
                midi->tracks[i].track_events[j].meta.data = malloc(sizeof(uint8_t)*lenght);
               
                if(lenght > 0){
                    fread(&midi->tracks[i].track_events[j].meta.data, 1, lenght, input);
                    bytes += lenght;
                }
                /*
                printf("event_type:\t%x\n"
                    "event_lenght:\t%x\n"
                   , 
                   midi->tracks[i].track_events[j].meta.type, lenght);
                */
                switch (midi->tracks[i].track_events[j].meta.type)
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
                    fprintf(stderr,"Meta event not found, closing the program.\n");
                    exit(1);
                }
            
            
            } else if (midi->tracks[i].track_events[j].event == 0xf0){// sysex event
                fprintf(stderr,"sysex...");
            
            } else {// midi event
                uint8_t type, channel;
                
                type = midi->tracks[i].track_events[j].event & 0xf0;
                type >>= 4;
                channel = midi->tracks[i].track_events[j].event & 0x0f;
                // if that checks if the event is a running status,
                // i.e., the event type is the same as before
                // it only happens when the data is < 0x08
                if (type < 0x08){
                    midi->tracks[i].track_events[j].midi.parameter1 = midi->tracks[i].track_events[j].event;
                    midi->tracks[i].track_events[j].event = 0;
                    type = midi->last_event & 0xf0;
                    type >>= 4;
                    channel = midi->last_event & 0x0f;
                } else {
                    midi->last_event = midi->tracks[i].track_events[j].event;
                    // all midi events need this parameter, most use the second
                    fread(&midi->tracks[i].track_events[j].midi.parameter1, 1, 1, input);
                    bytes++;
                }

                /*
                printf("event:\t%x\n"
                    "type:\t%x\n"
                    "channel:\t%x\n"
                    "parameter1:\t%x\n"
                    , 
                    midi->tracks[i].track_events[j].event,
                    type,
                    channel,
                    midi->tracks[i].track_events[j].midi.parameter1);
                */
                switch (type)
                {
                case 0x08: // note off
                    fread(&midi->tracks[i].track_events[j].midi.parameter2, 1, 1, input);
                    //printf("parameter2:\t%x\n",midi->tracks[i].track_events[j].midi.parameter2);
                    bytes++;
                    break;
                case 0x09: // note on
                    fread(&midi->tracks[i].track_events[j].midi.parameter2, 1, 1, input);
                    //printf("parameter2:\t%x\n",midi->tracks[i].track_events[j].midi.parameter2);
                    bytes++;
                    break;
                case 0x0a: // note aftertouch
                    fread(&midi->tracks[i].track_events[j].midi.parameter2, 1, 1, input);
                    //printf("parameter2:\t%x\n",midi->tracks[i].track_events[j].midi.parameter2);
                    bytes++;
                    break;
                case 0x0b: // controller
                    fread(&midi->tracks[i].track_events[j].midi.parameter2, 1, 1, input);
                    //printf("parameter2:\t%x\n",midi->tracks[i].track_events[j].midi.parameter2);
                    bytes++;
                    break;
                case 0x0c: // program change
                    /* code */
                    break;
                case 0x0d: // channel aftertouch
                    /* code */
                    break;
                case 0x0e: // pitch bend
                    fread(&midi->tracks[i].track_events[j].midi.parameter2, 1, 1, input);
                    //printf("parameter2:\t%x\n",midi->tracks[i].track_events[j].midi.parameter2);
                    bytes++;
                    break;
                default:
                   
                   fprintf(stderr,"MIDI event not found, closing the program. %d\n",j);
                   exit(1);
                }
            }
            j++;
        }
    }
}