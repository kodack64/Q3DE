# Copyright 2022 NTT CORPORATION

import os
import numpy as np
import networkx as nx


def add_3d_range(graph: nx.Graph, start_list: list, width_list: list) -> None:
    for z in range(start_list[2], start_list[2] + width_list[2], 2):
        for y in range(start_list[1], start_list[1] + width_list[1], 2):
            for x in range(start_list[0], start_list[0] + width_list[0], 2):
                node = (x, y, z)
                graph.add_node(node)


def create_stabx_surgery_graph(distance: int, cycle: int) -> None:
    graph = nx.Graph()
    d = distance
    c = cycle
    """ 0, 2, 4, ..., 2c-2
      0123456789012345678901234567890
    0 
    1 L-S-S-S-S-^.........^-S-S-S-S-R
    2   | | | |             | | | |
    3 L-S-S-S-S-^.........^-S-S-S-S-R
    4   | | | |             | | | |
    5 L-S-S-S-S-^.........^-S-S-S-S-R
    6   | | | |             | | | |
    7 L-S-S-S-S-^.........^-S-S-S-S-R
    8   | | | |             | | | |
    9 L-S-S-S-S-^.........^-S-S-S-S-R
    A
    """
    """ 2c
      0123456789012345678901234567890
    0 
    1 L-S-S-S-S-^.^.^.^.^.^-S-S-S-S-R
    2   | | | |             | | | |
    3 L-S-S-S-S-^.^.^.^.^.^-S-S-S-S-R
    4   | | | |             | | | |
    5 L-S-S-S-S-^.^.^.^.^.^-S-S-S-S-R
    6   | | | |             | | | |
    7 L-S-S-S-S-^.^.^.^.^.^-S-S-S-S-R
    8   | | | |             | | | |
    9 L-S-S-S-S-^.^.^.^.^.^-S-S-S-S-R
    A
    """
    add_3d_range(graph, (2, 1, 0), (2*d-2, 2*d, 2*c))
    add_3d_range(graph, (2+4*d, 1, 0), (2*d-2, 2*d, 2*c))
    """ 2c+2, ... , 4c-2
      0123456789012345678901234567890
    0 
    1 L-S-S-S-S-S-S-S-S-S-S-S-S-S-S-R
    2   | | | | | | | | | | | | | |
    3 L-S-S-S-S-S-S-S-S-S-S-S-S-S-S-R
    4   | | | | | | | | | | | | | |
    5 L-S-S-S-S-S-S-S-S-S-S-S-S-S-S-R
    6   | | | | | | | | | | | | | |
    7 L-S-S-S-S-S-S-S-S-S-S-S-S-S-S-R
    8   | | | | | | | | | | | | | |
    9 L-S-S-S-S-S-S-S-S-S-S-S-S-S-S-R
    A
    """
    add_3d_range(graph, (2, 1, 2*c), (6*d-2, 2*d, 2*c-2))
    """ 4c
      0123456789012345678901234567890
    0 
    1 L-S-S-S-S-v.v.v.v.v.v-S-S-S-S-R
    2   | | | |             | | | |
    3 L-S-S-S-S-v.v.v.v.v.v-S-S-S-S-R
    4   | | | |             | | | |
    5 L-S-S-S-S-v.v.v.v.v.v-S-S-S-S-R
    6   | | | |             | | | |
    7 L-S-S-S-S-v.v.v.v.v.v-S-S-S-S-R
    8   | | | |             | | | |
    9 L-S-S-S-S-v.v.v.v.v.v-S-S-S-S-R
    A
    """
    """ 4c+2, ... 6c
      0123456789012345678901234567890
    0 
    1 L-S-S-S-S-v.........v-S-S-S-S-R
    2   | | | |             | | | |
    3 L-S-S-S-S-v.........v-S-S-S-S-R
    4   | | | |             | | | |
    5 L-S-S-S-S-v.........v-S-S-S-S-R
    6   | | | |             | | | |
    7 L-S-S-S-S-v.........v-S-S-S-S-R
    8   | | | |             | | | |
    9 L-S-S-S-S-v.........v-S-S-S-S-R
    A
    """
    add_3d_range(graph, (2, 1, 4*c-2), (2*d-2, 2*d, 2*c))
    add_3d_range(graph, (2+4*d, 1, 4*c-2), (2*d-2, 2*d, 2*c))

    left_list = []
    right_list = []
    wedge_list = []
    vee_list = []
    for y in range(1, 2*d, 2):
        for z in range(0, 6*c-2, 2):
            left_list.append((0, y, z))
            right_list.append((6*d, y, z))
        for z in range(0, 2*c, 2):
            wedge_list.append((2*d, y, z))
            wedge_list.append((4*d, y, z))
        for z in range(4*c-2, 6*c-2, 2):
            vee_list.append((2*d, y, z))
            vee_list.append((4*d, y, z))
        for x in range(2*d+2, 4*d, 2):
            wedge_list.append((x, y, 2*c-2))
            vee_list.append((x, y, 4*c-2))
    boundary_dict = {
        "L": left_list,
        "R": right_list,
        "^": wedge_list,
        "v": vee_list,
    }

    boundary_path_length = {}
    name = ["L", "R", "^", "v"]
    for node in graph.nodes:
        len_L = node[0]//2
        len_R = (6*d - node[0])//2
        if node[2] < 2*d:
            len_W = np.min([abs(2*d-node[0])//2, abs(4*d-node[0])//2])
            len_V = 2*d
        elif node[2] >= 4*d-2:
            len_W = 2*d
            len_V = np.min([abs(2*d-node[0])//2, abs(4*d-node[0])//2])
        else:
            if node[0] < 2*d or node[0] > 4*d:
                len_W = np.min([abs(2*d-node[0])//2, abs(4*d-node[0])//2]) + abs(node[2]-(2*d-2))//2
                len_V = np.min([abs(2*d-node[0])//2, abs(4*d-node[0])//2]) + abs(node[2]-(4*d-2))//2
            else:
                len_W = abs(node[2]-(2*d-2))//2
                len_V = abs(node[2]-(4*d-2))//2

        vals = [len_L, len_R, len_W, len_V]
        min_len = np.min(vals)
        min_ind = vals.index(min_len)
        min_name = name[min_ind]
        boundary_path_length[node] = (min_name, min_len)
    return (graph, boundary_dict, boundary_path_length)


def create_stabz_surgery_graph(distance: int, cycle: int) -> None:
    graph = nx.Graph()
    d = distance
    c = cycle
    """ 0, 2, 4, ..., 2c | 4c, ... 6c
      0123456789012345678901234567890
    0  F F F F F . . . . . F F F F F
    1  | | | | |           | | | | |
    2  S-S-S-S-S . . . . . S-S-S-S-S
    3  | | | | |           | | | | |
    4  S-S-S-S-S . . . . . S-S-S-S-S
    5  | | | | |           | | | | |
    6  S-S-S-S-S . . . . . S-S-S-S-S
    7  | | | | |           | | | | |
    8  S-S-S-S-S . . . . . S-S-S-S-S
    9  | | | | |           | | | | |
    0  B B B B B . . . . . B B B B B
    """
    add_3d_range(graph, (1, 2, 0), (2*d, 2*d-2, 2*c))
    add_3d_range(graph, (1+4*d, 2, 0), (2*d, 2*d-2, 2*c))
    """ 2c+2, ... 4c-2
      0123456789012345678901234567890
    0  F F F F F F F F F F F F F F F
    1  | | | | | | | | | | | | | | |
    2  S-S-S-S-S-S-S-S-S-S-S-S-S-S-S
    3  | | | | | | | | | | | | | | |
    4  S-S-S-S-S-S-S-S-S-S-S-S-S-S-S
    5  | | | | | | | | | | | | | | |
    6  S-S-S-S-S-S-S-S-S-S-S-S-S-S-S
    7  | | | | | | | | | | | | | | |
    8  S-S-S-S-S-S-S-S-S-S-S-S-S-S-S
    9  | | | | | | | | | | | | | | |
    0  B B B B B B B B B B B B B B B
    """
    add_3d_range(graph, (1, 2, 2*c), (6*d, 2*d-2, 2*c-2))
    """ 0, 2, 4, ..., 2c | 4c, ... 6c
      0123456789012345678901234567890
    0  F F F F F . . . . . F F F F F
    1  | | | | |           | | | | |
    2  S-S-S-S-S . . . . . S-S-S-S-S
    3  | | | | |           | | | | |
    4  S-S-S-S-S . . . . . S-S-S-S-S
    5  | | | | |           | | | | |
    6  S-S-S-S-S . . . . . S-S-S-S-S
    7  | | | | |           | | | | |
    8  S-S-S-S-S . . . . . S-S-S-S-S
    9  | | | | |           | | | | |
    0  B B B B B . . . . . B B B B B
    """
    add_3d_range(graph, (1, 2, 4*c-2), (2*d, 2*d-2, 2*c))
    add_3d_range(graph, (1+4*d, 2, 4*c-2), (2*d, 2*d-2, 2*c))

    front_list = []
    back_list = []
    for z in range(0, 2*c, 2):
        for x in range(1, 2*d, 2):
            front_list.append((x, 0, z))
            back_list.append((x, 2*d, z))
        for x in range(4*d+1, 6*d, 2):
            front_list.append((x, 0, z))
            back_list.append((x, 2*d, z))
    for z in range(2*c, 4*c-2, 2):
        for x in range(1, 6*d, 2):
            front_list.append((x, 0, z))
            back_list.append((x, 2*d, z))
    for z in range(4*c-2, 6*c-2, 2):
        for x in range(1, 2*d, 2):
            front_list.append((x, 0, z))
            back_list.append((x, 2*d, z))
        for x in range(4*d+1, 6*d, 2):
            front_list.append((x, 0, z))
            back_list.append((x, 2*d, z))
    boundary_dict = {
        "F": front_list,
        "B": back_list,
    }

    boundary_path_length = {}
    for node in graph.nodes:
        len_F = node[1]//2
        len_B = (2*d - node[1])//2
        if len_B < len_F:
            boundary_path_length[node] = ("B", len_B)
        else:
            boundary_path_length[node] = ("F", len_F)
    return (graph, boundary_dict, boundary_path_length)


def create_stabx_idling_graph(distance: int, cycle: int) -> None:
    graph = nx.Graph()
    d = distance
    c = cycle
    """
      01234567890
    0 
    1 L-S-S-S-S-R
    2   | | | |  
    3 L-S-S-S-S-R
    4   | | | |  
    5 L-S-S-S-S-R
    6   | | | |  
    7 L-S-S-S-S-R
    8   | | | |  
    9 L-S-S-S-S-R
    A
    """
    add_3d_range(graph, (2, 1, 0), (2*d-2, 2*d, 2*c))

    left_list = []
    right_list = []
    for y in range(1, 2*d, 2):
        for z in range(0, 2*c, 2):
            left_list.append((0, y, z))
            right_list.append((2*d, y, z))
    boundary_dict = {
        "L": left_list,
        "R": right_list,
    }

    boundary_path_length = {}
    name = ["L", "R"]
    for node in graph.nodes:
        len_L = node[0]//2
        len_R = (2*d - node[0])//2
        vals = [len_L, len_R]
        min_len = np.min(vals)
        min_ind = vals.index(min_len)
        min_name = name[min_ind]
        boundary_path_length[node] = (min_name, min_len)
    return (graph, boundary_dict, boundary_path_length)


def create_stabz_idling_graph(distance: int, cycle: int) -> None:
    graph = nx.Graph()
    d = distance
    c = cycle
    """
      01234567890
    0  F F F F F 
    1  | | | | | 
    2  S-S-S-S-S 
    3  | | | | | 
    4  S-S-S-S-S 
    5  | | | | | 
    6  S-S-S-S-S 
    7  | | | | | 
    8  S-S-S-S-S 
    9  | | | | | 
    0  B B B B B 
    """
    add_3d_range(graph, (1, 2, 0), (2*d, 2*d-2, 2*c))

    front_list = []
    back_list = []
    for z in range(0, 2*c, 2):
        for x in range(1, 2*d, 2):
            front_list.append((x, 0, z))
            back_list.append((x, 2*d, z))
    boundary_dict = {
        "F": front_list,
        "B": back_list,
    }

    boundary_path_length = {}
    for node in graph.nodes:
        len_F = node[1]//2
        len_B = (2*d - node[1])//2
        if len_B < len_F:
            boundary_path_length[node] = ("B", len_B)
        else:
            boundary_path_length[node] = ("F", len_F)
    return (graph, boundary_dict, boundary_path_length)


def preprocess(graph: nx.Graph, bnd: dict):
    nodes = list(graph.nodes)
    difs = [
        (1, 0, 0),
        (-1, 0, 0),
        (0, 1, 0),
        (0, -1, 0),
        (0, 0, 1),
        (0, 0, -1),
    ]
    for node in nodes:
        for dif in difs:
            neighbor = (node[0]+dif[0]*2, node[1]+dif[1]*2, node[2]+dif[2]*2)
            pos = (node[0]+dif[0], node[1]+dif[1], node[2]+dif[2])

            if neighbor in nodes:
                graph.add_edge(node, neighbor, pos=pos)
            for name, bnds in bnd.items():
                if neighbor in bnds:
                    graph.add_edge(node, name, pos=pos)

def dump_graph(graph: nx.Graph, path: dict, folder: str, name: str):
    if not os.path.exists(folder):
        os.mkdir(folder)

    nodes = list(graph.nodes)

    with open(f"./{folder}/{name}.node", "w") as fout:
        for index, node in enumerate(nodes):
            if isinstance(node, tuple):
                fout.write(f"{index} {node[0]} {node[1]} {node[2]}\n")

    with open(f"./{folder}/{name}.boundary", "w") as fout:
        for index, node in enumerate(nodes):
            if isinstance(node, str):
                fout.write(f"{index} {node}\n")

    with open(f"./{folder}/{name}.boundary_path", "w") as fout:
        for index, node in enumerate(nodes):
            if isinstance(node, tuple):
                assert(node in path)
                info = path[node]
                fout.write(f"{index} {info[0]} {info[1]}\n")

    with open(f"./{folder}/{name}.edge", "w") as fout:
        for edge in graph.edges(data=True):
            ei1 = nodes.index(edge[0])
            ei2 = nodes.index(edge[1])
            pos = edge[2]["pos"]
            fout.write(f"{ei1} {ei2} {pos[0]} {pos[1]} {pos[2]}\n")


def visualize(gx: nx.Graph, gz: nx.Graph, bdx: dict, bdz: dict, distance: int, cycle: int) -> None:
    d = distance
    c = cycle
    bd = {}
    bd.update(bdx)
    bd.update(bdz)
    vals = np.empty(shape=(6*d+1, 2*d+1, 6*c-3), dtype=str)
    for node in gx.nodes:
        if isinstance(node, str):
            continue
        assert(vals[node] == "")
        vals[node] = "X"
    for node in gz.nodes:
        if isinstance(node, str):
            continue
        assert(vals[node] == "")
        vals[node] = "Z"
    for name, dic in bd.items():
        for node in dic:
            assert(vals[node] == "")
            vals[node] = name
    np.set_printoptions(threshold=np.inf, linewidth=np.inf)
    for z in range(vals.shape[2]):
        print(f"Cycle {z}")
        print("   ", end="")
        for x in range(vals.shape[0]):
            print(f"{x:>2}", end="")
        print()
        for y in range(vals.shape[1]):
            print(f"{y:>2} ", end="")
            for x in range(vals.shape[0]):
                s = vals[(x, y, z)]
                print(f"{s:2}", end="")
            print()
        print("-"*50)


def visualize_boundary_path(graph: nx.Graph, bd: dict, path, distance: int, cycle: int) -> None:
    np.set_printoptions(threshold=np.inf, linewidth=np.inf)
    d = distance
    c = cycle
    vals = np.empty(shape=(6*d+1, 2*d+1, 6*c-3), dtype="<U2")
    for node in graph.nodes:
        if isinstance(node, str):
            continue
        assert(vals[node] == "")
        assert(node in path)
        info = path[node]
        vals[node] = f"{info[0].lower()}{str(info[1])}"
    for name, dic in bd.items():
        for node in dic:
            assert(vals[node] == "")
            vals[node] = name
    for z in range(vals.shape[2]):
        print(f"Cycle {z}")
        print("   ", end="")
        for x in range(vals.shape[0]):
            print(f"{x:>2}", end="")
        print()
        for y in range(vals.shape[1]):
            print(f"{y:>2} ", end="")
            for x in range(vals.shape[0]):
                s = vals[(x, y, z)]
                print(f"{s:2}", end="")
            print()
        print("-"*50)


def create_surgery_graph(distance: int, cycle: int) -> None:
    assert(distance % 2 == 1)
    folder = "graph_surgery"
    post = f"distance_{distance}_cycle_{cycle}"
    gx, bdx, pathx = create_stabx_surgery_graph(distance, cycle)
    preprocess(gx, bdx)
    dump_graph(gx, pathx, folder, f"stabx_{post}")
    gz, bdz, pathz = create_stabz_surgery_graph(distance, cycle)
    preprocess(gz, bdz)
    dump_graph(gz, pathz, folder, f"stabz_{post}")

    # visualize(gx, gz, bdx, bdz, distance, cycle)
    # visualize_boundary_path(gx, bdx, pathx, distance, cycle)
    # visualize_boundary_path(gz, bdz, pathz, distance, cycle)



def create_idling_graph(distance: int, cycle: int) -> None:
    assert(distance % 2 == 1)
    folder = "graph_idling"
    post = f"distance_{distance}_cycle_{cycle}"
    gx, bdx, pathx = create_stabx_idling_graph(distance, cycle)
    preprocess(gx, bdx)
    dump_graph(gx, pathx, folder, f"stabx_{post}")
    gz, bdz, pathz = create_stabz_idling_graph(distance, cycle)
    preprocess(gz, bdz)
    dump_graph(gz, pathz, folder, f"stabz_{post}")

    # visualize(gx, gz, bdx, bdz, distance, cycle)
    # visualize_boundary_path(gx, bdx, pathx, distance, cycle)
    # visualize_boundary_path(gz, bdz, pathz, distance, cycle)


for cycle in [5, 7, 9]:
    for distance in [5, 7, 9]:
        create_idling_graph(distance, cycle)
        # create_idling_short_graph(distance, cycle)
        # create_surgery_graph(distance, cycle)
        # create_surgery_short_graph(distance, cycle)
