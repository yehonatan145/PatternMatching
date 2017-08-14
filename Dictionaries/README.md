Dictionaries Directory
======================

This is the folder contatining all the dictionaries for the system.

The file extension for dictionary files is ".dict"

## Files

### snort.dict and et.dict

These files are created by pulledpork.pl program in this directory (for more information see https://github.com/shirkdog/pulledpork).

Note that the next packages should be installed before pulledpork can run:
*		libcrypt-ssleay-perl
*		liblwp-useragent-determined-perl

The bash script to update this files is update-pulled-pork.sh and its usage is
./update-pulled-pork.sh < oinkcode >
(for more information on snort oinkcode see https://www.snort.org/oinkcodes)