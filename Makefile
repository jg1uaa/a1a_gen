TARGET = a1a_gen
OBJ = table.o player.o output.o main.o
CFLAGS = -O2 -Wall -c -fdata-sections -ffunction-sections
LFLAGS = -Wl,--gc-sections
LDLIBS = -lm

all: $(TARGET)

table.o: table.c
	$(CC) $(CFLAGS) $< -o $@

player.o: player.c
	$(CC) $(CFLAGS) $< -o $@

output.o: output.c
	$(CC) $(CFLAGS) $< -o $@

main.o: main.c
	$(CC) $(CFLAGS) $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(LFLAGS) $(OBJ) $(LDLIBS) -o $@

clean:
	rm -f $(TARGET) $(OBJ)
