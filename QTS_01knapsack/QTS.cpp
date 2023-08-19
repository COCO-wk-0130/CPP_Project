#include <iostream>
#include <stdlib.h>
#include <time.h>

using namespace std;

const int max_weight = 275; //最大重量
const int num_generation = 100; //最大世代
const int population_size = 10;//族群數量
const int experiment_times = 10;//實驗次數
const int theta = 10;//機率修改幅度
const int seed = 114;//隨機函式的種子碼
const int probability_bound[2] = {0, 100};//機率的上下限
const bool crispy = true;//是否使用鹹酥雞法
const bool second_chance = true;//使否使用second chance
int item_W_V[10][2]; //物品的重量及價值
int item_pick_rate[100]; //物品被拿起的機率
int pack[101][population_size]; //裝物品的背包，pack[100][k]拿來放fitness
int MAX[2], MIN[2];//找最佳及最差g
int best_pick[13];//每個世代最好的組合，[10~12]存weight, value, generation
int best_count[3][experiment_times] = {0};//每次最好的weight, value, generation
double best_average[3] = {0};//最佳平均

//資料初始化
void item_init(){
    for(int i = 0; i < 10; i++){
        item_W_V[i][0] = i+1;
        item_W_V[i][1] = i+6;
    }
    //每項物品得初始機率皆為50%
    for(int i = 0; i < 100; i++)
        item_pick_rate[i] = 50;
    for(int i = 0; i < 13; i++)
        best_pick[i] = 0;
}

//藉由亂數決定是否拿取物品
void pick_item(int k){
    double pick_rate;
    int weight = 0;
    bool full = !second_chance;
    for(int i = 0; i < 100; i++){
        pick_rate = rand()%100;
        if(item_pick_rate[i] >= pick_rate){
            if(crispy){
                weight+=item_W_V[i/10][0];
                if(weight > max_weight){
                    for(int j = i; j < 100; j++){
                        pack[j][k] = 0;
                    }
                    full = true;
                    break;
                }
            }
            pack[i][k] = 1;
        }
        else
            pack[i][k] = 0;
    }
    //Second Chance
    if(!full){
        for(int i = 0; i < 100; i++){
            if(pack[i][k] == 1)
                continue;
            pick_rate = (rand()%100)-10;
            if(item_pick_rate[i] >= pick_rate){
                weight+=item_W_V[i/10][0];
                if(weight > max_weight){
                    full = true;
                    break;
                }
                pack[i][k] = 1;
            }
        }
    }
}

//計算每個包的總價值
//重量超出上限的value設為-1
void fitness(int k){
    int weight = 0;
    pack[100][k] = 0;
    for(int i = 0; i < 100; i++){
        if(pack[i][k] == 1){
            weight+=item_W_V[i/10][0];
            pack[100][k]+=item_W_V[i/10][1];
        }
    }
    if(weight > max_weight){
        pack[100][k] = -1;
    }
}

//挑出最佳及最差的組合
void best_worst(){
    MAX[0] = 0;
    MAX[1] = -1;
    MIN[0] = 1050;
    MIN[1] = -1;
    for(int k = 0; k < population_size; k++){
        //遇到超過重量上限的直接跳過
        if(pack[100][k] == -1)
        {
            continue;
        }
        else{
            if(pack[100][k] < MIN[0]){
                MIN[0] = pack[100][k];
                MIN[1] = k;
            }
            if(pack[100][k] > MAX[0]){
                MAX[0] = pack[100][k];
                MAX[1] = k;
            }
        }
    }
}

//更新每個物品被拿的機率
void update(int best, int worst){
    for(int i = 0; i < 100; i++){
        if(pack[i][best] == pack[i][worst]){
            continue;
        }
        else{
            if(pack[i][best] == 1 && item_pick_rate[i] < probability_bound[1])
                item_pick_rate[i]+=theta;
            else if(item_pick_rate[i] > probability_bound[0])
                item_pick_rate[i]-=theta;
        }
    }
}

int main()
{
    //增加隨機性
    srand(seed);
    for(int times = 0; times < experiment_times; times++){
        item_init();
        for(int G = 0; G < num_generation; G++){
            //cout << "===================Generation " << G+1 << "===================" << endl;
            for(int k = 0; k < population_size; k++){
                pick_item(k);
                fitness(k);
            }
            best_worst();
            if(MAX[0] > best_pick[11]){
                best_pick[11] = MAX[0];
                best_pick[12] = G+1;
                for(int i = 0; i < 11; i++)
                    best_pick[i] = 0;
                for(int i = 0; i < 100; i++){
                    if(pack[i][MAX[1]] == 1){
                        best_pick[i/10]+=1;
                        best_pick[10]+=item_W_V[i/10][0];
                    }
                }
                if(best_pick[11] == 620)
                    break;
                for(int i = 0; i < 10; i++){
                    //cout << "Item " << i+1 << ": " << best_pick[i] << endl;
                }
                //cout << "Weight: " << best_pick[10] << endl;
            }
            update(MAX[1], MIN[1]);
            //cout << "Current best:" << best_pick[11] << endl;
        }
        for(int i = 0; i < 3; i++)
            best_count[i][times] = best_pick[i+10];
        cout << "===================TIMES " << times+1<< "==================="<< endl;
        cout << "Best Generation:" << best_pick[12] << endl;
        cout << "Best Weight:" << best_pick[10] << endl;
        cout << "Best Value:" << best_pick[11] << endl;
        cout << "Best Pick:" << endl;
        for(int i = 0; i < 10; i++){
            cout << "Item " << i+1 << ": " << best_pick[i] << endl;
        }
    }
    for(int i = 0; i < experiment_times; i++){
        for(int j = 0; j < 3; j++)
            best_average[j] +=best_count[j][i];
    }
    for(int j = 0; j < 3; j++)
        best_average[j] /=experiment_times;
    cout << "Average Generation:" << best_average[2] << endl;
    cout << "Average Weight:" << best_average[0] << endl;
    cout << "Average Value:" << best_average[1] << endl;

    return 0;
}
