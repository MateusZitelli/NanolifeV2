all:
	gcc -g -O3 -pipe -Wall main.c -o b.bin -fomit-frame-pointer `sdl-config --cflags` `sdl-config --libs`

