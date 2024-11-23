CC ?= gcc
LDLIBS = -lngspice

simulation: simulation.c
	$(CC) $< -o $@ $(LDLIBS)
