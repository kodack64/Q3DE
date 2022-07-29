// Copyright 2022 NTT CORPORATION

#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <cassert>
#include <queue>
#include <fstream>
#include <algorithm>

#ifdef _MSC_VER
//#define VISUALIZE
#include <Windows.h>
void setMyCursorPos(int x, int y) {
    HANDLE hCons = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos;
    pos.X = x;
    pos.Y = y;
    SetConsoleCursorPosition(hCons, pos);
}
#endif

using namespace std;

#define MAX_CYCLE 100000

class SpaceInfo {
public:
    bool parent;
    int belong;
    int burst_lifetime = 0;
    int expand_lifetime = 0;
    SpaceInfo() : parent(false), belong(-1), burst_lifetime(0) {};
    string to_string() {
        string s;
        if (belong == -1) s = "*";
        else s = std::to_string(belong);
        if (burst_lifetime > 0) s = "b" + s;
        return string(5 - s.length(), ' ') + s;
    }
};


class QubitPlane {
public:
    int w;
    int h;
    int n;
    vector<vector<SpaceInfo>> space;
    QubitPlane(int _w, int _h) : w(_w*2-1), h(_h*2-1), space(2*_w-1, std::vector<SpaceInfo>(2*_h-1)) {
        int index = 0;
        for (int y = 0; y < space.size(); ++y) {
            for (int x = 0; x < space[y].size(); ++x) {
                if (y % 2 == 1 && x % 2 == 1) {
                    space[y][x].belong = index;
                    space[y][x].parent = true;
                    index += 1;
                }
                else {
                    space[y][x].belong = -1;
                    space[y][x].parent = false;
                }
            }
        }
        n = index;
    };
    void next() {
        for (int y = 0; y < space.size(); ++y) {
            for (int x = 0; x < space[y].size(); ++x) {
                // decrement bust-error lifetime
                if (space[y][x].burst_lifetime > 0) {
                    space[y][x].burst_lifetime -= 1;
                }

                // decremenet expand lifetime
                if (space[y][x].expand_lifetime > 0) {
                    space[y][x].expand_lifetime -= 1;
                }

                // shrink code
                if (!space[y][x].parent && space[y][x].expand_lifetime == 0) {
                    space[y][x].belong = -1;
                }
            }
        }
    }
    bool bounded(int x, int y) {
        return 0 <= x && x < w && 0 <= y && y < h;
    }
    void hit_anomaly(int x, int y, int ano_life) {
        space[y][x].burst_lifetime = ano_life;

        // if hit on logical qubit, expand
        if (space[y][x].parent) {
            int dx[] = { 1, 0, 1 };
            int dy[] = { 0, 1, 1 };
            int mark = space[y][x].belong;
            for (int i = 0; i < 3; ++i) {
                //space[y+dy[i]][x + dx[i]].burst_lifetime = 0;
                space[y + dy[i]][x + dx[i]].belong = mark;
                space[y + dy[i]][x + dx[i]].expand_lifetime = ano_life;
            }
        }
    }
    bool allocate_path(int i1, int i2) {
        vector<vector<vector<pair<int,int>>>> hist_map(h, vector<vector<pair<int,int>>>(w));
        queue<pair<int, int>> queue;
        vector<pair<int, int>> goals;

        // enumrate starts and goals
        for (int y = 0; y < space.size(); ++y) {
            for (int x = 0; x < space[y].size(); ++x) {
                // add start
                if (space[y][x].belong == i1) {
                    // if target space is affected by burst error, cannot connect to it
                    if (space[y][x].burst_lifetime > 0) continue;
                    vector<pair<int, int>> history;
                    auto pos = make_pair(x, y);
                    queue.push(pos);
                    hist_map[y][x].push_back(pos);
                }

                // add goals
                if (space[y][x].belong == i2) {
                    // if target space is affected by burst error, cannot connect to it
                    if (space[y][x].burst_lifetime > 0) continue;
                    // left and right is smooth boundary
                    if (bounded(x + 1, y)) {
                        if (space[y][x+1].burst_lifetime == 0)
                            goals.push_back(make_pair(x + 1, y));
                    }
                    if (bounded(x - 1, y)) {
                        if (space[y][x - 1].burst_lifetime == 0)
                            goals.push_back(make_pair(x - 1, y));
                    }
                }
            }
        }

        // Find path with DP
        while (!queue.empty()) {
            auto pos = queue.front();
            queue.pop();

            int dx[] = { -1, 0, 1, 0 };
            int dy[] = { 0, -1, 0, 1 };

            // try to move NSWE if next position is "vacant" and "bounded" and "non-burst error" and "not-visited or shortest update"
            for (int i = 0; i < 4; ++i) {
                // first step must right or left
                if (hist_map[pos.second][pos.first].size()==1 && i % 2 == 1) continue;

                auto npos = make_pair(pos.first + dx[i], pos.second + dy[i]);
                // must be bounded
                if (!bounded(npos.first, npos.second)) continue;

                // must be unused
                if (space[npos.second][npos.first].belong != -1) continue;
                assert(space[npos.second][npos.first].expand_lifetime == 0);

                // must be burst-error free
                if (space[npos.second][npos.first].burst_lifetime > 0) continue;

                auto history = hist_map[pos.second][pos.first];
                auto n_history = hist_map[npos.second][npos.first];
                // not new position and has shorter history
                if (n_history.size() != 0 && n_history.size() <= history.size() + 1) continue;

                history.push_back(npos);
                hist_map[npos.second][npos.first] = history;
                queue.push(npos);
            }
        }

        // find best goal
        int best = -1;
        int min_len = 1 << 20;
        for (int i = 0; i < goals.size(); ++i) {
            auto pos = goals[i];
            int len = (int)hist_map[pos.second][pos.first].size();
            if (len == 0) continue;
            if (len < min_len) {
                min_len = len;
                best = i;
            }
        }

        // if no goal has history, return false
        if (best == -1) return false;

        // if found, allocate and return true
        auto best_goal = goals[best];
        auto best_hist = hist_map[best_goal.second][best_goal.first];
        for (int i = 1; i < best_hist.size(); ++i) {
            auto mid = best_hist[i];
            space[mid.second][mid.first].belong = i1;
        }
        return true;
    }
    string to_string() {
        string ss = "";
        for (int y = 0; y < space.size(); ++y) {
            for (int x = 0; x < space[y].size(); ++x) {
                ss += space[y][x].to_string();
            }
            ss += "\n";
        }
        return ss;
    }
};

