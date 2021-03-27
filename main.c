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

struct midi_event {
    uint8_t parameter1;
    uint8_t parameter2;
};
struct meta_event {  
    uint8_t type;
    uint8_t lenght;
    uint8_t *data;
};
struct sysex_event{
            //???
};
struct track_events {
    uint32_t delta_time; // it can use up to 4 bytes
    uint8_t event; // the 4 upper bits are the event type and the 4 lower the channel
    
    union{
        struct midi_event midi;
        struct meta_event meta;
    };

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
    uint8_t last_event;
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
    uint32_t test, tracks, swap, whole_size; 
    uint8_t time_format;
    
    whole_size = 0;

    input = fopen("midis/joel.mid", "r");
    if (!input) {
        perror("Error at file opening");
        exit(1);
    }

    // reads the main header of the file
    midi = malloc(sizeof(struct midi));
    // need to read in a buffer to correct the endianess
    if (fread(midi, 1, HEADER_SIZE, input) != HEADER_SIZE) {
        perror("Error reading the file header");
        exit(1);
    }
    whole_size += HEADER_SIZE;

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
    // maybe test malloc?
        
    tracks = midi->header.number_of_tracks;
    midi->tracks = malloc(sizeof(struct track)*tracks);
    for(int i = 0; i < tracks; i++){

        printf("\n\ntrack # %d\n\n", i);
        
        fread(&midi->tracks[i].chunk_ID, 1, 4, input);
        whole_size += 4;
        fread(&midi->tracks[i].chunk_size, 1, 4, input);
        whole_size += 4;
        midi->tracks[i].chunk_size = le2be32(midi->tracks[i].chunk_size);
        printf("%x\n", midi->tracks[i].chunk_size);

        /*
        printf("chunk_ID:\t%.4s\n"
               "chunk_size:\t%d\n"
               , midi->tracks[i].chunk_ID,
               midi->tracks[i].chunk_size);
        */

        // uses the chunk size to allocate sufficient memory
        int j, bytes;
        midi->tracks[i].track_events = malloc(sizeof(struct track_events)*(midi->tracks[i].chunk_size));
        bytes = 0;
        j = 0;
        // since many data have variable size, a while loop should suffice
        while(bytes < midi->tracks[i].chunk_size){
        //for(int j = 0; j < (int)(midi->tracks[i].chunk_size/8); j++ ){
            printf("size of %d\n", bytes);
            // variable lenght delta time, every event has this
            uint32_t value;
            uint8_t c;
            // is the if needed?
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
                printf("event_type:\t%x\n"
                    "event_lenght:\t%x\n"
                   , 
                   midi->tracks[i].track_events[j].meta.type, lenght);
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

                
                printf("event:\t%x\n"
                    "type:\t%x\n"
                    "channel:\t%x\n"
                    "parameter1:\t%x\n"
                    , 
                    midi->tracks[i].track_events[j].event,
                    type,
                    channel,
                    midi->tracks[i].track_events[j].midi.parameter1);
                // for now not needed, only when transcribing to a buzzer readable format
                switch (type)
                {
                case 0x08: // note off
                    fread(&midi->tracks[i].track_events[j].midi.parameter2, 1, 1, input);
                    printf("parameter2:\t%x\n",midi->tracks[i].track_events[j].midi.parameter2);
                    bytes++;
                    break;
                case 0x09: // note on
                    fread(&midi->tracks[i].track_events[j].midi.parameter2, 1, 1, input);
                    printf("parameter2:\t%x\n",midi->tracks[i].track_events[j].midi.parameter2);
                    bytes++;
                    break;
                case 0x0a: // note aftertouch
                    fread(&midi->tracks[i].track_events[j].midi.parameter2, 1, 1, input);
                    printf("parameter2:\t%x\n",midi->tracks[i].track_events[j].midi.parameter2);
                    bytes++;
                    break;
                case 0x0b: // controller
                    fread(&midi->tracks[i].track_events[j].midi.parameter2, 1, 1, input);
                    printf("parameter2:\t%x\n",midi->tracks[i].track_events[j].midi.parameter2);
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
                    printf("parameter2:\t%x\n",midi->tracks[i].track_events[j].midi.parameter2);
                    bytes++;
                    break;
                default:
                   
                   fprintf(stderr,"MIDI event not found, closing the program. %d\n",j);
                   exit(1);
                }
            }
            j++;
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
        whole_size += bytes;
    }
 
    /* rewriting the contents that just got read to a file 
    so I can check whether or not it got read correctly */

    
    fclose(input);
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
