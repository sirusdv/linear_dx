UNAME := $(shell uname)

PASM := $(shell which pasm)

CC = g++
CFLAGS=-Wall -O3 -D$(UNAME)

CFLAGS_Linux=-Wno-unused-result -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast

LD = gcc
LDFLAGS_Linux=-L/lib/arm-linux-gnueabihf 

TARGET=tx

PAL=pal/$(UNAME)

SRCDIR=src
OBJDIR=obj
BINDIR=bin

LIBS_Linux=-lrt -lprussdrv

LIBS=-lpthread -lm

COMMON_SRC := $(wildcard $(SRCDIR)/*.c)
COMMON_INC := $(wildcard $(SRCDIR)/*.h)
COMMON_OBJ := $(COMMON_SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

PAL_SRC := $(wildcard $(SRCDIR)/$(PAL)/*.c)
PAL_INC := $(wildcard $(SRCDIR)/$(PAL)/*.h)
PAL_OBJ := $(PAL_SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)


.PHONY: all clean

all: $(BINDIR)/$(TARGET)

$(BINDIR)/$(TARGET): $(COMMON_OBJ) $(EFFECT_OBJ) $(PAL_OBJ)
	$(LD) $(LDFLAGS) $(LDFLAGS_$(UNAME)) -o $@ $(COMMON_OBJ) $(PAL_OBJ) $(LIBS) $(LIBS_$(UNAME))

ifneq ($(strip $(PASM)),)

all: | $(BINDIR)/firmware.bin

$(BINDIR)/firmware.bin: $(SRCDIR)/$(PAL)/firmware.p
	$(PASM) -V3 -b $< $(basename $@)

endif

$(COMMON_OBJ): | $(OBJDIR)
$(PAL_OBJ):    | $(OBJDIR)/$(PAL)

$(OBJDIR):
	@mkdir -p $@

$(OBJDIR)/$(PAL):
	@mkdir -p $@

$(BINDIR)/$(TARGET) : | $(BINDIR)

$(BINDIR):
	@mkdir -p $@

$(COMMON_OBJ): $(OBJDIR)/%.o : $(SRCDIR)/%.c $(COMMON_INC) 
	$(CC) $(CFLAGS) -c -o $@ $<

$(EFFECT_OBJ): $(OBJDIR)/%.o : $(SRCDIR)/%.c $(COMMON_INC) 
	$(CC) $(CFLAGS) -I$(SRCDIR) -c -o $@ $<

$(PAL_OBJ): $(OBJDIR)/%.o : $(SRCDIR)/%.c $(COMMON_INC) 
	$(CC) $(CFLAGS) $(CFLAGS_$(UNAME)) -I$(SRCDIR) -I$(SRCDIR)/$(PAL) -c -o $@ $<

clean:
	rm -rf $(OBJDIR) $(BINDIR)
