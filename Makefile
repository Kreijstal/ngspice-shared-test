# Compiler settings
CC ?= gcc
CFLAGS ?= -Wall -I.
LDFLAGS ?=

# Project-specific settings
PLOT_LIBS = -lSDL2_gfx -lm `sdl2-config --cflags --libs`
SIM_LIBS = -lngspice -lm

# Source files
PLOT_SRCS = plot.c main_plot.c
SIM_SRCS = simulation.c simulation_impl.c main_sim.c
PLOT_OBJS = $(PLOT_SRCS:.c=.o)
SIM_OBJS = $(SIM_SRCS:.c=.o)

# Targets
all: plot_app simulation_app

plot_app: $(PLOT_OBJS)
	$(CC) $(CFLAGS) $(PLOT_OBJS) -o $@ $(LDFLAGS) $(PLOT_LIBS)

simulation_app: $(SIM_OBJS)
	$(CC) $(CFLAGS) $(SIM_OBJS) -o $@ $(LDFLAGS) $(SIM_LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f plot_app simulation_app *.o
