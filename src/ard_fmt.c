#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "ard_fmt.h"

void convert_to_ard(struct midi *midi){
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



}