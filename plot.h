#ifndef PLOT_H
#define PLOT_H

#include <SDL2/SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 640
#define NUM_SIGNALS 2

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
    SDL_Color colors[NUM_SIGNALS];
    int delay_ms;
    Slider amplitude_slider;
    int tick_counter;
} PlotConfig;

typedef struct {
    double values[NUM_SIGNALS];
} SignalValues;

void drawLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2, int y_cos1, int y_cos2);
void draw_slider(SDL_Renderer* renderer, Slider* slider);
bool is_point_in_slider(Slider* slider, int x, int y);
void update_slider_value(Slider* slider, int x);
PlotConfig setup_config(void);
SignalValues get_new_values(double t, PlotConfig* config);
void update_buffers(double** buffers, SignalValues values, PlotConfig* config);

#endif // PLOT_H
