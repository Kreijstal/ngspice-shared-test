CC ?= gcc
CFLAGS ?=
LDFLAGS ?=
LDLIBS = -lngspice -lSDL2_gfx -lm

simulation: simulation.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LDLIBS)

plot: plot.c
	$(CC) $(CFLAGS) $< -o $@ `sdl2-config --cflags --libs` $(LDLIBS)
