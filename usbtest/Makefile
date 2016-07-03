PREFIX = mipsel-linux-
CC := $(PREFIX)gcc
CFLAGS = --std=c99
LDFLAGS = -lusb-1.0

TARGET = usbtest

all: $(TARGET)

clean:
	rm -f $(TARGET) *.o

install: $(TARGET)
	scp $(TARGET) routerbot:~/ && touch install

run: install
	ssh routerbot ./$(TARGET)

