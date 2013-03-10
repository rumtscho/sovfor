CC=msp430-gcc
CFLAGS=-Os -Wall -g -mmcu=msp430g2553

OBJS=main.o panic.o interrupts.o serialcon.o ringbuf.o onewire.o delay.o
TARGET=main.elf

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

run: $(TARGET)
	mspdebug rf2500 "prog $(TARGET)" "run"
    
prog: $(TARGET)
	mspdebug rf2500 "prog $(TARGET)" "exit"
    
clean:
	rm -fr $(TARGET) $(OBJS)
