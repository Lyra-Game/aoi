#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "grid.h"

void query(Grid *grid, float x, float y, float w, float h) {
    printf("+++++++++++++ query(%f,%f,%f,%f) +++++++++++++\n", x, y, w, h);
    IntList *out_put = il_create(1);
    grid_query(grid, out_put, x, y, w, h);
    for (int i = 0; i < il_size(out_put); i++) {
        printf("found hanle in query area = %d\n", il_get(out_put, i, 0));
    }
    il_destroy(out_put);
    printf("+++++++++++++ query end +++++++++++++\n");
}

void main() {
    Grid *grid = (Grid *)malloc(sizeof(Grid));
    grid_create(grid, 10, 10, 0, 0, 1000, 1000);

    float x[4] = { 0, 501.11, 100.2, 999999 };
    float y[4] = { 0, 10.22, 88, 999999 };
    for (int i = 0; i < 4; i++) {
        printf("add node %d(%f,%f)\n", i + 1, x[i], y[i]);
        grid_insert(grid, i + 1, x[i], y[i]);
    }

    grid_optimize(grid);

    float sx[2] = { 9999, 99 };
    float sy[2] = { 9999990, 980 };
    for (int j = 0; j < 2; j++) {
        printf("in_bound(%f, %f) = %d\n", sx[j], sy[j], grid_in_bound(grid, sx[j], sy[j]));
    }

    grid_remove(grid, 3, x[2], y[2]);
    printf("remove node %d(%f,%f)\n", 3, x[2], y[2]);

    query(grid, 50, 50, 100, 100);

    grid_move(grid, 2, x[1], y[1], 200, 200);
    printf("move node(%d) from (%f,%f)->(%f,%f)\n", 2, x[1], y[1], 200.0, 200.0);

    query(grid, 150, 150, 200, 200);

    grid_optimize(grid);

    grid_destroy(grid);
    free(grid);
}
