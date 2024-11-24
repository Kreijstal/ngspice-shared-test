#include "plot.h"

void drawLine(SDL_Renderer *renderer, int x1, int* y1, int x2, int* y2, PlotConfig* config) {
    for (int s = 0; s < config->num_signals; s++) {
        lineRGBA(renderer, x1, y1[s], x2, y2[s],
                 config->colors[s].r, config->colors[s].g,
                 config->colors[s].b, config->colors[s].a);
    }
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
    float relative_x = (float)(x - slider->x);
    slider->value = slider->min_value + 
                   (relative_x * (slider->max_value - slider->min_value)) / (float)slider->width;
    if (slider->value < slider->min_value) slider->value = slider->min_value;
    if (slider->value > slider->max_value) slider->value = slider->max_value;
}

PlotConfig setup_config() {
    PlotConfig config = {
        .num_signals = 0,  // Will be set when first data arrives
        .time_increment = 0.1,
        .window_width = 640,
        .window_height = 480,
        .center_y = 240,
        .amplitude = 100,
        .colors = {
            {255, 255, 0, 255},   // Yellow
            {255, 0, 0, 255},     // Red
            {0, 255, 0, 255},     // Green
            {0, 0, 255, 255},     // Blue
            {255, 0, 255, 255},   // Magenta
            {0, 255, 255, 255},   // Cyan
            {255, 128, 0, 255},   // Orange
            {128, 0, 255, 255},   // Purple
            {0, 255, 128, 255},   // Spring Green
            {255, 255, 255, 255}, // White
            {128, 128, 255, 255}, // Light Blue
            {255, 128, 128, 255}, // Light Red
            {128, 255, 128, 255}, // Light Green
            {255, 128, 255, 255}, // Light Magenta
            {192, 192, 192, 255}  // Light Gray
        },
        .amplitude_slider = {
            .x = 50,
            .y = 20,
            .width = 200,
            .height = 20,
            .value = 0.0f,
            .min_value = -1.0f,
            .max_value = 1.0f,
            .dragging = false
        },
        .tick_counter = 0
    };
    return config;
}

SDL_Window* init_sdl(PlotConfig* config) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL konnte nicht initialisiert werden! SDL Fehler: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_Window* window = SDL_CreateWindow("Red Circle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                        config->window_width, config->window_height, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Fenster konnte nicht erstellt werden! SDL Fehler: %s\n", SDL_GetError());
        SDL_Quit();
    }
    return window;
}

SDL_Renderer* create_renderer(SDL_Window* window) {
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer konnte nicht erstellt werden! SDL Fehler: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
    return renderer;
}

double** init_buffers(PlotConfig* config) {
    double** buffers = malloc(config->num_signals * sizeof(double*));
    for (int j = 0; j < config->num_signals; j++) {
        buffers[j] = malloc(BUFFER_SIZE * sizeof(double));
        for (int i = 0; i < BUFFER_SIZE; i++) {
            buffers[j][i] = 0.0;
        }
    }
    return buffers;
}

void draw_grid(SDL_Renderer* renderer, PlotConfig* config) {
    hlineRGBA(renderer, 0, 639, 479, 255, 255, 255, 255);
    vlineRGBA(renderer, 0, 0, 479, 255, 255, 255, 255);
    
    for (int x = -config->tick_counter; x < BUFFER_SIZE; x += 50) {
        if (x >= 0) {
            vlineRGBA(renderer, x, 474, 479, 255, 255, 255, 255);
        }
    }
    for (int y = 0; y < 480; y += 50) {
        hlineRGBA(renderer, 0, 5, y, 255, 255, 255, 255);
    }
}

void draw_signals(SDL_Renderer* renderer, double** buffers, PlotConfig* config, int useInterpolation) {
    if (!buffers) return;  // Safety check
    // Calculate decimation factor - how many samples to skip per pixel
    int decimation = (BUFFER_SIZE + config->window_width - 1) / config->window_width;
    if (decimation < 1) decimation = 1;
    
    if (useInterpolation) {
        for (int x = 0; x < config->window_width - 1; x++) {
            int buffer_idx = x * decimation;
            int next_buffer_idx = (x + 1) * decimation;
            
            if (next_buffer_idx >= BUFFER_SIZE) break;
            
            int* y1 = malloc(config->num_signals * sizeof(int));
            int* y2 = malloc(config->num_signals * sizeof(int));
            
            for (int s = 0; s < config->num_signals; s++) {
                y1[s] = config->center_y - (int)(buffers[s][buffer_idx] * config->amplitude);
                y2[s] = config->center_y - (int)(buffers[s][next_buffer_idx] * config->amplitude);
            }
            
            drawLine(renderer, x, y1, x + 1, y2, config);
            free(y1);
            free(y2);
        }
    } else {
        for (int x = 0; x < config->window_width; x++) {
            int buffer_idx = x * decimation;
            if (buffer_idx >= BUFFER_SIZE) break;
            
            for (int s = 0; s < config->num_signals; s++) {
                int y = config->center_y - (int)(buffers[s][buffer_idx] * config->amplitude);
                pixelRGBA(renderer, x, y,
                         config->colors[s].r, config->colors[s].g, 
                         config->colors[s].b, config->colors[s].a);
            }
        }
    }
}

void handle_events(SDL_Event* e, PlotConfig* config, int* quit, int* useInterpolation) {
    if (e->type == SDL_QUIT) {
        *quit = 1;
    } else if (e->type == SDL_KEYDOWN) {
        if (e->key.keysym.sym == SDLK_i) {
            *useInterpolation = !(*useInterpolation);
        }
    } else if (e->type == SDL_MOUSEBUTTONDOWN) {
        if (is_point_in_slider(&config->amplitude_slider, e->button.x, e->button.y)) {
            config->amplitude_slider.dragging = true;
            update_slider_value(&config->amplitude_slider, e->button.x);
        }
    } else if (e->type == SDL_MOUSEBUTTONUP) {
        config->amplitude_slider.dragging = false;
    } else if (e->type == SDL_MOUSEMOTION) {
        if (config->amplitude_slider.dragging) {
            update_slider_value(&config->amplitude_slider, e->motion.x);
        }
    }
}

void cleanup(SDL_Renderer* renderer, SDL_Window* window, double** buffers, PlotConfig* config) {
    for (int i = 0; i < config->num_signals; i++) {
        free(buffers[i]);
    }
    free(buffers);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void update_buffers(double** buffers, SignalValues values, PlotConfig* config) {
    if (!buffers || config->num_signals == 0) return;  // Safety check
    for (int i = 0; i < config->num_signals; i++) {
        memmove(buffers[i], buffers[i] + 1, (BUFFER_SIZE - 1) * sizeof(double));
        buffers[i][BUFFER_SIZE - 1] = values.values[i];
    }
    config->tick_counter = (config->tick_counter + 1) % 50;
}

