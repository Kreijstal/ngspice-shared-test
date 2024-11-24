#include "plot.h"

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
    double t = 0.0;

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
        SignalValues new_values = get_new_values(t, &config);
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
