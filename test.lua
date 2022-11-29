local Grid = require("grid")

local function create_grid(t)
    print(string.format("create map: cell_w(%s) cell_h(%s) l(%s),t(%s),r(%s),b(%s)",
        t.cell_w, t.cell_h, t.l, t.t, t.r, t.b))
    return Grid.new(t)
end

local function add_node(grid, handle, node)
    grid:insert(handle, table.unpack(node))
    print(string.format("add node %d(%s,%s)", handle, table.unpack(node)))
end

local function remove_node(grid, handle, node)
    grid:remove(handle, table.unpack(node))
    print(string.format("remove node %d(%s,%s)", handle, table.unpack(node)))
end

local function move_node(grid, handle, node, nx, ny)
    print(string.format("move node(%s) from (%s,%s)->(%s,%s)", handle, node[1], node[2], nx, ny))
    grid:move(handle, node[1], node[2], nx, ny)
    node[1], node[2] = nx, ny
end

local function in_bound(grid, x, y)
    print(string.format("in_bound(%s, %s) = %d", x, y,  grid:in_bound(x, y)))
end

local function query(grid, x, y, w, h)
    print(string.format('+++++++++++++ query(%s,%s,%s,%s) +++++++++++++', x, y, w, h))
    local t = grid:query(x, y, w, h)
    for k, v in pairs(t) do
        print("found hanle in query area = ", v)
    end
    print(string.format('+++++++++++++ query end +++++++++++++'))
end



local t = {
    cell_w = 10,
    cell_h = 10,
    l = 0,
    t = 0,
    r = 1000,
    b = 1000,
}

local grid = create_grid(t)

local node = {
    { 0, 0 },
    { 501.11, 10.22 },
    { 100.2, 88 },
    { 999999, 999999 },
}
for i = 1, #node do
    add_node(grid, i, node[i])
end

grid:optimize()

in_bound(grid, 9999, 9999990)
in_bound(grid, 99, 980)

-- remove_node(grid, 1, node[1])
remove_node(grid, 3, node[3])

query(grid, 50, 50, 100, 100)

move_node(grid, 2, node[2], 200, 200)

query(grid, 150, 150, 200, 200)

grid:optimize()
