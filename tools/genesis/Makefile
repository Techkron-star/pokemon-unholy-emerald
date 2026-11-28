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

.PHONY: genesis2-test genesis2-asan genesis2-topology-test genesis2-topology-asan
G2_SRCS = ../../src/genesis2/world.c ../../src/genesis2/topology.c
../../include/genesis2/assets.generated.h: genesis2_assets.py genesis2_modules.py genesis_bake.py genesis_puzzles.py
	cd ../.. && python3 tools/genesis/genesis2_assets.py --output include/genesis2/assets.generated.h --manifest build/genesis2-assets.json
genesis2-test: ../../include/genesis2/assets.generated.h | build
	$(CC) $(CFLAGS) -I../../include $(G2_SRCS) genesis2_test.c -o build/genesis2_test
	./build/genesis2_test 1000
genesis2-asan: ../../include/genesis2/assets.generated.h | build
	$(CC) $(CFLAGS) -g -fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover=all -no-pie -I../../include $(G2_SRCS) genesis2_test.c -o build/genesis2_asan
	./build/genesis2_asan 1000
genesis2-topology-test: ../../include/genesis2/assets.generated.h | build
	$(CC) $(CFLAGS) -I../../include $(G2_SRCS) genesis2_topology_test.c -o build/genesis2_topology_test
	./build/genesis2_topology_test
genesis2-topology-asan: ../../include/genesis2/assets.generated.h | build
	$(CC) $(CFLAGS) -g -fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover=all -no-pie -I../../include $(G2_SRCS) genesis2_topology_test.c -o build/genesis2_topology_asan
	./build/genesis2_topology_asan
.PHONY: native-test
native-test: | build
	$(CC) -Inative_mock -I../../include $(CFLAGS) $(SRCS) genesis_native_test.c -o build/genesis_native_test
	./build/genesis_native_test
.PHONY: edge-test
edge-test: | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRCS) genesis_edge_test.c -o build/genesis_edge_test
	./build/genesis_edge_test
