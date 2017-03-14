#!/bin/sh
# genext2fs wrapper calculating needed blocks/inodes values if not specified
GENEXT2FS=genext2fs
E2FSCK=e2fsck
TUNE2FS=tune2fs

while getopts x:d:D:b:i:N:m:g:e:o:zfqUPhVv f
do
    case $f in
	N) INODES=$OPTARG ;;
	b) BLOCKS=$OPTARG ;;
	d) TARGET_DIR=$OPTARG ;;
	# -o custom parameter
	o) OUTPUT_DIR=$OPTARG ;;
    esac
done

# calculate needed inodes
if [ $INODES -eq 0 ];
then
   INODES=$(find $TARGET_DIR | wc -l)
   INODES=$(expr $INODES + 400)
fi

# calculate needed blocks
if [ $BLOCKS -eq 0 ];
then
    # size ~= superblock, block+inode bitmaps, inodes (8 per block), blocks
    # we scale inodes / blocks with 10% to compensate for bitmaps size + slack
    BLOCKS=$(du -s -c -k $TARGET_DIR | grep total | sed -e "s/total//")
    BLOCKS=$(expr 500 + \( $BLOCKS + $INODES / 8 \) \* 11 / 10)
    # we add 1300 blocks (a bit more than 1 MiB, assuming 1KiB blocks) for
    # the journal if ext3/4
    # Note: I came to 1300 blocks after trial-and-error checks. YMMV.
	#if [ ${GEN} -ge 3 ]; then
    #   BLOCKS=$(expr 1300 + $BLOCKS )
    #fi
    # set -- $@ -b $BLOCKS
fi

echo $GENEXT2FS" -d "$TARGET_DIR" -b "$BLOCKS" -N "$INODES" "$OUTPUT_DIR

$GENEXT2FS -d $TARGET_DIR -b $BLOCKS -N $INODES  $OUTPUT_DIR

echo $E2FSCK" -fy "$OUTPUT_DIR
echo $TUNE2FS" -j "$OUTPUT_DIR
echo $E2FSCK" -fy "$OUTPUT_DIR

$E2FSCK -fy $OUTPUT_DIR
$TUNE2FS -j $OUTPUT_DIR
#./tune2fs -L $imagename $OUTPUT
$TUNE2FS -O extents,uninit_bg,dir_index $OUTPUT_DIR
$E2FSCK -fy $OUTPUT_DIR
$E2FSCK -fy $OUTPUT_DIR
