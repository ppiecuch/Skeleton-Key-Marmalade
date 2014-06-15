#!/bin/bash
for f in background_forest_dark.png background_forest_light.png background_wood.png; do
	echo "** creating playbook_blur_$f ..."
	convert $f -filter Gaussian -resize 50% -define filter:sigma=10 -resize 600x1024! playbook_blur_$f
	# convert $f -channel RGBA -blur 0x15 -alpha off playbook_blur_$f
done
