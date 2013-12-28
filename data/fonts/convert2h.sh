#!/bin/bash

for f in `ls *.gxfont`
do
	EXTENSION=${f##*.}
	FILENAME=`basename $f | sed 's/\(.*\)\..*/\1/'`
	DIRPATH=`dirname $f`

	echo "converting $f ..."

	native2ascii -encoding UTF-8 $f | sed \
	    -e "s/CIwGxFont/CIwGxFont ${FILENAME} =/" \
	    -e "s/utf8 \(.*\)/utf8 : \1,/" \
	    -e "s/image \(.*\)/image : \"\1\",/" \
	    -e "s/charmap \(.*\)/charmap  : \1/" \
	    -e "s/^}$/};/g" \
	    -e "s/\\u00/\\x/g" \
	    > ${FILENAME}.h
done
