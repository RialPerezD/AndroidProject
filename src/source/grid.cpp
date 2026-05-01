#include "include/grid.h"
#include <SDL3/SDL.h>
#include <cctype>
#include <string>

Grid::Grid() {
    currentCells.assign(size * size, 0);
    ladderLayer.assign(size * size, 0);
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
                if (std::isdigit(static_cast<unsigned char>(content[j]))) {
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

bool Grid::setLevel(int levelNumber) {
    if (levelCache.count(levelNumber)) {
        currentLevel = levelNumber;
        currentCells = levelCache[levelNumber];
        ladderLayer.assign(size * size, 0);

        for (int i = 0; i < currentCells.size(); ++i) {
            if (currentCells[i] == 5) {
                ladderLayer[i] = 5;
                currentCells[i] = 0;
            }
        }

        findAndSetPlayerInitialPosition();
        countEnemies();

        return true;
    }

    return false;
}

void Grid::countEnemies() {
    enemyCount = 0;
    for (int cell : currentCells) {
        if (cell == 4) enemyCount++;
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

bool Grid::isLadder(int x, int y) const {
    if (x < 0 || x >= size || y < 0 || y >= size) return false;
    return ladderLayer[y * size + x] == 5;
}

int Grid::movePlayer(int dx, int dy) {
    if (dy < 0 && !isLadder(playerX, playerY)) {
        return false;
    }

    int nextX = playerX + dx;
    int nextY = playerY + dy;
    int targetCell = getCell(nextX, nextY);

    if (targetCell == 0 || targetCell == 4 || targetCell == 6) {
        if (targetCell == 4) enemyCount--;

        setCell(playerX, playerY, 0);
        playerX = nextX;
        playerY = nextY;
        setCell(playerX, playerY, 2);

        if (enemyCount <= 0) {
            if(!setLevel(currentLevel + 1)){
                return 2;
            }
        }
        return 1;
    }

    if (targetCell == 3) {
        int boxNextX = nextX + dx;
        int boxNextY = nextY + dy;
        if (getCell(boxNextX, boxNextY) == 0 && !isLadder(boxNextX, boxNextY)) {
            setCell(boxNextX, boxNextY, 3);
            setCell(nextX, nextY, 0);
            return 1;
        }
    }

    return 0;
}

bool Grid::applyGravity() {
    bool changed = false;
    for (int row = size - 2; row >= 0; --row) {
        for (int col = 0; col < size; ++col) {
            int cellType = getCell(col, row);

            if (cellType >= 2 && cellType <= 4) {
                if (cellType == 2 && isLadder(col, row)) continue;

                if (getCell(col, row + 1) == 0 && !isLadder(col, row + 1)) {
                    setCell(col, row + 1, cellType);
                    setCell(col, row, 0);
                    if (cellType == 2) {
                        playerX = col;
                        playerY = row + 1;
                    }
                    changed = true;
                }
            }
        }
    }
    return changed;
}