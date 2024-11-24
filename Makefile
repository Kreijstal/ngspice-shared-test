# Compiler
CC ?= gcc

# Compiler Flags
CFLAGS ?= -Wall

# Linker Flags
LDFLAGS ?=

# Libraries
LDLIBS = -lngspice -lSDL2_gfx -lm
SDL_LIBS = `sdl2-config --cflags --libs`

# Targets
simulation: simulation.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LDLIBS)

plot: plot.c main.c
	$(CC) $(CFLAGS) $^ -o $@ $(SDL_LIBS) $(LDLIBS)
