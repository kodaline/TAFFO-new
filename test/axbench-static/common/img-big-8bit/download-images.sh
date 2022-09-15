#!/bin/bash

echo "Downloading large images dataset..."
wget http://imagecompression.info/test_images/rgb8bit.zip
echo "Unzipping images..."
unzip rgb8bit.zip

echo "Converting images..."
mogrify -format png *.ppm

for img in ./*.png
do
    python3 ../scripts/png2rgb.py rgb $img $(basename -s .png $img).rgb
done

rm *.ppm readme.txt *.png

mkdir 1M
for img in artificial cathedral fireworks flower_foveon hdr leaves_iso_1600 leaves_iso_200 nightshot_iso_100 nightshot_iso_1600
do
    mv $img.rgb ./1M/
done

mkdir 10M
for img in big_building big_tree bridge deer spider_web
do
    mv $img.rgb ./10M/
done
