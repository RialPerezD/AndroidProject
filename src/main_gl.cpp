#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "include/renderer.h"
#include <algorithm>

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_Window* window = SDL_CreateWindow("Android Renderer", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN);
    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    Renderer renderer;
    renderer.init();

    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    float ammount = 16.0f;
    float minDim = (float)std::min(w, h);
    float cellSizePx = minDim / ammount;

    float cubeWidthNDC = (cellSizePx / (float)w) * 2.0f;
    float cubeHeightNDC = (cellSizePx / (float)h) * 2.0f;

    std::vector<Sprite> listaSprites;
    GLuint cube = renderer.loadTexture("knight.bmp");

    listaSprites.clear();

    for (int row = 0; row < ammount; ++row) {
        for (int col = 0; col < ammount; ++col) {
            float posX = ((col - (ammount / 2.0f)+0.5f) * cellSizePx) / ((float)w / 2.0f);
            float posY = -((row - (ammount / 2.0f)+0.5f) * cellSizePx) / ((float)h / 2.0f);

            listaSprites.push_back({posX, posY, cubeWidthNDC, cubeHeightNDC, cube});
        }
    }

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;
        }

        renderer.draw(listaSprites, w, h);

        SDL_GL_SwapWindow(window);
    }

    renderer.cleanup();
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}