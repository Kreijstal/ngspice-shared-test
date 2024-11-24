# Compiler
CC ?= gcc

# Compiler Flags
CFLAGS ?= -Wall -I.

# Linker Flags
LDFLAGS ?=

# Libraries
LDLIBS = -lngspice -lSDL2_gfx -lm
SDL_LIBS = `sdl2-config --cflags --libs`

# Source files for simulation
SIM_SRCS = simulation.c simulation_impl.c
SIM_OBJS = $(SIM_SRCS:.c=.o)

# Targets
all: simulation plot

simulation: $(SIM_OBJS)
	$(CC) $(CFLAGS) $(SIM_OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

%.o: %.c simulation.h
	$(CC) $(CFLAGS) -c $< -o $@

plot: plot.c main.c
	$(CC) $(CFLAGS) $^ -o $@ $(SDL_LIBS) $(LDLIBS)

clean:
	rm -f simulation plot *.o
