// Copyright 2022 NTT CORPORATION

#include "common.hpp"

// visualize error lattice in CUI
void visualize(
    int d,
    const RecoveryInfo &recovery_info,
    const vector<vector<vector<bool>>> &syndrome_map) {
    int y;
    cout << endl;
    rep(z, d) {
        cout << endl;
        rep(ysub, 2 * d - 1) {
            cout << "|";
            y = ysub / 2;
            if (ysub % 2 == 0) {
                rep(x, d - 1) {
                    if (recovery_info.recovery_horizontal[z][y][x])
                        cout << "-";
                    else
                        cout << ".";
                    if (syndrome_map[z][y][x])
                        cout << "O";
                    else
                        cout << " ";
                }
                if (recovery_info.recovery_horizontal[z][y][d - 1])
                    cout << "-";
                else
                    cout << ".";
            } else {
                rep(x, d - 1) {
                    cout << " ";
                    if (recovery_info.recovery_vertical[z][y + 1][x])
                        cout << "|";
                    else
                        cout << ".";
                }
                cout << " ";
            }
            cout << "|" << endl;
        }
        cout << endl;
        if (z < (d - 1)) {
            rep(ysub, 2 * d - 1) {
                cout << " ";
                y = ysub / 2;
                if (ysub % 2 == 0) {
                    rep(x, d - 1) {
                        cout << " ";
                        if (recovery_info.recovery_meas[z + 1][y][x])
                            cout << "M";
                        else
                            cout << ".";
                    }
                }
                cout << " " << endl;
            }
        }
        cout << endl;
    }
    return;
}
