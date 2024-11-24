# Compiler settings
CC ?= gcc
CFLAGS ?= -Wall -I.
LDFLAGS ?=

# Libraries
LIBS = -lngspice -lSDL2_gfx -lm `sdl2-config --cflags --libs`

# Source files
SRCS = main.c plot.c simulation.c simulation_impl.c
OBJS = $(SRCS:.c=.o)

# Target
all: simulation_plot

simulation_plot: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f simulation_plot *.o
