CC = gcc
EXEC = testgps
OBJS = tinygps.o testgps.o
LDFLAGS = -lm

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

%.$(o) : %.c
	$(CC) -c $< -o $@

clean:
	rm -rf *.o $(EXEC)

