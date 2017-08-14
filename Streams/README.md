Sterams Directory
=================

This is the folder containing all the streams for the system.

The file extension for stream files is ".stream"

## Generating stream files

### generating streams from dictionary files

You can generate stream files based on dictionary files with the script generate_stream_from_dictionaries.sh in this folder.

This script uses the file write_first_lines.py and together they create a stream file that have "almost matches" when trying
to match them with the given dictionary files.

Usage:
./generate_steram_from_dictionaries.sh output.stream len dictionary_files
Where output.stream is the name of the stream to generate, len is the length to generate, and dictionary_files is the list
of dictionaries to generate by. e.g.
./generate_stream_from_dictionaries.sh gen.stream 10240 ../Dictionaries/snort.dict ../Dictionaries/et.dict
(This is the way the file dictionaries_generated.stream was generated)

### Generating random streams

For generating random string you can use dd command
dd if=/dev/urandom of=./output.stream bs=len count=1
Where output.stream is the name of the stream, and len is the length to generate