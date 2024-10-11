CC=gcc
CFLAGS=-Wall -Wextra -g -pthread

SRC_DIR=src
OBJ_DIR=obj

SOURCES=$(wildcard $(SRC_DIR)/*.c)
OBJECTS=$(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

all: $(OBJECTS)
	$(CC) $(CFLAGS) -o log_monitor $(OBJECTS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/*.o log_monitor

.PHONY: all clean
