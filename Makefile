CC ?= gcc
CFLAGS ?=
LDFLAGS ?=
LDLIBS = -lngspice

simulation: simulation.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LDLIBS)
