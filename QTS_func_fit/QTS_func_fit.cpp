#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <fstream>

using namespace std;

//f() = 7x1-13x2+30

const int dimension = 3;
const int target_weight[dimension] = {7, -13, 30}; //{w1, w2, w3}
const int training_data_size = 500;
const int testing_data_size = 250;
const int Cycle = 150;
const int population_size = 10;
const float theta = 0.01;
const int Experiment_times = 10;
const int seed = 114;
const int data_value_range[2] = {-100, 100};
const float weight_bound[2] = {-100, 100};
float Particles[population_size][dimension+1]; //last positon put fitness
float Q_matrix[dimension][2];
float Gbest[dimension+1];
int training_data_value[training_data_size][dimension];
int training_data_answer[training_data_size];
int testing_data_value[testing_data_size][dimension];
int testing_data_answer[testing_data_size];
int Best_Worst_Index[2]; //{best, worst}

void training_data_generation(){
    int ans;
    for(int i = 0; i < training_data_size; i++){
        ans = 0;
        for(int j = 0; j < dimension-1; j++){
            training_data_value[i][j] = (float(rand())/float(RAND_MAX)*(data_value_range[1]-data_value_range[0]))+data_value_range[0];
            ans += training_data_value[i][j]*target_weight[j];
        }
        ans += target_weight[dimension-1];
        if(ans >= 0)
            training_data_answer[i] = 1;
        else
            training_data_answer[i] = 0;
    }
}

void testing_data_generation(){
    int ans;
    for(int i = 0; i < testing_data_size; i++){
        ans = 0;
        for(int j = 0; j < dimension-1; j++){
            testing_data_value[i][j] = (float(rand())/float(RAND_MAX)*(data_value_range[1]-data_value_range[0]))+data_value_range[0];
            ans += testing_data_value[i][j]*target_weight[j];
        }
        ans += target_weight[dimension-1];
        if(ans >= 0)
            testing_data_answer[i] = 1;
        else
            testing_data_answer[i] = 0;
    }
}

void testing_dataset_write(){
    ofstream file;
    file.open("testing_dataset.csv");
    file << "x1,x2,f(x)" << endl;
    for(int i = 0; i < testing_data_size; i++){
        for(int j = 0; j < dimension-1; j++){
            file << testing_data_value[i][j] << ",";
        }
        file << testing_data_answer[i] << endl;
    }
}

void training_dataset_write(){
    ofstream file;
    file.open("training_dataset.csv");
    file << "x1,x2,f(x)" << endl;
    for(int i = 0; i < training_data_size; i++){
        for(int j = 0; j < dimension-1; j++){
            file << training_data_value[i][j] << ",";
        }
        file << training_data_answer[i] << endl;
    }
}

void init(){
    for(int i = 0; i < dimension; i++){
        Q_matrix[i][0] = 0;
        Q_matrix[i][1] = 1;
    }
    Gbest[dimension] = 0;
}

void generate_new_particles(){
    float Rand;
    for(int i = 0; i < population_size; i++){
        for(int j = 0; j < dimension; j++){
            Rand = (float(rand())/float(RAND_MAX))*(Q_matrix[j][1]-Q_matrix[j][0])+Q_matrix[j][0]; // [0, 1]
            Particles[i][j] = Rand*(weight_bound[1]-weight_bound[0])+weight_bound[0];
        }
    }
}

void fitness(){ //training dataset acc
    float acc;
    float temp;
    for(int i = 0; i < population_size; i++){
        acc = 0;
        for(int j = 0; j < training_data_size; j++){
            temp = 0;
            for(int k = 0; k < dimension-1; k++){
                temp += Particles[i][k]*training_data_value[j][k];
            }
            temp += Particles[i][dimension-1];
            if(temp >= 0)
                temp = 1;
            else
                temp = 0;
            if(temp == training_data_answer[j])
                acc++;
        }
        acc /= training_data_size;
        Particles[i][dimension] = acc;
    }
}

void selection(){
    float best_temp = -1, worst_temp = 2;
    Best_Worst_Index[0] = -1;
    Best_Worst_Index[1] = -1;
    for(int i = 0; i < population_size; i++){
        if(Particles[i][dimension] > best_temp){
            best_temp = Particles[i][dimension];
            Best_Worst_Index[0] = i;
        }
        if(Particles[i][dimension] < worst_temp){
            worst_temp = Particles[i][dimension];
            Best_Worst_Index[1] = i;
        }
    }
}

void update(){
    for(int i = 0; i < dimension; i++){
        if(Particles[Best_Worst_Index[0]][i] > Particles[Best_Worst_Index[1]][i])
            Q_matrix[i][0] += theta;
        else if(Particles[Best_Worst_Index[0]][i] < Particles[Best_Worst_Index[1]][i])
            Q_matrix[i][1] -= theta;
    }
}

float test_acc(){
    float acc = 0;
    float temp;
    for(int j = 0; j < testing_data_size; j++){
        temp = 0;
        for(int k = 0; k < dimension-1; k++){
            temp += Gbest[k]*testing_data_value[j][k];
        }
        temp += Gbest[dimension-1];
        if(temp >= 0)
            temp = 1;
        else
            temp = 0;
        if(temp == testing_data_answer[j])
            acc++;
    }
    acc /= testing_data_size;
    cout << "ACC:" << acc << endl;
    return acc;
}

int main(){
    float avg_acc = 0;
    srand(seed);
    training_data_generation();
    testing_data_generation();
    training_dataset_write();
    testing_dataset_write();
    /*
    int count[2] = {0};
    for(int i = 0; i < 100; i++){
        count[training_data_answer[i]]++;
    }
    cout << count[0] << endl;
    cout << count[1] << endl;
    */
   for(int times = 0; times < Experiment_times; times++){
        init();
        for(int G = 0; G < Cycle; G++){
            generate_new_particles();
            fitness();
            selection();
            update();
            //update Global best
            if(Particles[Best_Worst_Index[0]][dimension] > Gbest[dimension]){
                for(int i = 0; i < dimension+1; i++)
                    Gbest[i] = Particles[Best_Worst_Index[0]][i];
            }
        }
        cout << "Experiment " << times << endl;
        avg_acc += test_acc();
   }
   avg_acc /= Experiment_times;
   cout << "Average Accruacy" << avg_acc << endl;
    
    /*
    cout << "Fitness" << endl;
    for(int i = 0; i < population_size; i++){
        cout << i << " " << Particles[i][dimension] << endl;
    } 
    cout << Best_Worst_Index[0] << endl;
    cout << Best_Worst_Index[1] << endl;
*/
    cout << "Gbest:" << endl;
    for(int i = 0; i < dimension+1; i++)
        cout << Gbest[i] << " ";
    cout << endl;
    test_acc();
    
    
    return 0;
}