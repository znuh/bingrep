OBJS=main.o mmap.o

CFLAGS=-Wall -O3

all: bingrep

bingrep: $(OBJS)
	$(CC) $(CFLAGS) -o bingrep $(OBJS)

clean:
	rm -f bingrep $(OBJS)


