#include "grid.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef DEBUG
#include <stdio.h>
#define DBGprint(fmt, args...) fprintf(stderr, "\033[1;32m  ---- grid in c ----(%s:%d):\t\033[0m" fmt, __func__, __LINE__, ##args)
#else
#define DBGprint(fmt, args...)
#endif

// 相除并且向上取整
static int ceil_div(float value, float divisor) {
    const float resultf = value / divisor;
    const int result = (int)resultf;
    return result < resultf ? result + 1: result;
}

// 32 位整数取较小值
static int min_int(int a, int b) {
    a -= b;
    a &= a >> 31;
    return a + b;
}

// 32 位整数取较大值
static int max_int(int a, int b) {
    a -= b;
    a &= (~a) >> 31;
    return a + b;
}

// 计算一行或者一列内的网格索引
static int to_cell_idx(float val, float inv_cell_size, int num_cells) {
    const int cell_pos = (int)(val * inv_cell_size);
    return min_int(max_int(cell_pos, 0), num_cells - 1);
}

// 计算 x 对应的网格行号
static int grid_cell_x(const Grid* grid, float x) {
    return to_cell_idx(x - grid->x, grid->inv_cell_w, grid->num_cols);
}

// 计算 y 对应的网格列号
static int grid_cell_y(const Grid* grid, float y) {
    return to_cell_idx(y - grid->y, grid->inv_cell_h, grid->num_rows);
}

void grid_create(Grid* grid, float cell_w, float cell_h, float l, float t, float r, float b) {
    const float w = r - l, h = b - t;
    const int num_cols = ceil_div(w, cell_w), num_rows = ceil_div(h, cell_h);

    grid->num_cols = num_cols;
    grid->num_rows = num_rows;
    grid->num_cells = num_cols * num_rows;
    grid->inv_cell_w = 1.0f / cell_w;
    grid->inv_cell_h = 1.0f / cell_w;
    grid->x = l;
    grid->y = t;
    grid->w = w;
    grid->h = h;

    grid->rows = (GridRow *)malloc(num_rows * sizeof(GridRow));
    for (int r = 0; r < num_rows; ++r) {
        grid->rows[r].cells = (int *)malloc(num_cols * sizeof(int));
        for (int c = 0; c < num_cols; ++c) {
            grid->rows[r].cells[c] = -1;
        }
        grid->rows[r].num_nodes = 0;
        grid->rows[r].nfl = nfl_create();
    }
    DBGprint("create grid(row:%d,col:%d)\n", num_rows, num_cols);
}

void grid_destroy(Grid* grid) {
    for (int r = 0; r < grid->num_rows; ++r) {
        nfl_destroy(grid->rows[r].nfl);
        free(grid->rows[r].cells);
    }
    free(grid->rows);
}

void grid_insert(Grid* grid, int handle, float x, float y) {
    const int cell_x = grid_cell_x(grid, x);
    const int cell_y = grid_cell_y(grid, y);
    GridRow* row = &grid->rows[cell_y];
    int* cell = &row->cells[cell_x];
    *cell = nfl_insert(row->nfl, handle, x, y, *cell);
    DBGprint("insert node(handle:%d,x:%f,y:%f) to cell(%d,%d)\n", handle, x, y, cell_x, cell_y);
}

void grid_remove(Grid* grid, int handle, float x, float y) {
    const int cell_x = grid_cell_x(grid, x);
    const int cell_y = grid_cell_y(grid, y);
    GridRow* row = &grid->rows[cell_y];

    int* link = &row->cells[cell_x];
    Node* node = nfl_get(row->nfl, *link);
    while (node && node->handle != handle) {
        link = &node->next;
        node = nfl_get(row->nfl, *link);
    }
    if (node) {
        const int idx = *link;
        *link = node->next;
        nfl_remove(row->nfl, idx);
        DBGprint("remove node(handle:%d,x:%f,y:%f) from cell(%d,%d)\n", node->handle,
                node->x, node->y, cell_x, cell_y);
    }
}

