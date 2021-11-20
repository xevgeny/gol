CC = gcc
LIBS = -lncurses
TARGET = gol

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) -o $(TARGET) $(TARGET).c $(LIBS)

clean:
	$(RM) $(TARGET)

