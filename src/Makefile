PKGS=sdl2
CFLAGS=-Wall -ggdb -std=c11 -pedantic `pkg-config --cflags sdl2 SDL2_image`
LIBS=`pkg-config --libs sdl2 SDL2_image`

main: main.c
	$(CC) $(CFLAGS) -o main main.c $(LIBS)
