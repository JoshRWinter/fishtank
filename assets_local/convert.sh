#!/bin/bash

# if no files in /assets_local are .png, this script need not be run. convert is imagemagick
for f in *.png; do
	if [ "$f" == '*.png' ]; then
		exit 0
	fi
	rm -rf ${f%%.png}.tga
	convert $f -compress none ${f%%.png}.tga
done

# combine platform textures
convert platform_1.tga platform_2.tga platform_3.tga +append platform.tga

# combine particle platform textures
convert particle_platform_1.tga particle_platform_2.tga +append particle_platform.tga

# combine beacon textures
convert beacon_1.tga beacon_2.tga +append beacon.tga

# combine tank textures
convert tank_base.tga tank_dmg.tga +append tank.tga

rm *.png

