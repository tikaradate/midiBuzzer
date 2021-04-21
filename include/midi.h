#ifndef __MIDI__
#define __MIDI__
#include <inttypes.h>

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

// functions that convert the endianess of 2 or 4 bytes
uint16_t le2be16 (uint16_t number);
uint32_t le2be32 (uint32_t number);

#endif