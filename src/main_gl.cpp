#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "include/renderer.h"
#include "include/grid.h"
#include <algorithm>
#include <cmath>
#include <vector>

enum GameState { MENU, PLAYING };

struct TouchState {
    float startX = 0.0f;
    float startY = 0.0f;
    bool isPressed = false;
    const float minSwipeDist = 0.15f;
};

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_Window* window = SDL_CreateWindow("Dreamland Warrior", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN);
    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    Renderer renderer;
    renderer.init();

    Grid gameGrid;
    std::vector<Sprite> listaSprites;
    GameState currentState = MENU;

    GLuint texWall   = renderer.loadTexture("cube.bmp");
    GLuint texPlayer = renderer.loadTexture("knight.bmp");
    GLuint texBox    = renderer.loadTexture("box.bmp");
    GLuint texGoblin = renderer.loadTexture("goblin.bmp");
    GLuint texLadder = renderer.loadTexture("ladder.bmp");
    GLuint texLeafs  = renderer.loadTexture("leafs.bmp");
    GLuint texMenu   = renderer.loadTexture("menu.bmp");
    GLuint texBack   = renderer.loadTexture("background.bmp");

    TouchState touch;
    bool needsSpriteUpdate = true;
    bool running = true;
    SDL_Event event;

    Uint64 lastGravityTime = SDL_GetTicks();
    const Uint64 gravityInterval = 150;

    while (running) {
        int w, h;
        SDL_GetWindowSize(window, &w, &h);

        int playerState = 0;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;
            if (event.type == SDL_EVENT_WINDOW_RESIZED) needsSpriteUpdate = true;

            if (currentState == MENU) {
                if (event.type == SDL_EVENT_FINGER_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                    float clickX, clickY;
                    if (event.type == SDL_EVENT_FINGER_DOWN) {
                        clickX = event.tfinger.x; clickY = event.tfinger.y;
                    } else {
                        clickX = event.button.x / (float)w; clickY = event.button.y / (float)h;
                    }

                    float minDim = (float)std::min(w, h);
                    float menuRectW = minDim / (float)w;
                    float menuRectH = minDim / (float)h;
                    float menuLeft = (1.0f - menuRectW) / 2.0f;
                    float menuTop = (1.0f - menuRectH) / 2.0f;

                    float localX = (clickX - menuLeft) / menuRectW;
                    float localY = (clickY - menuTop) / menuRectH;

                    if (localX > 0.34f && localX < 0.66f) {
                        if (localY > 0.67f && localY < 0.75f) {
                            currentState = PLAYING;
                            gameGrid.setLevel(1);
                            needsSpriteUpdate = true;
                        }
                        else if (localY > 0.77f && localY < 0.85f) {
                            running = false;
                        }
                    }
                }
            }
            else {
                if (event.type == SDL_EVENT_FINGER_DOWN) {
                    touch.startX = event.tfinger.x; touch.startY = event.tfinger.y;
                    touch.isPressed = true;
                }
                else if (event.type == SDL_EVENT_FINGER_UP && touch.isPressed) {
                    float dx_f = event.tfinger.x - touch.startX;
                    float dy_f = event.tfinger.y - touch.startY;
                    touch.isPressed = false;
                    int dx = 0, dy = 0;
                    if (std::abs(dx_f) > touch.minSwipeDist || std::abs(dy_f) > touch.minSwipeDist) {
                        if (std::abs(dx_f) > std::abs(dy_f)) dx = (dx_f > 0) ? 1 : -1;
                        else dy = (dy_f > 0) ? 1 : -1;
                        playerState = gameGrid.movePlayer(dx, dy);
                        if (playerState == 1) needsSpriteUpdate = true;
                    }
                }
                if (event.type == SDL_EVENT_KEY_DOWN) {
                    int dx = 0, dy = 0;
                    if (event.key.key == SDLK_UP) dy = -1;
                    if (event.key.key == SDLK_DOWN) dy = 1;
                    if (event.key.key == SDLK_LEFT) dx = -1;
                    if (event.key.key == SDLK_RIGHT) dx = 1;
                    if (event.key.key == SDLK_ESCAPE) { currentState = MENU; needsSpriteUpdate = true; }
                    playerState = gameGrid.movePlayer(dx, dy);
                    if ((dx != 0 || dy != 0) && playerState == 1) needsSpriteUpdate = true;
                }
            }
        }

        if(playerState == 2){
            currentState = MENU;
            playerState = 0;
            needsSpriteUpdate = true;
        }

        if (currentState == PLAYING) {
            Uint64 currentTime = SDL_GetTicks();
            if (currentTime - lastGravityTime > gravityInterval) {
                if (gameGrid.applyGravity()) needsSpriteUpdate = true;
                lastGravityTime = currentTime;
            }
        }

        if (needsSpriteUpdate) {
            listaSprites.clear();
            float minSide = (float)std::min(w, h);
            float squareW = (minSide / (float)w) * 2.0f;
            float squareH = (minSide / (float)h) * 2.0f;

            if (currentState == MENU) {
                listaSprites.push_back({0.0f, 0.0f, squareW, squareH, texMenu});
            }
            else {
                listaSprites.push_back({0.0f, 0.0f, squareW, squareH, texBack});

                int gridSize = gameGrid.getSize();
                float gridDim = (float)gridSize;
                float cellSizePx = minSide / gridDim;
                float spriteW = (cellSizePx / (float)w) * 2.0f;
                float spriteH = (cellSizePx / (float)h) * 2.0f;

                for (int row = 0; row < gridSize; ++row) {
                    for (int col = 0; col < gridSize; ++col) {
                        float posX = ((col - (gridDim / 2.0f) + 0.5f) * cellSizePx) / (w / 2.0f);
                        float posY = -((row - (gridDim / 2.0f) + 0.5f) * cellSizePx) / (h / 2.0f);

                        if (gameGrid.isLadder(col, row)) listaSprites.push_back({posX, posY, spriteW, spriteH, texLadder});

                        int cellType = gameGrid.getCell(col, row);
                        if (cellType != 0) {
                            GLuint tex = 0;
                            switch (cellType) {
                                case 1: tex = texWall;   break;
                                case 2: tex = texPlayer; break;
                                case 3: tex = texBox;    break;
                                case 4: tex = texGoblin; break;
                                case 6: tex = texLeafs;  break;
                            }
                            if (tex != 0) listaSprites.push_back({posX, posY, spriteW, spriteH, tex});
                        }
                    }
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