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
    void setLevel(int levelNumber);

    int getCell(int x, int y) const;
    void setCell(int x, int y, int value);
    int getSize() const { return size; }
    bool movePlayer(int dx, int dy);

private:
    int size = 16;
    std::vector<int> currentCells;
    std::map<int, std::vector<int>> levelCache;
    int playerX, playerY;

    void findAndSetPlayerInitialPosition();
};

#endif