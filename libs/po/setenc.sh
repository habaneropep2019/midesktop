#!/bin/bash

FILES="$(ls -1 *.po)"

for f in $FILES
do
	echo "Filename: "$f""
	fbase="$(basename "$f" .po)"
	if [ -e "$fbase.charset" ]
	then
		echo "Charset file indicates this encoding is appropriate: "$(cat $fbase.charset)""
	fi
	echo "Encoding detected as: "$(file -bi $f)""
	echo ""
done
