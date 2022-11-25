CC = gcc
OBJS = main.o optimal.o fifo.o lifo.o lru.o lfu.o sc.o esc.o makePage.o user.o
TARGET = main.out

all : $(TARGET)

$(TARGET) : $(OBJS)
		$(CC) -o $@ $^

new:
		$(MAKE) clean
		$(MAKE) all

clean:
		rm -f *.o main.out
