// Copyright 2022 NTT CORPORATION

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <queue>
#include <string>
#include <fstream>

using namespace std;


int poisson(double lambda, double value){
    double sum = 0;
    int k=0;
    while(true){
        double pdm = pow(lambda, k) * exp(-lambda);
        for(int ki=k;ki>0;ki--){
            pdm/=ki;
        }
        sum += pdm;
        if(value < sum) break;
        k+=1;
    }
    return k;
}

class Anomaly{
public:
    Anomaly(double _x, double _y, int _lifetime): x(_x), y(_y), lifetime(_lifetime){};
    double x;
    double y;
    int lifetime;
};

class Node{
public:
    Node(int _x, int _y, int _cost): x(_x), y(_y), cost(_cost){};
    int x;
    int y;
    int cost;
};

bool is_edge(int x, int y, int d){
    return (x>=1) && (x<2*d) && (y>=0) && (y<=2*d-2) &&((x+y)%2==1); 
}
bool is_node(int x, int y, int d){
    return (x>=0) && (x<=2*d) && (y>=0) && (y<=2*d-2) && (x%2==0) && (y%2==0); 
}

vector<int> run(int seed, double freq, int max_cycle, double anomaly_size, int anomaly_lifetime, int distance){

    int plane_size = 2 * distance;
    double margined_size = 4 * distance;
    double shift = (margined_size - plane_size) / 2;
    double margined_freq = freq * pow(margined_size / plane_size, 2);
    mt19937 mt(seed);
    poisson_distribution<> poisson(margined_freq);
    uniform_real_distribution<> urd(0, margined_size);

    vector<Anomaly> anomaly_list;
    bool update = true;
    vector<vector<int>> plane(distance*2-1, vector<int>(distance*2+1, 0));
    vector<vector<int>> search(distance*2-1, vector<int>(distance*2+1,0));
    vector<int> effective_dist;
    int last_effective_dist = distance;

    for(int cycle_count=0;cycle_count<max_cycle;++cycle_count){
        // generate anomlay
        int num_anomaly = poisson(mt);
        for(int ki=0;ki<num_anomaly;++ki){
            double x = urd(mt);
            double y = urd(mt);
            anomaly_list.push_back(Anomaly(x,y,anomaly_lifetime));
            update = true;
        }

        if(update){
            // mark plane
            for(int y=0;y<plane.size();++y){
                for(int x=0;x<plane[0].size();++x){
                    if(!is_edge(x,y,distance)) continue;
                    bool hit = false;
                    double px = x + shift;
                    double py = y + shift;
                    for(const auto& ano : anomaly_list){
                        //int dx = abs(ano.x-px);
                        //int dy = abs(ano.y-py);
                        if(ano.x <= px && px < ano.x+anomaly_size 
                        && ano.y <= py && py < ano.y+anomaly_size){
                            hit = true;
                            break;
                        }
                    }
                    if(hit){
                        plane[y][x]=1;
                    }else{                        
                        plane[y][x]=0;
                    }
                }
            }

            // search paths
            int INF = distance*10;
            for(int y=0;y<plane.size();++y){
                for(int x=0;x<plane[0].size();++x){
                    search[y][x] = INF;
                }
            }
            queue<Node> queue;
            for(int y=0;y<plane.size();y+=2){
                search[y][0] = 0;
                queue.push(Node(0,y,0));
            }
            while(!queue.empty()){
                auto node = queue.front();
                queue.pop();
                if(search[node.y][node.x] < node.cost)continue;
                int dx[] = {0,0,1};
                int dy[] = {1,-1,0};
                for(int i=0;i<3;++i){
                    int px = node.x + dx[i]*2;
                    int py = node.y + dy[i]*2;
                    int pcost = node.cost;
                    //cout << px << " " << py << " " << pcost << endl;
                    if(!is_node(px,py,distance)) continue;
                    if(plane[node.y+dy[i]][node.x+dx[i]]==0) pcost +=1;
                    if(search[py][px] <= pcost) continue;
                    search[py][px] = pcost;
                    queue.push(Node(px,py,pcost));
                }
            }
            last_effective_dist = INF;
            for(int y=0;y<plane.size();++y){
                last_effective_dist = min(last_effective_dist, search[y][2*distance]);
            }
            update = false;
        }
        effective_dist.push_back(last_effective_dist);

        // remove anomaly
        for(int ai=0;ai<anomaly_list.size();++ai){
            anomaly_list[ai].lifetime -= 1;
            if(anomaly_list[ai].lifetime<=0) update = true;
        }
        auto remove_itr = remove_if(anomaly_list.begin(), anomaly_list.end(), [](Anomaly a)->bool{ return a.lifetime<=0;});
        anomaly_list.erase(remove_itr, anomaly_list.end());

        // visualize anomaly
#ifdef _DEBUG
        bool debug = false;
        if(debug){
            cout << "cycle " << cycle_count << endl;
            /*
            for(const auto& ano : anomaly_list) cout << ano.x << " " << ano.y << " " << ano.lifetime << endl;
            */
            for(int y=0;y<plane.size();++y){
                for(int x=0;x<plane[0].size();++x){
                    if(is_node(x,y,distance)) cout << search[y][x] << " ";
                    else if(is_edge(x,y,distance)) {
                        if(plane[y][x]) cout << "* ";
                        else cout << ". ";
                    }
                    else cout << "  ";
                }
                cout << endl;
            }
            cout << endl;
            cout << last_effective_dist << endl;
        }
#endif
    }

    vector<int> counter(distance+1, 0);
    for(auto val : effective_dist){
        counter[val]+=1;
    }
    for(int i=0;i<counter.size();++i){
        //cout << i << " " << counter[i] << endl;
    }
    return counter;
}

