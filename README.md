PatternMatching
===============

This is a project for comparing some Multi-Patterns Matching (a.k.a "Dictionary Matching") Streaming Algorithms.

A Multi-Pattern Matching Streaming algorithm is an algorithm that, given a dictionary (list of patterns), can go
through a stream (stream is a long text, that the algorithm can get only character by character, instead of getting
all of the text at once like the usual pattern-matching model), and on every arriving character from the stream,
can say what patterns from the given dictionary matched the stream with their last character matching the current character.

This model is great for internet traffic monitoring (i.e. discover known viruses by sniffing the internet traffic),
because the list of viruses is known in advanced, and the traffic arriving is like a stream (packet by packet).

This system can measure the performance of some Multi-Pattern Matching Streaming Algorithms, and their success rate
(since some of the algorithms are rendom), and output the results to a csv file.

# User Manual

There is dictionary files in the subdirectory "Dictionaries", and stream files in the subdirectory "Streams".

The dictionariy files extension is ".dict", and the stream files extension is ".stream".

For more information on those, see the README files in the subdirectories

## Setup

Compile the system using make command in the terminal (from the main directory), to create a new file named exe.

Note that for the measurement I used linux's perf_event interface, which might not work properly on every computer,
which mean that there is a possibility that some linux machines would fail to run the system.
To fix this, you can change the Core/src/measure.h file so the measurement performed would fit your computer.
For more information see the Developer's manual

## Using the system

Lets say you have the next dictionary files in the Dictionaries directory:
*  a.dict
*  b.dict
*  c.dict

And you have the next stream files in the Stream directory:
*  A.stream
*  B.stream

And you want to measure the algorithms performance on them, and save the results in results.csv

So you need to run the following command in the terminal in the main directory:

	./exe -d Dictionaries/a.dict -d Dictionaries/b.dict -d Dictionaries/c.dict -s Streams/A.stream -s Streams/B.stream -o results.csv

Flags:

* -d before every dictionary file
* -s before every stream file
* -o before the output file (only one output file allowed)
* -v (optional) for verbose mode (print more detailed output)

Note that by putting several dictionary files, the algorithm get all the patterns in all of them as one dictionary.

# Developer Manual

The source code is in Core/src/ directory.

For full developer's guide, go to Core/src/README.md