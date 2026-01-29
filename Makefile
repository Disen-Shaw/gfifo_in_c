
TARGET_GFIFO = gfifo
TARGET_SFIFO = sfifo

CC = gcc

INC_DIR = inc
BUILD_DIR = build

CFLAGS = \
$(patsubst %,-I%,$(INC_DIR)) \
-O0 \
-g

GFIFO_SOURCE = demo_gfifo.c
SFIFO_SOURCE = demo_sfifo.c

vpath %.c demo/

$(BUILD_DIR):
	mkdir -p $@

.PHONY: all clean run_gfifo run_sfifo

all: $(BUILD_DIR) \
     $(BUILD_DIR)/$(TARGET_GFIFO) \
     $(BUILD_DIR)/$(TARGET_SFIFO)

$(BUILD_DIR)/$(TARGET_GFIFO): $(GFIFO_SOURCE) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET_SFIFO): $(SFIFO_SOURCE) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

run_gfifo: $(BUILD_DIR)/$(TARGET_GFIFO)
	$(BUILD_DIR)/$(TARGET_GFIFO)

run_sfifo: $(BUILD_DIR)/$(TARGET_SFIFO)
	$(BUILD_DIR)/$(TARGET_SFIFO)

clean:
	rm -rf $(BUILD_DIR)
