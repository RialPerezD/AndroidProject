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

struct Joystick {
    float centerX = 0.65f;
    float centerY = -0.6f;
    float radius = 0.2f;
    float knobX = 0.0f;
    float knobY = 0.0f;
    bool active = false;
    Uint64 lastMoveTime = 0;
    const Uint64 moveDelay = 200;

    void updateKnob(float touchX, float touchY, float bgW, float bgH) {
        float dx = touchX - centerX;
        float dy = touchY - centerY;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist > radius) {
            knobX = (dx / dist) * radius;
            knobY = (dy / dist) * radius;
        } else {
            knobX = dx;
            knobY = dy;
        }
    }

    void reset() {
        knobX = 0.0f;
        knobY = 0.0f;
        active = false;
    }
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
    GLuint texReset  = renderer.loadTexture("reset.bmp");
    GLuint texJoyBase = renderer.loadTexture("joy-outer.bmp");
    GLuint texJoyKnob = renderer.loadTexture("joy-inner.bmp");

    TouchState touch;
    Joystick joy;
    bool needsSpriteUpdate = true;
    bool running = true;
    SDL_Event event;

    Uint64 lastGravityTime = SDL_GetTicks();
    const Uint64 gravityInterval = 150;

    while (running) {
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        float minSide = (float)std::min(w, h);

        float bgW = (minSide / (float)w) * 2.0f;
        float bgH = (minSide / (float)h) * 2.0f;
        float bgRight = bgW / 2.0f;
        float bgTop = bgH / 2.0f;

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

                    float menuLeft = (1.0f - (minSide / w)) / 2.0f;
                    float menuTopPos = (1.0f - (minSide / h)) / 2.0f;

                    float localX = (clickX - menuLeft) / (minSide / w);
                    float localY = (clickY - menuTopPos) / (minSide / h);

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
                if (event.type == SDL_EVENT_FINGER_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
                    event.type == SDL_EVENT_FINGER_MOTION || event.type == SDL_EVENT_MOUSE_MOTION) {

                    float clickX, clickY;
                    bool isDown = (event.type == SDL_EVENT_FINGER_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_DOWN);

                    if (event.type == SDL_EVENT_FINGER_DOWN || event.type == SDL_EVENT_FINGER_MOTION) {
                        clickX = (event.tfinger.x * 2.0f) - 1.0f;
                        clickY = 1.0f - (event.tfinger.y * 2.0f);
                    } else {
                        clickX = (event.button.x / (float)w * 2.0f) - 1.0f;
                        clickY = 1.0f - (event.button.y / (float)h * 2.0f);
                    }

                    float distToJoy = std::sqrt(std::pow(clickX - joy.centerX, 2) + std::pow(clickY - joy.centerY, 2));

                    if (isDown && distToJoy < joy.radius * 2.0f) {
                        joy.active = true;
                    }

                    if (joy.active) {
                        joy.updateKnob(clickX, clickY, bgW, bgH);
                        needsSpriteUpdate = true;
                    } else if (isDown) {
                        float iconSize = 0.15f;
                        float iconW = (iconSize * minSide) / w;
                        float iconH = (iconSize * minSide) / h;
                        float resetX = bgRight - (iconW / 2.0f);
                        float resetY = bgTop - (iconH / 2.0f);

                        if (std::abs(clickX - resetX) < (iconW / 2.0f) && std::abs(clickY - resetY) < (iconH / 2.0f)) {
                            gameGrid.setLevel(gameGrid.getCurrentLevel());
                            needsSpriteUpdate = true;
                        }
                    }
                }
                else if (event.type == SDL_EVENT_FINGER_UP || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                    joy.reset();
                    needsSpriteUpdate = true;
                }
            }

            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key >= SDLK_1 && event.key.key <= SDLK_9) {
                    int targetLevel = event.key.key - SDLK_0;
                    gameGrid.setLevel(targetLevel);
                    currentState = PLAYING;
                    needsSpriteUpdate = true;
                }

                int dx = 0, dy = 0;
                if (event.key.key == SDLK_UP) dy = -1;
                if (event.key.key == SDLK_DOWN) dy = 1;
                if (event.key.key == SDLK_LEFT) dx = -1;
                if (event.key.key == SDLK_RIGHT) dx = 1;
                if (event.key.key == SDLK_ESCAPE) { currentState = MENU; needsSpriteUpdate = true; }
                if (event.key.key == SDLK_R && currentState == PLAYING) { gameGrid.setLevel(gameGrid.getCurrentLevel()); needsSpriteUpdate = true; }

                if (currentState == PLAYING) {
                    playerState = gameGrid.movePlayer(dx, dy);
                    if ((dx != 0 || dy != 0) && playerState == 1) needsSpriteUpdate = true;
                }
            }
        }

        if (currentState == PLAYING && joy.active) {
            Uint64 now = SDL_GetTicks();
            if (now - joy.lastMoveTime > joy.moveDelay) {
                int dx = 0, dy = 0;
                if (std::abs(joy.knobX) > joy.radius * 0.5f || std::abs(joy.knobY) > joy.radius * 0.5f) {
                    if (std::abs(joy.knobX) > std::abs(joy.knobY)) dx = (joy.knobX > 0) ? 1 : -1;
                    else dy = (joy.knobY > 0) ? -1 : 1;

                    playerState = gameGrid.movePlayer(dx, dy);
                    if (playerState == 1) needsSpriteUpdate = true;
                    joy.lastMoveTime = now;
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

            if (currentState == MENU) {
                listaSprites.push_back({0.0f, 0.0f, bgW, bgH, texMenu});
            }
            else {
                listaSprites.push_back({0.0f, 0.0f, bgW, bgH, texBack});

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

                float iconScale = 0.15f;
                float iconW = (iconScale * minSide) / w;
                float iconH = (iconScale * minSide) / h;
                float resetX = bgRight - (iconW / 2.0f);
                float resetY = bgTop - (iconH / 2.0f);
                listaSprites.push_back({resetX, resetY, iconW, iconH, texReset});

                float joyBaseW = (joy.radius * 2.0f * minSide) / w;
                float joyBaseH = (joy.radius * 2.0f * minSide) / h;
                listaSprites.push_back({joy.centerX, joy.centerY, joyBaseW, joyBaseH, texJoyBase});

                float knobSize = joy.radius * 0.8f;
                float knobW = (knobSize * 2.0f * minSide) / w;
                float knobH = (knobSize * 2.0f * minSide) / h;
                listaSprites.push_back({joy.centerX + joy.knobX, joy.centerY + joy.knobY, knobW, knobH, texJoyKnob});
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