#!/bin/bash

## this script update the rules in this directory using pulledpork and regenerate the patterns-list files from it

# snort rules file extension
snort_rules_ext='rules'
# generated patterns file extension
patterns_ext='dict'

# variables for the online rules updated
oinkcode='82f3cb8a183c29ef718df48136b1723e08caa894'
snort_url="https://www.snort.org/rules/|snortrules-snapshot.tar.gz|$oinkcode"
snort_file_name='snort'
et_url='https://rules.emergingthreats.net/|emerging.rules.tar.gz|open-nogpl'
et_file_name='et'

# run pulledpork to pull rules from online
# $1 - the url of the rules (in pulledpork format)
# $2 - the output file name (the output file will be with snort rules file extnesion)
function run_pulledpork {
	rm -f tmp/*
	./pulledpork.pl -T -u $1 -o $2.$snort_rules_ext -c pulledpork.conf -S 2.9.8.3 > /dev/null
}

# convert snort-rules format file to patterns-list format file (simply list of patterns seperated by \n)
# $1 - the file name (input file is with snort-rules extension, output file would be with patterns-list extension)
function convert_rules_to_patterns {
	grep -o 'content:\"[^"]*' < $1.$snort_rules_ext | cut -c 10- > $1.$patterns_ext
}

# script actions:

run_pulledpork		$snort_url	$snort_file_name
run_pulledpork		$et_url		$et_file_name

convert_rules_to_patterns	$snort_file_name
convert_rules_to_patterns	$et_file_name

rm -f tmp/*