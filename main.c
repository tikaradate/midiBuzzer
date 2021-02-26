#include <stdio.h>
#include <inttypes.h>

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

int main(void){
    return 0;
}