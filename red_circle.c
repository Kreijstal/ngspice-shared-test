#include <SDL2/SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 640
#define NUM_SIGNALS 2

typedef struct {
    double time_increment;
    int window_width;
    int window_height;
    int center_y;
    int amplitude;
    SDL_Color colors[NUM_SIGNALS];
    int delay_ms;
} PlotConfig;

typedef struct {
    double values[NUM_SIGNALS];
} SignalValues;

void drawLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2, int y_cos1, int y_cos2) {
    lineRGBA(renderer, x1, y1, x2, y2, 255, 255, 0, 255); // Yellow for sine
    lineRGBA(renderer, x1, y_cos1, x2, y_cos2, 255, 0, 0, 255); // Red for cosine
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
        .delay_ms = 100
    };
    return config;
}

SignalValues get_new_values(double t) {
    SignalValues values;
    values.values[0] = sin(t);  // First signal: sine
    values.values[1] = cos(t);  // Second signal: cosine
    return values;
}

void update_buffers(double** buffers, SignalValues new_vals) {
    for (int i = 0; i < NUM_SIGNALS; i++) {
        memmove(buffers[i], buffers[i] + 1, (BUFFER_SIZE - 1) * sizeof(double));
        buffers[i][BUFFER_SIZE - 1] = new_vals.values[i];
    }
}

int main(int argc, char* argv[]) {
    PlotConfig config = setup_config();
    
    // Allocate buffers for all signals
    double* buffers[NUM_SIGNALS];
    for (int j = 0; j < NUM_SIGNALS; j++) {
        buffers[j] = malloc(BUFFER_SIZE * sizeof(double));
        for (int i = 0; i < BUFFER_SIZE; i++) {
            SignalValues vals = get_new_values(i * config.time_increment);
            buffers[j][i] = vals.values[j];
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
    double t = BUFFER_SIZE * 0.1;
    int useInterpolation = 1;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_i) {
                    useInterpolation = !useInterpolation;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (useInterpolation) {
            for (int x = 0; x < BUFFER_SIZE - 1; x++) {
                int y1[NUM_SIGNALS], y2[NUM_SIGNALS];
                for (int s = 0; s < NUM_SIGNALS; s++) {
                    y1[s] = config.center_y + (int)(buffers[s][x] * config.amplitude);
                    y2[s] = config.center_y + (int)(buffers[s][x + 1] * config.amplitude);
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
        int tick_offset = (int)(t * 10) % 50;

        hlineRGBA(renderer, 0, 639, 479, 255, 255, 255, 255);
        vlineRGBA(renderer, 0, 0, 479, 255, 255, 255, 255);
        for (int x = -tick_offset; x < BUFFER_SIZE; x += 50) {
            if (x >= 0)
                vlineRGBA(renderer, x, 474, 479, 255, 255, 255, 255);
        }
        for (int y = 0; y < 480; y += 50) {
            hlineRGBA(renderer, 0, 5, y, 255, 255, 255, 255);
        }

        SDL_RenderPresent(renderer);
        SignalValues new_vals = get_new_values(t);
        update_buffers(buffers, new_vals);
        t += config.time_increment;
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
