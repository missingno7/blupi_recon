struct Point { int x, y; };
struct Cell { short icon; };
extern Cell grid[100][100];
int TargetGrid(Point pos)
{
    int y = pos.y / 64;
    int x = (pos.x + 30) / 64;
    return grid[x][y].icon;
}
