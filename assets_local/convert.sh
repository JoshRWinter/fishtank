#!/bin/bash

# if no files in /assets_local are .png, this script need not be run. convert is imagemagick
for f in *.png; do
	if [ "$f" == '*.png' ]; then
		exit 0
	fi
	rm -rf ${f%%.png}.tga
	convert $f -compress none ${f%%.png}.tga
done

# combine tank textures
convert tank_base.tga tank_dmg.tga +append tank.tga

rm *.png