class Instruction {
public:
    Instruction(int c1, int c2) : con1(c1), con2(c2) {};
    int con1;
    int con2;
};

int run(int width, int n_inst, int seed, double ano_prob, int ano_life) {
    mt19937 mt(seed);
    QubitPlane plane(width, width);
    uniform_real_distribution<> urd;

    int n = plane.n;

    // create inst list
    vector<Instruction> inst_list;
    vector<int> inst_finish;
    vector<int> indices;
    for (int i = 0; i < n; ++i) {
        indices.push_back(i);
    }
    for (int i = 0; i < n_inst; ++i) {
        shuffle(indices.begin(), indices.end(), mt);
        inst_list.push_back(Instruction(indices[0], indices[1]));
        inst_finish.push_back(0);
    }

    // run simulation
    int finish_count = 0;
    int cycle_unit = 0;
    vector<int> stale(n, 0);
    while (finish_count < inst_list.size()) {
        // check burst errors
        vector<pair<int,int>> bursts;
        for (int y = 0; y < plane.h; ++y) {
            for (int x = 0; x < plane.w; ++x) {
                if (urd(mt) < ano_prob) {
                    plane.hit_anomaly(x, y, ano_life);
                    bursts.push_back(make_pair(x, y));
                }
            }
        }

        // refresh stale
        std::fill(stale.begin(), stale.end(), 0);

        // try execute
        vector<int> processed;
        vector<pair<int, int>> blocked;
        for (int i = 0; i < inst_list.size(); ++i) {
            // skip executed
            if (inst_finish[i]) continue;

            auto& inst = inst_list[i];
            // skip if either is stale and block the following
            if (stale[inst.con1] || stale[inst.con2]) {
                stale[inst.con1] = 1;
                stale[inst.con2] = 1;
                blocked.push_back(make_pair(inst.con1, inst.con2));
            }
            else {
                // try allocate
                int result = plane.allocate_path(inst.con1, inst.con2);
                if (result) {
                    processed.push_back(i);
                    inst_finish[i] = 1;
                    finish_count += 1;
                }
                else {
                    blocked.push_back(make_pair(inst.con1, inst.con2));
                }
                stale[inst.con1] = 1;
                stale[inst.con2] = 1;
            }
        }

#ifdef VISUALIZE
        setMyCursorPos(0, 0);
        system("cls");
        cout << "* Code cycle: " << cycle_unit << endl;
        cout << plane.to_string() << endl;

        cout << "*** processed" << endl;
        for (auto i : processed) {
            printf("(%d, %d)    \n", inst_list[i].con1, inst_list[i].con2);
        }
        cout << "*** blocked" << endl;
        for (auto i : blocked) {
            printf("(%d, %d)    \n", i.first, i.second);
        }
        cout << "*** burst" << endl;
        for (auto i : bursts) {
            printf("(%d, %d)    \n", i.first, i.second);
        }
        cout << "***" << cycle_unit << endl;
        Sleep(16);
        getchar();
#endif

        plane.next();
        cycle_unit += 1;
        if (cycle_unit >= MAX_CYCLE) break;
    }
    return cycle_unit;
}


int repeat(string filename, int width, int n_inst, int trial, double ano_prob, int ano_life) {
    random_device rd;
    mt19937 mt(rd());
    vector<int> cycle;
    int sum = 0;
    for (int i = 0; i < trial; ++i) {
        int seed = mt();
        int c = run(width, n_inst, seed, ano_prob, ano_life);
        sum += c;
        cycle.push_back(c);
        cout << i << " " << c << " " << sum * 1.0 / (i + 1) << endl;

        ofstream fout(filename, ios::app);
        fout << width << " " << n_inst << " " << ano_prob << " " << ano_life << " " << c << endl;
        fout.close();
    }
    return 0;
}

int main(int argc, char** argv) {
    string filename = "test.txt";
    int width = 5;
    int n_inst = 100;
    int trial = 100;
    double ano_prob = 10e-4 / 1;
    int ano_life = int(10e-3 / 10e-6);

    if (argc > 1) {
        if (argc != 7) {
            cout << "invalid argument; filename, width, n_inst, trial, ano_prob, ano_life" << endl;
            return 0;
        }
        filename = argv[1];
        width = atoi(argv[2]);
        n_inst = atoi(argv[3]);
        trial = atoi(argv[4]);
        ano_prob = atof(argv[5]);
        ano_life = atoi(argv[6]);
    }

    repeat(filename, width, n_inst, trial, ano_prob, ano_life);
    return 0;
}