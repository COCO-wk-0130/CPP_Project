#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <sstream>
#include <fstream>
#include <math.h>

using namespace std;

const int dimension = 3; //解維度
const int target_weight[dimension] = {7, -13, 30}; //{w1, w2, w3}
const int Cycle = 500; //迭代次數
const int population_size = 10;//族群大小
const int experiment_times = 10;//實驗次數
const int theta = 2;//旋轉角度
const int seed = 114;//亂數種子
const int probability_bound[2] = {0, 100};//機率區間
const int data_value_range[2] = {-100, 100};//值域區間
const int training_data_size = 500;
const int testing_data_size = 250;
const int float_bit = 64;
const int Qbit_num = dimension*float_bit;
int Population[population_size][Qbit_num];//族群
float Population_dec_fitness[population_size][dimension+1];
float Q_matrix[Qbit_num];
int Gbest_bin[Qbit_num];
float Gbest_dec[dimension+1];
int Best_Worst_Index[2]; //{best, worst}
int training_data_value[training_data_size][dimension];//x1,x2,sign(f(x))
int testing_data_value[testing_data_size][dimension];

void read_data(){
    fstream file;
    string training_filename("training_dataset.csv");
    string testing_filename("testing_dataset.csv");
    string line;

    //read_training_data
    file.open(training_filename);
    getline( file, line,'\n'); //跳過資料類別(x1,x2,f(x))
    for(int i = 0; i < training_data_size; i++){
        getline( file, line,'\n');
        istringstream templine(line);
        string data;
	    for(int j = 0; j < dimension; j++)
	    {
            getline( templine, data,',');
	        training_data_value[i][j] = atof(data.c_str());  //string 轉換成數字
	    }
    }
    file.close();

    //read_training_data
    file.open(testing_filename);
    getline( file, line,'\n'); //跳過資料類別(x1,x2,f(x))
    for(int i = 0; i < testing_data_size; i++){
        getline( file, line,'\n');
        istringstream templine(line);
        string data;
	    for(int j = 0; j < 3; j++)
	    {
            getline( templine, data,',');
	        testing_data_value[i][j] = atof(data.c_str());  //string 轉換成數字
	    }
    }
    file.close();
}

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
        Gbest_dec[i] = 0;
}

//return[0, 1]
float Random(){
    return float(rand())/float(RAND_MAX);
}

void generate_new_population(){
    for(int i = 0; i < population_size; i++){
        for(int j = 0; j < Qbit_num; j++){
            Population[i][j] = Random()*100 < Q_matrix[j];
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
            for(int j = 1; j < 9; j++){
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

void fitness(){
    float acc, temp;
    for(int i = 0; i < population_size; i++){
        acc = 0;
        for(int j = 0; j < training_data_size; j++){
            temp = 0;
            for(int d = 0; d < dimension-1; d++){
                temp += Population_dec_fitness[i][d]*training_data_value[j][d];
            }
            temp += Population_dec_fitness[i][dimension-1];
            if(temp >= 0)
                temp = 1;
            else
                temp = 0;
            if(temp == training_data_value[j][dimension-1])
                acc++;
        }
        acc /= training_data_size;
        Population_dec_fitness[i][dimension] = acc;
    }
}

void selection(){
    float best_temp = -1, worst_temp = 2;
    Best_Worst_Index[0] = -1;
    Best_Worst_Index[1] = -1;
    for(int i = 0; i < population_size; i++){
        if(Population_dec_fitness[i][dimension] > best_temp){
            best_temp = Population_dec_fitness[i][dimension];
            Best_Worst_Index[0] = i;
        }
        if(Population_dec_fitness[i][dimension] < worst_temp){
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

float test_acc(){
    float acc = 0;
    float temp;
    int judge;
    for(int j = 0; j < testing_data_size; j++){
        temp = 0;
        for(int k = 0; k < dimension-1; k++){
            temp += Gbest_dec[k]*testing_data_value[j][k];
        }
        temp += Gbest_dec[dimension-1];
        if(temp >= 0)
            judge = 1;
        else
            judge = 0;
        if(judge == testing_data_value[j][dimension-1])
            acc++;
    }
    acc /= testing_data_size;
    cout << "Testing Accuracy:" << acc*100 << "%" << endl;
    return acc;
}

int main(){
    float avg_acc = 0;
    srand(seed);
    read_data();
    for(int times = 0; times < experiment_times; times++){
        init();
        for(int G = 0; G < Cycle; G++){
            generate_new_population();
            IEEE754();
            fitness();
            selection();
            update();
            //update Gbest
            if(Population_dec_fitness[Best_Worst_Index[0]][dimension] > Gbest_dec[dimension]){
                for(int d = 0; d < dimension+1; d++)
                    Gbest_dec[d] = Population_dec_fitness[Best_Worst_Index[0]][d];
                for(int q = 0; q < Qbit_num; q++)
                    Gbest_bin[q] = Population[Best_Worst_Index[0]][q];
            }
        }
        cout << "Gbest:" << endl;
        for(int i = 0; i < dimension; i++){
            cout << Gbest_dec[i] << " ";
        }
        cout << endl;
        cout << "Training Accruacy:" << Gbest_dec[dimension]*100 << "%" << endl;
        avg_acc += test_acc();
    }
    avg_acc /= experiment_times;
    cout << "Average Testing Accruacy:" << avg_acc*100 << "%" << endl;
    
}
