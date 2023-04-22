
CC = cc
TARGET = k1

$(TARGET): $(TARGET).c
	$(CC) -std=c11 -O2 -Wall -o $@ $^ `pkg-config gtk+-3.0 --cflags --libs`

clean:
	-rm $(TARGET) $(TARGET).c

