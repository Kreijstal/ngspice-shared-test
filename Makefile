# Compiler settings
CC ?= gcc
CFLAGS ?= -Wall -I.
LDFLAGS ?=

# Check for required packages
SDL2_GFX_LIBS := $(shell pkg-config --libs SDL2_gfx)
SDL2_GFX_CFLAGS := $(shell pkg-config --cflags SDL2_gfx)

# Libraries
LIBS = -lngspice $(SDL2_GFX_LIBS) -lm `sdl2-config --cflags --libs`
CFLAGS += $(SDL2_GFX_CFLAGS)

# Source files
SRCS = main.c plot.c simulation.c simulation_impl.c
OBJS = $(SRCS:.c=.o)

# Target
all: check_deps simulation_plot

check_deps:
	@pkg-config --exists SDL2_gfx || (echo "SDL2_gfx not found. Please install libsdl2-gfx-dev" && exit 1)

simulation_plot: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f simulation_plot *.o
