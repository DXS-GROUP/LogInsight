CC=gcc
CFLAGS=-Wall -Wextra -g -pthread

SRC_DIR=src
OBJ_DIR=obj

SOURCES=$(wildcard $(SRC_DIR)/*.c)
OBJECTS=$(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

VERSION=1.0.1

RED=\033[0;31m
GREEN=\033[0;32m
YELLOW=\033[1;33m
BLUE=\033[0;34m
RESET=\033[0m




all: $(OBJECTS)
	@mkdir -p $(OBJ_DIR) # Ensure obj directory exists
	$(CC) $(CFLAGS) -o LogInsight $(OBJECTS)
	@echo -e ""
	@echo -e ""
	@echo -e "${GREEN}"
	@echo -e "▄▄▌         ▄▄ • ▪   ▐ ▄ .▄▄ · ▪   ▄▄ •  ▄ .▄▄▄▄▄▄"
	@echo -e "██•  ▪     ▐█ ▀ ▪██ •█▌▐█▐█ ▀. ██ ▐█ ▀ ▪██▪▐█•██  "
	@echo -e "██▪   ▄█▀▄ ▄█ ▀█▄▐█·▐█▐▐▌▄▀▀▀█▄▐█·▄█ ▀█▄██▀▐█ ▐█.▪"
	@echo -e "▐█▌▐▌▐█▌.▐▌▐█▄▪▐█▐█▌██▐█▌▐█▄▪▐█▐█▌▐█▄▪▐███▌▐▀ ▐█▌·"
	@echo -e ".▀▀▀  ▀█▄▀▪·▀▀▀▀ ▀▀▀▀▀ █▪ ▀▀▀▀ ▀▀▀·▀▀▀▀ ▀▀▀ · ▀▀▀ "
	@echo -e ""
	@echo -e "${YELLOW}                 LogInsight v$(VERSION)"
	@echo -e "${YELLOW}            💫 Created by Nighty3098"
	@echo -e "${YELLOW}          https://nighty3098.github.io/"
	@echo -e ""
	@echo -e ""

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR) # Ensure obj directory exists
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/*.o LogInsight
	rm -rf $(OBJ_DIR)

run: all
	./LogInsight

help:
	@echo -e "${RED}Makefile for LogInsight${RESET}"
	@echo -e "${YELLOW}Usage:"
	@echo -e "  make        Build the project"
	@echo -e "  make run    Build and run the project"
	@echo -e "  make clean  Remove object files and the executable"
	@echo -e "  make help   Show this help message${RESET}"

.PHONY: all clean run help
