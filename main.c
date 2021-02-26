#include <stdio.h>
#include <inttypes.h>

struct midi {
    struct header {
        char chunk_ID[4];
        uint32_t chunk_size;
        uint16_t format_type;
        uint16_t number_of_tracks;
        uint16_t time_division;

    }
    struct track {
        char chunk_ID[4];
        uint32_t chunk_size;     
    }
};

int main(void){
    return 0;
}