CC = clang
CFLAGS = -I../src -I../include -I../build/external/raylib-master/src -I../build/external/raylib-master/src/external -I../build/external/raylib-master/src/external/glfw/include -g -std=c17 -Wall

BIN_DIR = bin
SRC_DIR = .
SRC_TESTS = $(wildcard $(SRC_DIR)/*_test.c)
TESTS = $(patsubst $(SRC_DIR)/%_test.c,$(BIN_DIR)/%_test,$(SRC_TESTS))

all: tests

tests: $(BIN_DIR) $(TESTS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Pattern rule: build each test binary from its .c file
$(BIN_DIR)/tile_map_test: $(SRC_DIR)/tile_map_test.c ../src/tile/tile_map.c
	$(CC) $(CFLAGS) -o $@ $^

# Pool logic test needs more dependencies
$(BIN_DIR)/pool_logic_test: $(SRC_DIR)/pool_logic_test.c \
	../src/game/board.c \
	../src/game/camera.c \
	../src/tile/tile_map.c \
	../src/tile/pool_manager.c \
	../src/tile/pool.c \
	../src/tile/tile.c \
	../src/grid/hex_geometry.c \
	../src/grid/grid_cell_utils.c \
	../src/grid/edge_map.c \
	../src/renderer/edge_render_list.c \
	../src/utility/array_shuffle.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TESTS)
	rm -rf $(BIN_DIR)