void grid_move(Grid* grid, int handle, float px, float py, float x, float y) {
    const int prev_cell_x = grid_cell_x(grid, px);
    const int prev_cell_y = grid_cell_y(grid, py);
    const int next_cell_x = grid_cell_x(grid, x);
    const int next_cell_y = grid_cell_y(grid, y);
    GridRow* prev_row = &grid->rows[prev_cell_y];

    if (next_cell_x == prev_cell_x && next_cell_y == prev_cell_y) {
        // 如果还在同一个网格内，直接更新位置
        int node_idx = prev_row->cells[prev_cell_x];
        Node* node = nfl_get(prev_row->nfl, node_idx);
        while (node && node->handle != handle) {
            node = nfl_get(prev_row->nfl, node->next);
        }
        if (node) {
            DBGprint("move node(handle:%d,x:%f,y:%f) -> (x:%f,y:%f) in old cell(%d,%d)\n",
                    node->handle, node->x, node->y, x, y, prev_cell_x, prev_cell_y);
            node->x = x;
            node->y = y;
        }
    } else {
        // 从原网格删除，向新网格添加
        GridRow* next_row = &grid->rows[next_cell_y];
        int* link = &prev_row->cells[prev_cell_x];
        Node* node = nfl_get(prev_row->nfl, *link);
        while (node && node->handle != handle) {
            link = &node->next;
            node = nfl_get(prev_row->nfl, *link);
        }
        if (node) {
            const int node_idx = *link;
            *link = node->next;
            nfl_remove(prev_row->nfl, node_idx);
            DBGprint("move node(handle:%d,x:%f,y:%f) -> (x:%f,y:%f) old cell(%d,%d) del, new cell(%d,%d) add\n",
                    node->handle, node->x, node->y, x, y, prev_cell_x, prev_cell_y, next_cell_x, next_cell_y);
            node->x = x;
            node->y = y;

            node->next = next_row->cells[next_cell_x];
            next_row->cells[next_cell_x] = nfl_insert(next_row->nfl, node->handle,
                    node->x, node->y, node->next);
        }
    }
}

void grid_query(const Grid* grid, IntList* out, float x, float y, float w, float h) {
    const float half_w = w / 2;
    const float half_h = h / 2;
    // 找到相交网格的最小跟最大范围，超出地图返回地图边缘网格序号
    const int min_cell_x = grid_cell_x(grid, x - half_w);
    const int min_cell_y = grid_cell_y(grid, y - half_h);
    const int max_cell_x = grid_cell_x(grid, x + half_w);
    const int max_cell_y = grid_cell_y(grid, y + half_h);

    DBGprint("query cell idx from min(%d,%d) -> max(%d,%d)\n", min_cell_x, min_cell_y, max_cell_x, max_cell_y);
    // 遍历相交网格内的所有节点，找到在指定视野内的节点
    il_clear(out);
    for (int cell_y = min_cell_y; cell_y <= max_cell_y; ++cell_y) {
        const GridRow* row = &grid->rows[cell_y];
        for (int cell_x = min_cell_x; cell_x <= max_cell_x; ++cell_x) {
            int node_idx = row->cells[cell_x];
            while (node_idx != -1) {
                const Node* node = nfl_get(row->nfl, node_idx);
                DBGprint("check node in cell(%d,%d) node(handle:%d,x:%f,y:%f)\n", cell_x, cell_y,
                        node->handle, node->x, node->y);
                if (fabs(x - node->x) <= half_w && fabs(y - node->y) <= half_h) {
                    il_set(out, il_push_back(out), 0, node->handle);
                }
                node_idx = node->next;
            }
        }
    }
}

int grid_in_bound(const Grid* grid, float x, float y) {
    x -= grid->x;
    y -= grid->y;
    return x >= 0.0f && x < grid->w && y >= 0.0f && y < grid->h;
}

void grid_optimize(Grid* grid) {
    for (int r = 0; r < grid->num_rows; ++r) {
        NodeFreeList *new_nfl = nfl_create();
        GridRow* row = &grid->rows[r];
        nfl_reserve(new_nfl, row->num_nodes);
        for (int c = 0; c < grid->num_cols; ++c) {
            // 替换旧 freelist 指针
            IntList new_node_idxs;
            il_create(&new_node_idxs, 1);
            int* link = &row->cells[c];
            while (*link != -1) {
                const Node* node = nfl_get(row->nfl, *link);
                il_set(&new_node_idxs, il_push_back(&new_node_idxs), 0,
                        nfl_insert(new_nfl, node->handle, node->x, node->y, node->next));
                *link = node->next;
            }
            for (int j = 0; j < il_size(&new_node_idxs); ++j) {
                const int new_node_idx = il_get(&new_node_idxs, j, 0);
                Node* node = nfl_get(new_nfl, new_node_idx);
                node->next = *link;
                *link = new_node_idx;
            }
        }
        nfl_destroy(row->nfl);
        row->nfl = new_nfl;
    }
}
