#include <SDL2/SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 640

void drawLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2) {
    lineRGBA(renderer, x1, y1, x2, y2, 255, 0, 0, 255); // Red for sine
    lineRGBA(renderer, x1, y1 + (y2 - y1), x2, y2 + (y2 - y1), 255, 255, 0, 255); // Yellow for cosine
}

void update_values(double* sin_buffer, double* cos_buffer, double new_val) {
    memmove(sin_buffer, sin_buffer + 1, (BUFFER_SIZE - 1) * sizeof(double));
    memmove(cos_buffer, cos_buffer + 1, (BUFFER_SIZE - 1) * sizeof(double));
    sin_buffer[BUFFER_SIZE - 1] = sin(new_val);
    cos_buffer[BUFFER_SIZE - 1] = cos(new_val);
}

int main(int argc, char* argv[]) {
    double sin_buffer[BUFFER_SIZE];
    double cos_buffer[BUFFER_SIZE];

    for (int i = 0; i < BUFFER_SIZE; i++) {
        sin_buffer[i] = sin(i * 0.1);
        cos_buffer[i] = cos(i * 0.1);
    }
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL konnte nicht initialisiert werden! SDL Fehler: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Red Circle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
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

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (int x = 0; x < BUFFER_SIZE - 1; x++) {
            int y_sin1 = 240 + (int)(sin_buffer[x] * 100);
            int y_cos1 = 240 + (int)(cos_buffer[x] * 100);
            int y_sin2 = 240 + (int)(sin_buffer[x + 1] * 100);
            int y_cos2 = 240 + (int)(cos_buffer[x + 1] * 100);
            drawLine(renderer, x, y_sin1, x + 1, y_sin2);
            drawLine(renderer, x, y_cos1, x + 1, y_cos2);
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
        update_values(sin_buffer, cos_buffer, t);
        t += 0.1;
        SDL_Delay(100);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
