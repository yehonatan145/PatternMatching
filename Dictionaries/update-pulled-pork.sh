#!/bin/bash

## this script update the rules in this directory using pulledpork and regenerate the patterns-list files from it
## this script should get an oinkcode as a first and only parameter (see https://www.snort.org/oinkcodes)

# snort rules file extension
snort_rules_ext='rules'
# generated patterns file extension
patterns_ext='dict'

# variables for the online rules updated
oinkcode="$1"
snort_url="https://www.snort.org/rules/|snortrules-snapshot.tar.gz|$oinkcode"
snort_file_name='snort'
et_url='https://rules.emergingthreats.net/|emerging.rules.tar.gz|open-nogpl'
et_file_name='et'

# run pulledpork to pull rules from online
# $1 - the url of the rules (in pulledpork format)
# $2 - the output file name (the output file will be with snort rules file extnesion)
function run_pulledpork {
	rm -f tmp/*
	./pulledpork.pl -T -u $1 -o $2.$snort_rules_ext -c pulledpork.conf -S 2.9.8.3 #> /dev/null
}

# convert snort-rules format file to patterns-list format file (simply list of patterns seperated by \n)
# $1 - the file name (input file is with snort-rules extension, output file would be with patterns-list extension)
function convert_rules_to_patterns {
	grep -o 'content:\"[^"]*' < $1.$snort_rules_ext | cut -c 10- > $1.$patterns_ext
}

# generate the dict file
# $1 - the url of the rules (in pulledpork format)
# $2 - the output file name
function generate_dict_file {
	run_pulledpork $1 $2
	convert_rules_to_patterns $2
	tmp_file=$( mktemp tmp.XXXXXXXXXXXXXXX )
	uniq $2.$patterns_ext > $tmp_file
	mv -f $tmp_file $2.$patterns_ext
	rm -f $2.$snort_rules_ext
}

# script actions:

generate_dict_file $snort_url $snort_file_name
generate_dict_file $et_url    $et_file_name

rm -f tmp/*