CC = clang
CFLAGS = -I../src -I../include \
         -I../build/external/raylib-master/src \
         -I../build/external/raylib-master/src/external \
         -I../build/external/raylib-master/src/external/glfw/include \
         -g -std=c17 -Wall

TESTS = tile_map_test

all: $(TESTS)

tile_map_test: tile_map_test.c ../src/tile/tile_map.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TESTS)
