CC=msp430-gcc
CFLAGS=-Os -Wall -g -mmcu=msp430g2553

OBJS=main.o
TARGET=main.elf

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

run: $(TARGET)
	mspdebug rf2500 "prog $(TARGET)" "run"
    
clean:
	rm -fr $(TARGET) $(OBJS)
