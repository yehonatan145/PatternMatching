Developer's Guide
=================

The system configuration is defined in conf.h file, and the configuration is filled during argument parsing.

## Patterns Tree

A patterns tree is a reversed suffix tree, i.e. if a pattern x is suffix of pattern y, then x is ancestor of y in the tree.

The system builds the patterns tree from the dictionary files, in the following way:

### Full Patterns Tree

First, we build a full patterns tree, which means that the edges to children save the prefix of the children.

For example, if we have the patterns: {abcdefg, cdefg, efg, afg, fg}, the tree would be:
(where every edge saves what is written on it)

                                root
                                 |
                                 | fg
                                 |
                                 1
                                / \
                             e /   \ a
                              /     \
                             2       3
                             |
                          cd |
                             |
                             4
                             |
                          ab |
                             |
                             5

Where the patterns of the nodes are:
* node 1 - fg
* node 2 - efg
* node 3 - afg
* node 4 - cdefg
* node 5 - abcdefg


As you can see, if we have the longest pattern that matches at some position, then all the other pattern matching
in the same position, is its suffixes, and the suffixes of a pattern is exactly all the nodes on the path from
the pattern to the root.

On every node, we save an InternalPatternID, which is the dictionary file number, and the line number in that file
in which the pattern was found.

### Regular Patterns Tree

Now we convert the full patterns tree to a reuglar one, which is the same, except that the edges doesn't contain
information on the children prefixes (reducing the size, from O(sum of patterns' lengths) to O(number of patterns) )

Now, although we lost the way to construct the pattern from the tree, we can know what the pattern is, because
of the InternalPatternID and the dictionary files used.

We also define a pointer to a patterns tree node as pattern_id_t, and give that to the algoirthms as the pattern id.
When the algorithms return the id of the longest matching pattern, we can find out all the matching patterns from it.

All of the pattern tree implementation and definition is in "PatternsTree.h" & "PatternsTree.c"

## Mps interface

Every mps algo (Multi-Pattern Matching Algorithm) implement the same interface for searching patterns.

The functions for the interface is saved in variable mps_table defined in "mps.h".

The mps_table is an array of MpsElem where every algo have an index in the table representing it.

The table is initialized in the function mps_table_setup in "mps.c".

MpsElem have "name" member for the name of the algorithm and the following functions that every algo must implement:

### void* create()

Create new object for that algorithm (the object is always void*)

### void add_pattern(void* obj, char* pat, size_t len, pattern_id_t)

Add new pattern to the mps object

Note that the pattern is binary, and therefore can contain \x00 in the middle of the pattern,
and that is the reason we also give the length of the pattern.

If you want to save the pattern, you should copy it, since the given pattern can change in memory
(And be sure to use memcpy and not strcpy since strcpy stops on \x00 and you shouldn't)

The pattern_id_t is primitive type, and therefore can be used with operators such as '=' & '=='.

The null_pattern_id variable represents non-pattern and you can use it to save that something is not a pattern

### void compile(void* obj)

Compile the mps object, making it ready to search in the stream (no more patterns are added after that function was called).

### pattern_id_t read_char(void* obj, char c)

This function is called on every character arriving from the stream. (called only after compile was called)

The function should return the pattern_id_t of the longest pattern matches the current position.

If there is no matching pattern, return null_pattern_id

### void reset(void* obj)

Reset the mps object, so that the next arriving character would be considered the first ("deleting stream history")

### void total_mem(void* obj)

Return the total memory currently used for the mps object, used for measurement purposes

### void free(void *obj)

Free all allocated memory in the mps object

## Adding new algorithm instruction

To add an algorithm to the system, follow the next steps:

* Add the algo to the enum at the beginning of "mps.h" (before MPS_SIZE!!!)

* create function to add the implementation to the mps_table (preferably mps_algo_name_register)
  that adds the implemented mps functions to mps_table[ALGO_ENUM]

* include your header file in "mps.c", and add a call to your registering function in mps_table_setup()

And thats it.

## measurement

The file "measure.c" is responsible for measuring the algorithms performance and accuracy (based on a reliable algorithm).

It uses perf_event interface in linux.

We initialize static variable perf_events (in "measure.h") containing the information on what performance are measured by the system.
Note that not all computers have all the possible measures and therefore the current measurement would not work on any computer.
To make it suitable for your computer, check what perf_events counters your system soppurt, and divide them to perf_events groups
whatever you like, and set perf_events to that groups.

In order to check accuracy of the algorithms, we must save the algorithm results, but we can't save all of them in memory
(the stream is too big for that).

So we work in buffers of size STREAM_BUF_SIZE, in the following way:
We read the stream in chunks of STREAM_BUF_SIZE, and save in buffer.
We save space for holding the algorithm answers on all the characters in the stream, for both the current algo and the reliable one.
we continue the measuring, fill the results buffer with the algorihm running on the stream buffer, and stop the measuring.
We do the same thing for the reliable algo (without measuring), and then compare the results
