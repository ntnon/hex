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
$(BIN_DIR)/%_test: $(SRC_DIR)/%_test.c ../src/tile/tile_map.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TESTS)
	rm -rf $(BIN_DIR)
