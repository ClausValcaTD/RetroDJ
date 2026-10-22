# RetroDJ - Game Boy Target Makefile (GBDK-2020)
# GPLv3 - RetroDJ Project

GBDKDIR ?= /opt/gbdk
ifneq ($(lastword $(subst /, ,$(GBDKDIR))),)
    GBDKDIR := $(GBDKDIR)/
endif

LCC := $(GBDKDIR)bin/lcc

INCS := -I. -Isrc -I../../ -I../../core -I../../drivers/gb_sound -I../../backends -I../../targets/genesis/src
CFLAGS := -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -Wm-yC $(INCS)

OUT_DIR := out
TARGET_GB := $(OUT_DIR)/rom.gb
TARGET_GBC := $(OUT_DIR)/rom.gbc

C_SRCS := src/main.c \
          src/ui_gb.c \
          ../../drivers/gb_sound/gb_rdj.c \
          ../../backends/backend_gb_hw.c \
          ../../targets/genesis/src/input_mapper.c

C_OBJS := $(C_SRCS:.c=.o)

.SUFFIXES:
.SUFFIXES: .c .o

.PHONY: all clean

all: $(TARGET_GB) $(TARGET_GBC)

%.o: %.c
	GBDKDIR=$(GBDKDIR) $(LCC) $(CFLAGS) -c $< -o $@

$(TARGET_GB): $(C_OBJS)
	mkdir -p $(OUT_DIR)
	GBDKDIR=$(GBDKDIR) $(LCC) $(CFLAGS) $(C_OBJS) -o $(TARGET_GB)

$(TARGET_GBC): $(TARGET_GB)
	cp $(TARGET_GB) $(TARGET_GBC)

clean:
	rm -rf $(OUT_DIR)
	rm -f src/*.o ../../drivers/gb_sound/*.o ../../backends/*.o ../../targets/genesis/src/*.o
	rm -f src/*.lst src/*.sym ../../drivers/gb_sound/*.lst ../../backends/*.lst
