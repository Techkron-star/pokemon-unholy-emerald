CC ?= gcc
CFLAGS ?= -O2 -std=c99 -Wall -Wextra -Wconversion -Werror
CPPFLAGS += -DGENESIS_HOST -I../../include
SRCS = $(wildcard ../../src/genesis/*.c)
HEADERS = $(wildcard ../../include/genesis_*.h)
SEEDS ?= 10000
.PHONY: test fast asan clean demo
build:
	mkdir -p build
build/genesis_test: $(SRCS) $(HEADERS) genesis_test.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -DGENESIS_TEST_SEEDS=$(SEEDS) $(SRCS) genesis_test.c -o $@
test: build/genesis_test
	./build/genesis_test
fast: | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -DGENESIS_TEST_SEEDS=100 $(SRCS) genesis_test.c -o build/genesis_fast
	./build/genesis_fast
asan: | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -g -fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover=all -no-pie -DGENESIS_TEST_SEEDS=$(SEEDS) $(SRCS) genesis_test.c -o build/genesis_asan
	./build/genesis_asan
clean:
	rm -rf build
build/genesis_demo: $(SRCS) $(HEADERS) genesis_demo.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRCS) genesis_demo.c -o $@
demo: build/genesis_demo
	./build/genesis_demo $(SEED) $(BIOME)
.PHONY: native-test
native-test: | build
	$(CC) -Inative_mock -I../../include $(CFLAGS) $(SRCS) genesis_native_test.c -o build/genesis_native_test
	./build/genesis_native_test
.PHONY: edge-test
edge-test: | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRCS) genesis_edge_test.c -o build/genesis_edge_test
	./build/genesis_edge_test
