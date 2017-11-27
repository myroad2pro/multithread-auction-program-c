#define the C compiler to use
CC = gcc

# define any conpile-time flags
CFLAGS = -g -Wall -pthread

#define the C source files
SRCS = server.c app.c

#define the C object files
OBJS = $(SRCS:.c=.o)

#define the executable files
MAIN = server

all: $(MAIN)
	@echo server has been compiled

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.o $(MAIN)