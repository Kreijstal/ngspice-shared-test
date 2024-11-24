#ifndef PLOT_H
#define PLOT_H

#include <SDL2/SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 128000

typedef struct {
    int x;
    int y;
    int width;
    int height;
    int value;
    int min_value;
    int max_value;
    bool dragging;
} Slider;

typedef struct {
    double time_increment;
    int window_width;
    int window_height;
    int center_y;
    int amplitude;
    int num_signals;
    SDL_Color colors[15];  // Basic color palette for plotting
    Slider amplitude_slider;
    int tick_counter;
} PlotConfig;

typedef struct {
    double values[15];  // Fixed size matching colors array
} SignalValues;

void drawLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2, int y_cos1, int y_cos2);
void draw_slider(SDL_Renderer* renderer, Slider* slider);
bool is_point_in_slider(Slider* slider, int x, int y);
void update_slider_value(Slider* slider, int x);
PlotConfig setup_config(void);
void update_buffers(double** buffers, SignalValues values, PlotConfig* config);

// SDL initialization functions
SDL_Window* init_sdl(PlotConfig* config);
SDL_Renderer* create_renderer(SDL_Window* window);
double** init_buffers(PlotConfig* config);

// Drawing functions
void draw_grid(SDL_Renderer* renderer, PlotConfig* config);
void draw_signals(SDL_Renderer* renderer, double** buffers, PlotConfig* config, int useInterpolation);
void handle_events(SDL_Event* e, PlotConfig* config, int* quit, int* useInterpolation);
void cleanup(SDL_Renderer* renderer, SDL_Window* window, double** buffers, PlotConfig* config);

#endif // PLOT_H
