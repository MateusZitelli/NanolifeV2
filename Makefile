all:
	gcc -O3 -pipe -g -Wall main.c -o b.bin -fomit-frame-pointer `sdl-config --cflags` `sdl-config --libs`

