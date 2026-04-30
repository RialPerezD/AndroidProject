#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "include/renderer.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_Window* window = SDL_CreateWindow("Android Renderer", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN);
    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    Renderer renderer;
    renderer.init();

    std::vector<Sprite> listaSprites;

    GLuint cube = renderer.loadTexture("cube.bmp");

    listaSprites.push_back({0.0f, 0.0f, 0.4f, 0.4f, cube});
    listaSprites.push_back({0.7f, 0.7f, 0.2f, 0.2f, cube});
    listaSprites.push_back({-0.5f, -0.5f, 0.6f, 0.3f, cube});

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;
            if (event.type == SDL_EVENT_FINGER_DOWN) {
            }
        }

        int w, h;
        SDL_GetWindowSize(window, &w, &h);

        renderer.draw(listaSprites, w, h);

        SDL_GL_SwapWindow(window);
    }

    return 0;
}