#ifndef GRID_H
#define GRID_H

#include <vector>
#include <string>
#include <map>

class Grid {
public:
    Grid();
    ~Grid();

    void loadAllLevels();
    bool setLevel(int levelNumber);

    int getCell(int x, int y) const;
    void setCell(int x, int y, int value);
    bool isLadder(int x, int y) const;
    int getSize() const { return size; }

    int movePlayer(int dx, int dy);
    bool applyGravity();

private:
    int size = 16;
    int currentLevel = 1;
    int enemyCount = 0;
    std::vector<int> currentCells;
    std::vector<int> ladderLayer;
    std::map<int, std::vector<int>> levelCache;
    int playerX, playerY;

    void findAndSetPlayerInitialPosition();
    void countEnemies();
};

#endif