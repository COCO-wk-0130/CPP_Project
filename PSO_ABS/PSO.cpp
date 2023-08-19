#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <fstream>

using namespace std;

//Function to Solute：f(x) = |x1|+|x2|, 找最小
//參數設定
const int Dim = 2; //dimension
const int particle_num = 30;    //粒子數量
const int MAX_Cycle = 1000;  //執行次數
const int experiment_num = 100;
const float W = 0.1;
const float C1 = 0.8, C2 = 1.4;
const float x_range[2] = {-100, 100}; //值域上下界
const int seed = 114; //亂數產生種子
const float volocity_range[2] = {-35, 35};
//變數設定
float particle_position[particle_num][Dim];  //current position
float particle_volocity[particle_num][Dim] = {0};  //current volocity
float particle_fit[particle_num];
float gbest_position[Dim];    //global best positon
float gbest_fit = 201;  //global best fitness
float sbest_position[particle_num][Dim];    //social best position
float sbest_fit[particle_num];  //social best fitness




//初始化
void init(){
    //設定粒子初始位置
    for(int i = 0; i < particle_num; i++){
        for(int j = 0; j < Dim; j++){
            float random_position = (float(rand())/float(RAND_MAX)*(x_range[1]-x_range[0]))+x_range[0];
            particle_position[i][j] = random_position;
            sbest_position[i][j] = particle_position[i][j];
            particle_volocity[i][j] = 0;
        }
        sbest_fit[i] = 201;
    }
    gbest_fit = 201;
    gbest_position[0] = 101;
    gbest_position[1] = 101;
}

//計算Fitness
void fitness(){
    for(int i = 0; i < particle_num; i++){
        particle_fit[i] = abs(particle_position[i][0])+abs(particle_position[i][1]);
    }
}

//更新自己與歷史的最佳
void update_best(){
    fitness();
    for(int i = 0; i < particle_num; i++){
        if(particle_fit[i] < sbest_fit[i]){ //update social
            sbest_fit[i] = particle_fit[i];
            for(int j = 0; j < Dim; j++)
                sbest_position[i][j] = particle_position[i][j];
            if(sbest_fit[i] < gbest_fit){   //update global
                gbest_fit = sbest_fit[i];
                for(int j = 0; j < Dim; j++)
                    gbest_position[j] = sbest_position[i][j];
            }
        }
    }
}

//計算粒子接下來的速度
void calculate_velocity(){
    for(int i = 0; i < particle_num; i++){
        for(int j = 0; j < Dim; j++){
            float R1 = float(rand())/float(RAND_MAX);
            float R2 = float(rand())/float(RAND_MAX);
            //float R3 = float(rand())/float(RAND_MAX);
            particle_volocity[i][j] = W*particle_volocity[i][j]+C1*R1*(sbest_position[i][j]-particle_position[i][j])+C2*R2*(gbest_position[j]-particle_position[i][j]);
            if(particle_volocity[i][j] < volocity_range[0]){
                particle_volocity[i][j] = volocity_range[0];
            }
            else{
                if(particle_volocity[i][j] > volocity_range[1])
                    particle_volocity[i][j] = volocity_range[1];
            }
        }
    }
}

//更新粒子的位置
void update_position(){
    for(int i = 0; i < particle_num; i++){
        for(int j = 0; j < Dim; j++){
            particle_position[i][j] += particle_volocity[i][j];
            //確認更新後的值沒有超過限制
            if(particle_position[i][j] > x_range[1])
                particle_position[i][j] = x_range[1];
            else{
                if(particle_position[i][j] < x_range[0])
                    particle_position[i][j] = x_range[0];
            }
        }
    }
}

int main(){
    srand(seed);    //設定亂數種子
    ofstream file;
    string file_name = "./EpinFile/PSO_Experiment";
    double avg_best = 0;
    int best_count = 0;
    float avg_cycle =0;
    for(int n = 0; n < experiment_num; n++){
        file.open(file_name+to_string(n)+".epin");
        file << "Dimension : 2" << endl;
        file << "Formula : sum(|Xi|)" << endl;
        file << "BP : 0,0" << endl;
        file << "Range : -100 ~ 100" << endl;
        file << "Position : " << endl;
        init();
        file << "*" << 0 << ",";
        file << "Particle=" << particle_num << ", ";
        file << "W=" << W << ", ";
        file << "C1=" << C1 << ", ";
        file << "C2=" << C2 << ", ";
        file << "Gbest_X1=" << gbest_position[0] << ", ";
        file << "Gbest_X2=" << gbest_position[1] << " ";
        file << gbest_fit << ":";
        for(int i = 0; i < particle_num; i++){
            file << " " << particle_position[i][0] << "," << particle_position[i][1] << ",";
            if(particle_fit[i] == gbest_fit){
                file << "9,0,2";
            }
            else
                file << "8,0,2";
        }
        file << endl;
        for(int G = 0; G < MAX_Cycle; G++){
            file << "*" << G+1 << ",";
            file << "Particle=" << particle_num << ", ";
            file << "W=" << W << ", ";
            file << "C1=" << C1 << ", ";
            file << "C2=" << C2 << ", ";
            update_best();
            calculate_velocity();
            update_position();
            file << "Gbest_X1=" << gbest_position[0] << ", ";
            file << "Gbest_X2=" << gbest_position[1] << " ";
            file << gbest_fit << ":";
            for(int i = 0; i < particle_num; i++){
                file << " " << particle_position[i][0] << "," << particle_position[i][1] << ",";
                if(particle_fit[i] == gbest_fit){
                    file << "9,0,2";
                }
                else
                    file << "8,0,2";
            }
            file << endl;
            if(gbest_fit == 0){
                best_count++;
                avg_cycle+=G;
                break;
            }
        }
        avg_best += gbest_fit;
        //cout << "Experiment " << n << endl;
        //cout << "Best:" << endl;
        //cout << gbest_fit << endl;
        //cout << "Position:" << endl;
        //cout << gbest_position[0] << " " << gbest_position[1] << endl;
        file.close();
    }

    avg_best /= experiment_num;
    avg_cycle /= experiment_num;
    cout << "Average Best:" << avg_best << endl;
    cout << "Average best generation:" << avg_cycle << endl;
    cout << "Find zero:" << best_count << endl;

}