PROG_SRC := $(filter-out hardware.c, $(wildcard *.c))
PROGS := $(PROG_SRC:.c=.bin)

DRIVER_SRC := $(wildcard drivers/*.c)
OBJS := start.o hardware.o
OBJS += $(DRIVER_SRC:.c=.o)


MEMMAP := ./memmap

ARM     := arm-none-eabi
CC      := $(ARM)-gcc

CFLAGS := -O -Wall -Wno-unused-variable -Werror -std=gnu99 
CFLAGS += -nostdlib -nostartfiles -ffreestanding 
CFLAGS += -mcpu=arm1176jzf-s
CFLAGS += -mno-unaligned-access
CFLAGS += -I. -Idrivers

all: $(PROGS)

# main.bin: main.o $(OBJS) $(MEMMAP)
# 	$(CC) $(CFLAGS) start.o $< $(filter-out start.o, $(OBJS)) -T $(MEMMAP) -lgcc -o main.elf
# 	$(ARM)-objcopy main.elf -O binary $@
# 	$(ARM)-objdump -D main.elf > main.list
# 	pi-install main.bin

transmit_test.bin: transmit_test.o $(OBJS) $(MEMMAP)
	$(CC) $(CFLAGS) start.o $< $(filter-out start.o, $(OBJS)) -T $(MEMMAP) -lgcc -o transmit_test.elf
	$(ARM)-objcopy transmit_test.elf -O binary $@
	$(ARM)-objdump -D transmit_test.elf > transmit_test.list
	pi-install transmit_test.bin

%.bin: %.o $(OBJS) $(MEMMAP) Makefile *.h *.c drivers/*.h drivers/*.c
	$(CC) $(CFLAGS) $(OBJS) $< -T $(MEMMAP) -lgcc -o $*.elf
	$(ARM)-objcopy $*.elf -O binary $@
	$(ARM)-objdump -D $*.elf > $*.list

%.o: %.c Makefile *.h *.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S Makefile *.h *.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.bin *.list *.elf *.o Makefile.bak *~

.PHONY: all clean
