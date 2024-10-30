CFLAGS = -Wall -Werror

all: build

build:
	gcc $(CFLAGS) main.c read.c keyboard.c send.c recv.c list.o sharedMutex.c -o s-talk -lpthread

clean:
	rm -f s-talk