#ifndef GRID_H
#define GRID_H

#include "node_freelist.h"
#include "intlist.h"

#ifdef __cplusplus
#define GRID_FUNC extern "C"
#else
#define GRID_FUNC
#endif

typedef struct Grid Grid;
typedef struct GridRow GridRow;

// 一行网格
struct GridRow {
    // 存储这行所有网格里的节点列表
    NodeFreeList* nfl;

    // 存储这行的网格
    // 每一个 cell 存放的是指向这个网格内第一个元素的指针
    // 如果是 -1 说明这个网格是空的
    int* cells;

    // 存储这行网格的节点个数
    int num_nodes;
};

// 整个网格
struct Grid {
    // 按行存储所有网格
    GridRow* rows;

    // 网格的列、行、网格数量
    int num_cols, num_rows, num_cells;

    // 存储 1/cell_w，1/cell_h
    float inv_cell_w, inv_cell_h;

    // 存储整个地图的左上角
    float x, y;

    // 地图宽、高
    float w, h;
};

// 创建地图网格，网格宽、高，地图左上右下
GRID_FUNC void grid_create(Grid* grid, float cell_w, float cell_h,
                    float l, float t, float r, float b);

// 销毁
GRID_FUNC void grid_destroy(Grid* grid);

// 添加一个节点，handle 为外部句柄，比如 roleid、teamid 之类的，需要保证在 grid 中的所有节点唯一
GRID_FUNC void grid_insert(Grid* grid, int handle, float x, float y);

// 删除一个节点，传递节点的当前位置是为了快速找到节点所在的网格，所以外部应该缓存节点的当前位置
GRID_FUNC void grid_remove(Grid* grid, int handle, float x, float y);

// 移动一个节点，px/py 跟 grid_remove 一样是为了快速找到节点所在的网格，外部自己缓存住
GRID_FUNC void grid_move(Grid* grid, int handle, float px, float py, float x, float y);

// 查询指定区域内的所有节点，注意有可能包含了自己的 handle，外部自己排除自己
// x,y 为视野中心，w,h 为宽高
GRID_FUNC void grid_query(const Grid* grid, IntList* out, float x, float y, float w, float h);

// 返回位置是否在地图范围里
GRID_FUNC int grid_in_bound(const Grid* grid, float x, float y);

// 优化网格空间，把空网格内的 freelist 清除，一般来说一段时间调用一次，也可以依据节点变动情况调用
GRID_FUNC void grid_optimize(Grid* grid);

#endif
