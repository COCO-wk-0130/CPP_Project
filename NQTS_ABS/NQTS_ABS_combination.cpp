#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <sstream>
#include <fstream>
#include <math.h>

using namespace std;

const int dimension = 5; //解維度
const int Cycle = 2000; //迭代次數
const int population_size = 30;//族群大小
const int experiment_times = 10;//實驗次數
const float theta = 1;//旋轉角度
const int seed = 114;//亂數種子
const int probability_bound[2] = {0, 100};//機率區間
const int data_value_range[2] = {-100, 100};//值域區間
const int float_bit = 32;
const int Qbit_num = dimension*float_bit;
int Population[population_size][Qbit_num];//族群
double Population_dec_fitness[population_size][dimension+1];
float Q_matrix[Qbit_num];
int Gbest_bin[Qbit_num];
double Gbest_dec[dimension+1];
int Best_Worst_Index[2]; //{best, worst}


void init(){
    for(int i = 0; i < population_size; i++){
        for(int j = 0; j < Qbit_num; j++){
            Population[i][j] = 0;
        }
    }
    for(int i = 0; i < Qbit_num; i++){
        Gbest_bin[i] = 0;
        Q_matrix[i] = 50;
    }
    for(int i = 0; i < dimension+1; i++)
        Gbest_dec[i] = 101;
}

//return[0, 1]
float Random(){
    return float(rand())/float(RAND_MAX);
}

void generate_new_population(){
    for(int i = 0; i < population_size; i++){
        for(int j = 0; j < Qbit_num; j++){
            Population[i][j] = Random()*100 > Q_matrix[j];
        }
    }
}

void IEEE754(){
    int sign, Exponent;
    float fraction;
    for(int i = 0; i < population_size; i++){
        for(int w = 0; w < 3; w++){
            //sign
            sign = pow(-1, Population[i][0+w*float_bit]);
            //Exponent
            Exponent = 0;
            //exp_bit0~6 left->right(跟一般書寫相反)
            for(int j = 1; j < 8; j++){
                Exponent += pow(2, j)*Population[i][1+j+w*float_bit];
            }
            Exponent *= pow(-1, Population[i][1+w*float_bit]);  
            //fraction
            fraction = 0;
            for(int j = 0; j < float_bit-9; j++){
                fraction += pow(0.5, j+1)*Population[i][9+j+w*float_bit];
            }
            Population_dec_fitness[i][w] = fraction*pow(2, Exponent)*sign;
            if(Population_dec_fitness[i][w] > data_value_range[1])
                Population_dec_fitness[i][w] = data_value_range[1];
            else if(Population_dec_fitness[i][w] < data_value_range[0])
                Population_dec_fitness[i][w] = data_value_range[0];
        }
    }
}

void Bin2Dec(){
    int sign;
    float temp;
    for(int i = 0; i < population_size; i++){
        for(int w = 0; w < dimension; w++){
            temp = 0;
            sign = pow(-1, Population[i][0+w*float_bit]);
            for(int j = 0; j < float_bit-1; j++){
                temp += pow(0.5, j)*Population[i][1+j+w*float_bit];
            }
            Population_dec_fitness[i][w] = temp*pow(2, 6)*sign;
            /*
            if(Population_dec_fitness[i][w] > data_value_range[1])
                Population_dec_fitness[i][w] = data_value_range[1];
            else if(Population_dec_fitness[i][w] < data_value_range[0])
                Population_dec_fitness[i][w] = data_value_range[0];
            */
            
        }
        
    }
}

void fitness(){
    for(int i = 0; i < population_size; i++){
        Population_dec_fitness[i][dimension] = 0;
        for(int j = 0; j < dimension; j++){
            Population_dec_fitness[i][dimension] += abs(Population_dec_fitness[i][j]);
        }
    }
}

void selection(){
    float best_temp = dimension*100+1, worst_temp = -1;
    Best_Worst_Index[0] = -1;
    Best_Worst_Index[1] = -1;
    for(int i = 0; i < population_size; i++){
        if(Population_dec_fitness[i][dimension] < best_temp){
            best_temp = Population_dec_fitness[i][dimension];
            Best_Worst_Index[0] = i;
        }
        if(Population_dec_fitness[i][dimension] > worst_temp){
            worst_temp = Population_dec_fitness[i][dimension];
            Best_Worst_Index[1] = i;
        }
    }
} 

void update(){
    for(int i = 0; i < Qbit_num; i++){
        if(Population[Best_Worst_Index[0]][i] != Population[Best_Worst_Index[1]][i]){
            if(Population[Best_Worst_Index[0]][i] == 1)
                Q_matrix[i] += theta;
            else
                Q_matrix[i] -= theta;
        }
    }
}

void update_Gbest(){
    for(int i = 0; i < Qbit_num; i++){
        if(Gbest_bin[i] != Population[Best_Worst_Index[1]][i]){
            if(Gbest_bin[i] == 1)
                Q_matrix[i] += theta;
            else
                Q_matrix[i] -= theta;
        }
    }
}

int main(){
    ofstream file;
    string file_name = "./EpinFile/NQTS_ABS_Combination/NQTS_ABS_Experiment";
    float avg_best = 0;
    float avg_G = 0;
    srand(seed);
    for(int times = 0; times < experiment_times; times++){
        file.open(file_name+to_string(times+1)+".epin");
        file << "Particle :" << endl;
        init();
        for(int G = 0; G < Cycle; G++){
            file << "*" << G+1 << " ";
            generate_new_population();
            IEEE754();
            fitness();
            //update Gbest
            if(Population_dec_fitness[Best_Worst_Index[0]][dimension] < Gbest_dec[dimension]){
                for(int d = 0; d < dimension+1; d++)
                    Gbest_dec[d] = Population_dec_fitness[Best_Worst_Index[0]][d];
                for(int q = 0; q < Qbit_num; q++)
                    Gbest_bin[q] = Population[Best_Worst_Index[0]][q];
                if(Gbest_dec[dimension] == 0){
                    avg_G += G;
                }
            }
            file << Gbest_dec[dimension] << ":";
            for(int i = 0; i < population_size; i++){
                file << " " << Population_dec_fitness[i][dimension];
                /*
                for(int j = 0;j < dimension; j++){
                    file << " " << Population_dec_fitness[i][j];
                }
                */
                
                for(int j = 0; j < Qbit_num; j++){
                    file << " " << Population[i][j] << ",";
                    file << Q_matrix[j]/100 << ",";
                    file << "bit" << j+1;
                }
                file << "/";
            }
            selection();
            update();
            file << endl;
        }
        file.close();
        cout << "----------" << times+1 << "----------" << endl;
        cout << "Gbest:" << endl;
        for(int i = 0; i < dimension; i++){
            cout << Gbest_dec[i] << " ";
        }
        cout << endl;
        cout << Gbest_dec[dimension] << endl;
        avg_best += Gbest_dec[dimension];
    }
    avg_best /= experiment_times;
    avg_G /= experiment_times;
    cout << "Average Best:" << avg_best << endl;
    cout << "Average Generation:" << avg_G << endl;
    
    /*
    for(int q = 0; q < Qbit_num; q++)
        cout << Gbest_bin[q] << " ";
    cout << endl;
    */
    
    
}
