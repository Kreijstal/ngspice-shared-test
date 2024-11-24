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

SignalValues get_new_values(double t) {
    SignalValues values;
    values.values[0] = sin(t);  // First signal: sine
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

int main(int argc, char* argv[]) {
    PlotConfig config = setup_config();
    
    // Allocate buffers for all signals
    double* buffers[NUM_SIGNALS];
    for (int j = 0; j < NUM_SIGNALS; j++) {
        buffers[j] = malloc(BUFFER_SIZE * sizeof(double));
        for (int i = 0; i < BUFFER_SIZE; i++) {
            buffers[j][i] = 0.0;  // Initialize with zeros
        }
    }
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL konnte nicht initialisiert werden! SDL Fehler: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Red Circle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                        config.window_width, config.window_height, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Fenster konnte nicht erstellt werden! SDL Fehler: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer konnte nicht erstellt werden! SDL Fehler: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int quit = 0;
    SDL_Event e;
    int useInterpolation = 1;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_i) {
                    useInterpolation = !useInterpolation;
                }
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (is_point_in_slider(&config.amplitude_slider, e.button.x, e.button.y)) {
                    config.amplitude_slider.dragging = true;
                    update_slider_value(&config.amplitude_slider, e.button.x);
                }
            } else if (e.type == SDL_MOUSEBUTTONUP) {
                config.amplitude_slider.dragging = false;
            } else if (e.type == SDL_MOUSEMOTION) {
                if (config.amplitude_slider.dragging) {
                    update_slider_value(&config.amplitude_slider, e.motion.x);
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (useInterpolation) {
            for (int x = 0; x < BUFFER_SIZE - 1; x++) {
                int y1[NUM_SIGNALS], y2[NUM_SIGNALS];
                for (int s = 0; s < NUM_SIGNALS; s++) {
                    int amp = (s == 0) ? config.amplitude_slider.value : config.amplitude;
                    y1[s] = config.center_y + (int)(buffers[s][x] * amp);
                    y2[s] = config.center_y + (int)(buffers[s][x + 1] * amp);
                }
                drawLine(renderer, x, y1[0], x + 1, y2[0], y1[1], y2[1]);
            }
        } else {
            for (int x = 0; x < BUFFER_SIZE; x++) {
                for (int s = 0; s < NUM_SIGNALS; s++) {
                    int y = config.center_y + (int)(buffers[s][x] * config.amplitude);
                    pixelRGBA(renderer, x, y, 
                             config.colors[s].r, config.colors[s].g, 
                             config.colors[s].b, config.colors[s].a);
                }
            }
        }
        hlineRGBA(renderer, 0, 639, 479, 255, 255, 255, 255);
        vlineRGBA(renderer, 0, 0, 479, 255, 255, 255, 255);
        for (int x = -config.tick_counter; x < BUFFER_SIZE; x += 50) {
            if (x >= 0) {
                vlineRGBA(renderer, x, 474, 479, 255, 255, 255, 255);
            }
        }
        for (int y = 0; y < 480; y += 50) {
            hlineRGBA(renderer, 0, 5, y, 255, 255, 255, 255);
        }

        // Draw the amplitude slider
        draw_slider(renderer, &config.amplitude_slider);
        
        // Generate and update signal values
        static double t = 0.0;
        SignalValues new_values = get_new_values(t);
        update_buffers(buffers, new_values, &config);
        t += config.time_increment;

        SDL_RenderPresent(renderer);
        SDL_Delay(config.delay_ms);
    }

    // Clean up
    for (int i = 0; i < NUM_SIGNALS; i++) {
        free(buffers[i]);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
