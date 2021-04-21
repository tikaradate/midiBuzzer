#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "rewrite.h"

void rewrite(struct midi *midi){

    FILE *output;
    output = fopen("./teste.mid","w");
    midi->header.chunk_size = le2be32(midi->header.chunk_size);
    midi->header.format_type = le2be16(midi->header.format_type);
    midi->header.number_of_tracks = le2be16(midi->header.number_of_tracks);
    midi->header.time_division = le2be16(midi->header.time_division);
    fwrite(&midi->header,1,HEADER_SIZE, output);
    for(int i = 0; i < midi->header.number_of_tracks; i++){
        
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
}