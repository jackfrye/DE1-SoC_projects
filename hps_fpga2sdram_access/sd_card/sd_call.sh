sudo ./make_sdimage.py -f \
-P u-boot-with-spl.sfp,num=3,format=raw,size=10M,type=A2  \
-P sdfs/*,num=1,format=vfat,size=100M \
-P rootfs/*,num=2,format=ext3,size=300M \
-s 512M \
-n sdcard_cv.img
