#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "read.h"


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

    input = fopen("midis/joel.mid", "r");
    if (!input) {
        perror("Error at file opening");
        exit(1);
    }

    // reads the main header of the file
    midi = malloc(sizeof(struct midi));

    read_midi(midi, input);

    fclose(input);
    /* rewriting the contents that just got read to a file 
    so I can check whether or not it got read correctly */

    /*
    FILE *output;
    output = fopen("./teste.mid","w");
    midi->header.chunk_size = le2be32(midi->header.chunk_size);
    midi->header.format_type = le2be16(midi->header.format_type);
    midi->header.number_of_tracks = le2be16(midi->header.number_of_tracks);
    midi->header.time_division = le2be16(midi->header.time_division);
    fwrite(&midi->header,1,HEADER_SIZE, output);
    for(int i = 0; i < tracks; i++){
        
        midi->tracks[i].chunk_size = le2be32(midi->tracks[i].chunk_size);
        fwrite(&midi->tracks[i],1,TRACK_HDR_SIZE, output);
        midi->tracks[i].chunk_size = le2be32(midi->tracks[i].chunk_size);

        int bytes = 0;
        int j = 0;

        while(bytes < midi->tracks[i].chunk_size){
            
            uint32_t buffer;
            buffer = midi->tracks[i].track_events[j].delta_time & 0x7F;
            bytes++;
            while ( (midi->tracks[i].track_events[j].delta_time >>= 7) )
            {
                bytes++;
                buffer <<= 8;
                buffer |= ((midi->tracks[i].track_events[j].delta_time & 0x7F) | 0x80);
            }
            
            while (1)
            {
                putc(buffer,output);
                if (buffer & 0x80)
                    buffer >>= 8;
                else
                   break;
            }
            if(midi->tracks[i].track_events[j].event != 0){
                fwrite(&midi->tracks[i].track_events[j].event, 1, 1, output);
                bytes++;
            }
            if(midi->tracks[i].track_events[j].event == 0xff){
                fwrite(&midi->tracks[i].track_events[j].meta.type,1,1,output);
                bytes++;
                fwrite(&midi->tracks[i].track_events[j].meta.lenght,1,1,output);
                bytes++;
                if(midi->tracks[i].track_events[j].meta.type == 0x03){
                    printf("wtf\n");

                }
                fwrite(&midi->tracks[i].track_events[j].meta.data,1,midi->tracks[i].track_events[j].meta.lenght,output);
                bytes += midi->tracks[i].track_events[j].meta.lenght;
            } else if(midi->tracks[i].track_events[j].event == 0xf0) {
                //??
            } else {
                uint8_t type;
                if(midi->tracks[i].track_events[j].event == 0){
                    type = midi->last_event & 0xf0;
                    type >>= 4;
                } else {
                    midi->last_event = midi->tracks[i].track_events[j].event;
                    type = midi->tracks[i].track_events[j].event & 0xf0;
                    type >>= 4;

                }
                fwrite(&midi->tracks[i].track_events[j].midi.parameter1, 1, 1, output);
                bytes++;
                switch (type)
                {
                case 0x08: // note off
                    fwrite(&midi->tracks[i].track_events[j].midi.parameter2, 1, 1, output);
                    bytes++;
                    break;
                case 0x09: // note on
                    fwrite(&midi->tracks[i].track_events[j].midi.parameter2, 1, 1, output);
                    bytes++;
                    break;
                case 0x0a: // note aftertouch
                    fwrite(&midi->tracks[i].track_events[j].midi.parameter2, 1, 1, output);
                    bytes++;
                    break;
                case 0x0b: // controller
                    fwrite(&midi->tracks[i].track_events[j].midi.parameter2, 1, 1, output);
                    bytes++;
                    break;
                case 0x0c: // program change
                    
                    break;
                case 0x0d: // channel aftertouch
                    
                    break;
                case 0x0e: // pitch bend
                    fwrite(&midi->tracks[i].track_events[j].midi.parameter2, 1, 1, output);
                    bytes++;
                    break;
                default:
                    fprintf(stderr,"MIDI event not found, closing the program. %d\t%d\n",j, bytes);
                    exit(1);
                }
            }
            j++;
        }
        
    }
    */

    FILE *output;
    char file[20];

    for(int i = 0; i < midi->header.number_of_tracks; i++){

        char number[3];
        sprintf(number,"%d",i);
        strcpy(file, "./tracks/track_");
        if(i < 10){
            strncat(file, number, 1);
        } else {
            strncat(file, number, 2);
        }

        output = fopen(file,"w+");
        // maybe not necessary?
        fprintf(output, "delta_time\tnote\n");

        int bytes = 0;
        int j = 0;

        while(bytes < midi->tracks[i].chunk_size){
            
            uint32_t buffer;
            buffer = midi->tracks[i].track_events[j].delta_time & 0x7F;
            bytes++;
            fprintf(output, "%d\t", midi->tracks[i].track_events[j].delta_time);
            while ( (midi->tracks[i].track_events[j].delta_time >>= 7) )
            {
                bytes++;
                buffer <<= 8;
                buffer |= ((midi->tracks[i].track_events[j].delta_time & 0x7F) | 0x80);
            }

            
            
            if(midi->tracks[i].track_events[j].event != 0){
                bytes++;
            }
            if(midi->tracks[i].track_events[j].event == 0xff){
                bytes++;
                bytes++;
                bytes += midi->tracks[i].track_events[j].meta.lenght;
                fprintf(output, "0\n");
            } else if(midi->tracks[i].track_events[j].event == 0xf0) {
                fprintf(output, "0\n");
                //??
            } else {
                uint8_t type;
                if(midi->tracks[i].track_events[j].event == 0){
                    type = midi->last_event & 0xf0;
                    type >>= 4;
                } else {
                    midi->last_event = midi->tracks[i].track_events[j].event;
                    type = midi->tracks[i].track_events[j].event & 0xf0;
                    type >>= 4;

                }
                bytes++;
                switch (type)
                {
                case 0x08: // note off
                    fprintf(output, "%d\n", midi->tracks[i].track_events[j].midi.parameter1);
                    bytes++;
                    break;
                case 0x09: // note on
                    fprintf(output, "%d\n", midi->tracks[i].track_events[j].midi.parameter1);
                    bytes++;
                    break;
                default:
                    fprintf(output, "0\n");
                }
            }
            j++;
        }
        fclose(output);
    }



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
