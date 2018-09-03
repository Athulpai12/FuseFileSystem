ramdisk:
	gcc -Wall ramdisk.c `pkg-config fuse --cflags --libs` -o fuse_output

clean:
	rm ramdisk

