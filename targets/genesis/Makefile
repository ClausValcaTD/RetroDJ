# RetroDJ - Sega Genesis Target Makefile (SGDK)
# GPLv3 - RetroDJ Project

#
# Copyright (C) 2025 RetroDJ Project
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should receive a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

GDK ?= /sgdk
BIN := $(GDK)/bin
LIB := $(GDK)/lib
INC := $(GDK)/inc

CC := $(BIN)/m68k-elf-gcc
LD := $(BIN)/m68k-elf-gcc
OBJCOPY := $(BIN)/m68k-elf-objcopy

SRC_CS := $(wildcard src/*.c) \
          $(wildcard src/**/*.c) \
          $(wildcard ../../core/*.c) \
          $(wildcard ../../drivers/ym2612/*.c) \
          ../../backends/backend_genesis_hw.c

INCS := -Iinc -I$(INC) -Isrc -I../../core -I../../drivers/ym2612 -I../../backends

FLAGS := -m68000 -Wall -Wextra -O2 -fno-builtin -fomit-frame-pointer -DRDJ_GENESIS_HARDWARE
CFLAGS := $(FLAGS) $(INCS)

OBJS := $(SRC_CS:.c=.o)
OUT_DIR := out
TARGET := $(OUT_DIR)/rom.md

.PHONY: all clean

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	mkdir -p $(OUT_DIR)
	$(LD) -T $(GDK)/md.ld -nostdlib $(OBJS) $(LIB)/libmd.a -o $(OUT_DIR)/out.elf
	$(OBJCOPY) -O binary $(OUT_DIR)/out.elf $(TARGET)

clean:
	rm -f $(OBJS) $(OUT_DIR)/out.elf $(TARGET)
