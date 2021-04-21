#include <inttypes.h>

#include "midi.h"

#define HEADER_SIZE 14
#define TRACK_HDR_SIZE 8

void read_midi(struct midi *midi, FILE *input);

void read_header(struct midi *midi, FILE *input);

void read_tracks(struct midi *midi, FILE *input);