double lp(double base, double p_pth, int d){
    return base * pow(p_pth, (d*1.+1)/2);
}
double lp_noq3de(double base, double p_pth, int d, int od){
    d = od - (od-d)*2;
    d = max(0,d);
    return base * pow(p_pth, (d*1.+1)/2);
}

pair<double,double> stat(vector<int> result, int distance, double lp_base, double p_pth) {
    double sum = 0;
    double sum_noq3de = 0;
    for (int i = distance; i >= 0; i--) {
        sum += lp(lp_base, p_pth, i) * result[i];
        sum_noq3de += lp_noq3de(lp_base, p_pth, i, distance) * result[i];
    }
    return make_pair(sum, sum_noq3de);
}

int main(int argc, char** argv){
    string filename = "result.txt";
    double anomaly_size = 0.25;
    int anomaly_lifetime = 3000;
    int distance = 9;
    double freq = 10;
    int max_cycle = 100000;
    double lp_base = 0.1;
    double p_pth = 0.1;

    if (argc > 1){
        if (argc != 9) {
            cerr << "invalid argument" << endl;
            exit(1);
        }
        filename = argv[1];
        anomaly_size = atof(argv[2]);
        anomaly_lifetime = atoi(argv[3]);
        distance = atoi(argv[4]);
        freq = atof(argv[5]);
        lp_base = atof(argv[6]);
        p_pth = atof(argv[7]);
        max_cycle = atoi(argv[8]);
    }

    random_device rd;
    int seed = rd();

    auto result = run(seed, freq, max_cycle, anomaly_size, anomaly_lifetime, distance);
    auto lps = stat(result, distance, lp_base, p_pth);
    double sum_q3de = lps.first;
    double sum_noq3de = lps.second;

    cout << anomaly_size
        << " " << anomaly_lifetime
        << " " << distance
        << " " << freq
        << " " << max_cycle
        << " " << sum_q3de / max_cycle
        << " " << sum_noq3de / max_cycle
        << endl;

    ofstream ofs(filename, ios::app);
    ofs << anomaly_size
        << " " << anomaly_lifetime
        << " " << distance
        << " " << freq
        << " " << max_cycle
        << " " << sum_q3de / max_cycle
        << " " << sum_noq3de / max_cycle
        << endl;
    for (int i = 0;i<result.size();++i) {
        ofs << i << " " << result[i] << " ";
    }
    ofs << endl;
    ofs.close();

    return 0;
}
