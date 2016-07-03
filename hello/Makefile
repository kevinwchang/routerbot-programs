PREFIX = mipsel-linux-
CC := $(PREFIX)gcc

TARGET = hello

all: $(TARGET)

clean:
	rm -f $(TARGET) *.o

install: $(TARGET)
	scp $(TARGET) routerbot:~/
