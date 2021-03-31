# midiBuzzer

A little project to convert a midi file to something a piezo buzzer can play.

# status

Dumping the note on and note off events into a file that can be read and interpreted by the arduino.

# needs

Translation between the midi note information, which maps to a note in the twelve-tone equal temperament, to the Tone.h library on the arduino, which uses the frenquency of the note itself.

Translation between the delta time into a delay in ms.

# resources

I'm using the following sites to help with decoding the midi file:


https://web.archive.org/web/20141227205754/http://www.sonicspot.com:80/guide/midifiles.html

http://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html

http://www.somascape.org/midi/tech/mfile.html (somascape is, imo, the best of the three)

# ideas (in order of importance)

Create a directory instead of having one already there.

Debug mode with all the printf's scattered through the code.

Get the name of the current track.

Organize the project into proper files.

# bugs (in order of importance)

The text gets jumbled up on the respective meta event, it isn't a problem at all but it is there.
