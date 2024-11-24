#include "plot.h"

SignalValues get_new_values(double t, PlotConfig* config) {
    SignalValues values;
    values.values[0] = sin(t) * (config->amplitude_slider.value / 100.0);  // First signal: sine with slider amplitude
    values.values[1] = cos(t);  // Second signal: cosine
    return values;
}

int main(int argc, char* argv[]) {
    PlotConfig config = setup_config();
    double** buffers = init_buffers(&config);
    
    SDL_Window* window = init_sdl(&config);
    if (!window) return 1;

    SDL_Renderer* renderer = create_renderer(window);
    if (!renderer) return 1;

    int quit = 0;
    SDL_Event e;
    int useInterpolation = 1;
    double t = 0.0;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            handle_events(&e, &config, &quit, &useInterpolation);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        draw_signals(renderer, buffers, &config, useInterpolation);
        draw_grid(renderer, &config);
        draw_slider(renderer, &config.amplitude_slider);
        
        SignalValues new_values = get_new_values(t, &config);
        update_buffers(buffers, new_values, &config);
        t += config.time_increment;

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // Cap at roughly 60 FPS
    }

    cleanup(renderer, window, buffers, &config);
    return 0;
}
