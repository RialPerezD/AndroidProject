#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "include/renderer.h"
#include "include/grid.h"
#include <algorithm>
#include <cmath>
#include <vector>

struct TouchState {
    float startX, startY;
    bool isPressed = false;
    const float minSwipeDist = 0.15f;
};

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_Window* window = SDL_CreateWindow("SDL3 Maze Game", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    Renderer renderer;
    renderer.init();

    Grid gameGrid;
    int currentLevel = 1;

    std::vector<Sprite> listaSprites;
    GLuint knight = renderer.loadTexture("knight.bmp");
    GLuint cube = renderer.loadTexture("cube.bmp");

    TouchState touch;
    bool needsSpriteUpdate = true;
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;

            if (event.type == SDL_EVENT_FINGER_DOWN) {
                touch.startX = event.tfinger.x;
                touch.startY = event.tfinger.y;
                touch.isPressed = true;
            }
            else if (event.type == SDL_EVENT_FINGER_UP && touch.isPressed) {
                float dx_f = event.tfinger.x - touch.startX;
                float dy_f = event.tfinger.y - touch.startY;
                touch.isPressed = false;

                int dx = 0, dy = 0;
                if (std::abs(dx_f) > touch.minSwipeDist || std::abs(dy_f) > touch.minSwipeDist) {
                    if (std::abs(dx_f) > std::abs(dy_f)) {
                        dx = (dx_f > 0) ? 1 : -1;
                    } else {
                        dy = (dy_f > 0) ? 1 : -1;
                    }

                    if (gameGrid.movePlayer(dx, dy)) {
                        needsSpriteUpdate = true;
                    }
                }
            }

            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key >= SDLK_1 && event.key.key <= SDLK_9) {
                    currentLevel = event.key.key - SDLK_0;
                    gameGrid.setLevel(currentLevel);
                    needsSpriteUpdate = true;
                }
            }
        }

        int w, h;
        SDL_GetWindowSize(window, &w, &h);

        if (needsSpriteUpdate) {
            float gridDim = (float)gameGrid.getSize();
            float minDim = (float)std::min(w, h);
            float cellSizePx = minDim / gridDim;
            float spriteW = (cellSizePx / (float)w) * 2.0f;
            float spriteH = (cellSizePx / (float)h) * 2.0f;

            listaSprites.clear();
            for (int row = 0; row < gameGrid.getSize(); ++row) {
                for (int col = 0; col < gameGrid.getSize(); ++col) {
                    int cellType = gameGrid.getCell(col, row);
                    if (cellType == 0) continue;

                    float posX = ((col - (gridDim / 2.0f) + 0.5f) * cellSizePx) / ((float)w / 2.0f);
                    float posY = -((row - (gridDim / 2.0f) + 0.5f) * cellSizePx) / ((float)h / 2.0f);

                    GLuint tex = (cellType == 1) ? cube : knight;
                    listaSprites.push_back({posX, posY, spriteW, spriteH, tex});
                }
            }
            needsSpriteUpdate = false;
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