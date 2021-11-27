PKGS=sdl2
CFLAGS=-Wall -ggdb -std=c11 -pedantic `pkg-config --cflags sdl2`
LIBS=`pkg-config --libs sdl2`

main: main.c
	$(CC) $(CFLAGS) -o main main.c $(LIBS)
