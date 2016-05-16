INC_DIR= /usr/include/uarm
CFLAGS= -c -I$(INC_DIR)
all: p1test.elf.core.uarm p1test.elf.stab.uarm

p1test.elf.core.uarm p1test.elf.stab.uarm: p1test.elf
	elf2uarm -k p1test.elf

p1test.elf: pcb.o asl.o p1test.o
	arm-none-eabi-ld -T $(INC_DIR)/ldscripts/elf32ltsarm.h.uarmcore.x -o p1test.elf $(INC_DIR)/crtso.o $(INC_DIR)/libuarm.o p1test.o pcb.o asl.o

pcb.o: pcb.c
	arm-none-eabi-gcc -mcpu=arm7tdmi -c pcb.c

asl.o: asl.c
	arm-none-eabi-gcc -mcpu=arm7tdmi -c asl.c

p1test.o: p1test.c
	arm-none-eabi-gcc -mcpu=arm7tdmi $(CFLAGS) p1test.c

.PHONY: clean

clean:
	rm *.o *.elf *.uarm

