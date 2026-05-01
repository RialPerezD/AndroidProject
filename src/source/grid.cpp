#include "include/grid.h"
#include <SDL3/SDL.h>

Grid::Grid() {
    currentCells.assign(size * size, 0);
    loadAllLevels();
    setLevel(1);
}

Grid::~Grid() {}

void Grid::loadAllLevels() {
    for (int i = 1; i <= 10; ++i) {
        std::string path = "levels/" + std::to_string(i) + ".txt";
        size_t fileSize;
        void* buffer = SDL_LoadFile(path.c_str(), &fileSize);

        if (buffer) {
            std::vector<int> levelData;
            char* content = (char*)buffer;

            for (size_t j = 0; j < fileSize; ++j) {
                if (content[j] == '0' || content[j] == '1' || content[j] == '2') {
                    levelData.push_back(content[j] - '0');
                }
            }

            if (levelData.size() == (size_t)(size * size)) {
                levelCache[i] = levelData;
            }
            SDL_free(buffer);
        }
    }
}

void Grid::setLevel(int levelNumber) {
    if (levelCache.count(levelNumber)) {
        currentCells = levelCache[levelNumber];
        findAndSetPlayerInitialPosition();
    }
}

void Grid::findAndSetPlayerInitialPosition() {
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            if (getCell(x, y) == 2) {
                playerX = x;
                playerY = y;
                return;
            }
        }
    }
}

int Grid::getCell(int x, int y) const {
    if (x < 0 || x >= size || y < 0 || y >= size) return 1;
    return currentCells[y * size + x];
}

void Grid::setCell(int x, int y, int value) {
    if (x >= 0 && x < size && y >= 0 && y < size) {
        currentCells[y * size + x] = value;
    }
}

bool Grid::movePlayer(int dx, int dy) {
    int nextX = playerX + dx;
    int nextY = playerY + dy;

    if (getCell(nextX, nextY) == 0) {
        setCell(playerX, playerY, 0);
        playerX = nextX;
        playerY = nextY;
        setCell(playerX, playerY, 2);
        return true;
    }
    return false;
}