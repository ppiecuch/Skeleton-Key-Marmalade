#!/bin/bash

ascii2hex(){ a="$@";s=0000000;printf "$a" | hexdump | grep "^$s" | sed s/' '*$// | sed s/' '/\\\\x/g | sed s/^$s//;}

for f in `ls *.gxfont`
do
	EXTENSION=${f##*.}
	FILENAME=`basename $f | sed 's/\(.*\)\..*/\1/'`
	DIRPATH=`dirname $f`

	echo "converting $f ..."

	sed \
	    -e "s/CIwGxFont/CIwGxFont ${FILENAME} =/" \
	    -e "s/utf8 \(.*\)/utf8 : \1,/" \
	    -e "s/image \(.*\)/image : \"\1\",/" \
	    -e "s/charmap \(.*\)/charmap  : \1/" \
	    -e "s/^}$/};/g" \
	    -e "s/\\u0000/\\x/g" \
	    $f > ${FILENAME}.h
done
