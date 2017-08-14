#!/bin/bash

## generate stream with "almost matches" from dictionaries files
## $1 - the name of the output file
## $2 - the length of the output (in bytes)
## $3.. - the list of dictionaries

## get random number between $1 (included) and $2 (not included)
function get_random {
	echo $(( $RANDOM % ($2 - $1) + $1 ))
}

if [ $# -lt 3 ]; then
	echo "have less than 3 arguments"
	echo "Usage:"
	echo "$0 <output file name> <length of output (in bytes)> <list of dictionary files..>"
	exit
fi

tmp_shuffled_file=$( mktemp tmp.XXXXXXXXXXXXXX )
dict_files=$( echo $@ | cut -f 3- -d" " )
cat $dict_files | shuf > $tmp_shuffled_file

rm $1

python write_first_lines.py $1 $tmp_shuffled_file $2

rm -f $tmp_shuffled_file