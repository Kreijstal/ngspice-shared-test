#include <SDL2/SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <stdio.h>
#include <math.h>

int main(int argc, char* argv[]) {
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

    for (int x = 0; x < 640; x++) {
        int y_sin = 240 + (int)(sin(x * 0.01) * 100); // Sine wave in red
        int y_cos = 240 + (int)(cos(x * 0.01) * 100); // Cosine wave in yellow
        pixelRGBA(renderer, x, y_sin, 255, 0, 0, 255);
        pixelRGBA(renderer, x, y_cos, 255, 255, 0, 255);
    }

    SDL_RenderPresent(renderer);

    SDL_Delay(5000); // Warte 5 Sekunden

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
