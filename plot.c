#include "plot.h"

void drawLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2, int y_cos1, int y_cos2) {
    lineRGBA(renderer, x1, y1, x2, y2, 255, 255, 0, 255); // Yellow for sine
    lineRGBA(renderer, x1, y_cos1, x2, y_cos2, 255, 0, 0, 255); // Red for cosine
}

void draw_slider(SDL_Renderer* renderer, Slider* slider) {
    // Draw slider track
    boxRGBA(renderer, slider->x, slider->y + slider->height/2 - 2,
            slider->x + slider->width, slider->y + slider->height/2 + 2,
            100, 100, 100, 255);
    
    // Draw slider handle
    int handle_pos = slider->x + (slider->value - slider->min_value) * slider->width / 
                     (slider->max_value - slider->min_value);
    boxRGBA(renderer, handle_pos - 5, slider->y,
            handle_pos + 5, slider->y + slider->height,
            200, 200, 200, 255);
}

bool is_point_in_slider(Slider* slider, int x, int y) {
    return x >= slider->x && x <= slider->x + slider->width &&
           y >= slider->y && y <= slider->y + slider->height;
}

void update_slider_value(Slider* slider, int x) {
    int relative_x = x - slider->x;
    slider->value = slider->min_value + 
                   (relative_x * (slider->max_value - slider->min_value)) / slider->width;
    if (slider->value < slider->min_value) slider->value = slider->min_value;
    if (slider->value > slider->max_value) slider->value = slider->max_value;
}

PlotConfig setup_config() {
    PlotConfig config = {
        .time_increment = 0.1,
        .window_width = 640,
        .window_height = 480,
        .center_y = 240,
        .amplitude = 100,
        .colors = {
            {255, 255, 0, 255},  // Yellow for first signal
            {255, 0, 0, 255}     // Red for second signal
        },
        .delay_ms = 100,
        .amplitude_slider = {
            .x = 50,
            .y = 20,
            .width = 200,
            .height = 20,
            .value = 100,
            .min_value = 10,
            .max_value = 200,
            .dragging = false
        },
        .tick_counter = 0
    };
    return config;
}

SignalValues get_new_values(double t, PlotConfig* config) {
    SignalValues values;
    values.values[0] = sin(t) * (config->amplitude_slider.value / 100.0);  // First signal: sine with slider amplitude
    values.values[1] = cos(t);  // Second signal: cosine
    return values;
}

void update_buffers(double** buffers, SignalValues values, PlotConfig* config) {
    for (int i = 0; i < NUM_SIGNALS; i++) {
        memmove(buffers[i], buffers[i] + 1, (BUFFER_SIZE - 1) * sizeof(double));
        buffers[i][BUFFER_SIZE - 1] = values.values[i];
    }
    config->tick_counter = (config->tick_counter + 1) % 50;
}

