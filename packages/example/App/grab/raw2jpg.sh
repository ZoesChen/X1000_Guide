#/system/bin/sh

busybox find . -name '*.raw' | while read file; do 
	JPGFILE=${file/raw/jpg}
	echo "===$file ----- $JPGFILE"
	/data/raw2jpg -w 224 -h 208 -i $file -o $JPGFILE
done
