#include <pthread.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#define QUEUE_CAPACITY 10

using namespace std;

typedef int semaphore;

//THIS FUNCTIONS ARE TEMPORARY JUST FOR COMPILE
//TODO: IMPLEMENT SEMAPHORE CLASS/STRUCT WITH THIS TWO FUNCTIONS
void up(int *x) {
    x+=1;
}

void down(int *x) {
    x-=1;
}


//class from API Legado
class Legado {
    public: 

        string car_plate;
        string car_name;
        string car_model;
        string car_year;
        int actual_capacity; //WAITING do codigo do barbeiro
        int queue_capacity;

        Legado(int capacity) {
            this->queue_capacity=QUEUE_CAPACITY;
        };

        int query_vehicle(string plate) {
            // codigo para abrir o arquivo e procurar a placa
            vector<string> data;
            string line;
            fstream file("../data/legado_data.csv", ios::in);
            if(file.is_open()) {
                while(getline(file, line)) {
                    if (line.substr(0, 7)==plate) {
                        stringstream s(line);
                        string word;
                        while (getline(s, word, ',')) {
                            data.push_back(word); // adiciona placa, proprietario, modelo e ano (nesta ordem)
                        }
                    }    
                }
            }
            this->car_plate = data[0];
            this->car_name = data[1];
            this->car_model = data[2];
            this->car_year = data[3];
            return 0;
        }

        string get_name() {
            return this->car_name;
        }

        string get_model() {
            return this->car_model;
        }

        string get_year() {
            return this->car_year;
        }

};

//Struct for passing multiple arguments into threads
struct arg_struct {
    string plate;
    int id_thread_;
    Legado* legado_ ;
    semaphore* requests_;
    semaphore* legado_api_;
    semaphore* mutex_;
    vector<vector<string>>* return_data_;
};

//From here, we begin our implementation of sleeping barber

//Client function
void * request(void *arguments) {
    struct arg_struct *args = (struct arg_struct *) arguments;
    vector<string> car_data;
    down(args->mutex_);
    if ((args->legado_)->actual_capacity < args->legado_->queue_capacity) {

        //Check-in
        args->legado_->actual_capacity += 1;
        up(args->requests_); //Adiciona que tem uma request pra ser processada
        up(args->mutex_); //Libera a classe para que outras requests também solicitem
        
        down(args->legado_api_);
        
        //Get informations about this car
        car_data[0] = args->legado_->car_plate;
        car_data[1] = args->legado_->car_name;
        car_data[2] = args->legado_->car_model;
        car_data[3] = args->legado_->car_year;
        args->return_data_->at(args->id_thread_) = car_data; //Save the informations putting the data into the global vector in the right position
    } else { //Fila ja estava cheia, saindo e continuando sem fazer nada
        up(args->mutex_); //Para esses que não tiveram os dados, o array vai ficar vazio.
    }
}

//Barber function
void * procces_request(void *arguments) {
    struct arg_struct *args = (struct arg_struct *) arguments;
    while (true) {
        down(args->requests_);
        down(args->mutex_);
        args->legado_->actual_capacity -= 1;

        up(args->legado_api_);
        up(args->mutex_);

        //Analyse
        args->legado_->query_vehicle(args->plate);
    }
}

//THREAD 4 with sleeping barber
vector<vector<string>> get_info(vector<string> data) {
    
    //Get vector with unique plates from the data
    vector<string> plates;
    for (string line : data) {
        plates.push_back(line.substr(0,7));
    }

    //Get number of plates in this data (it will be the number of threads);
    int N_PLATES = plates.size();

    vector<vector<string>> return_data;

    Legado* legado = new Legado(QUEUE_CAPACITY);

    semaphore requests = 0;
    semaphore legado_api = 0;
    semaphore mutex = 1;

    arg_struct args = {
        "init",
        0,
        legado,
        &requests,
        &legado_api,
        &mutex,
        &return_data
    };

    pthread_t thr_request[N_PLATES], thr_proccess;

    //Creating the request threads
    for(int i=0; i<N_PLATES; i++) {
        args.id_thread_ = i;
        args.plate = plates[i];
        pthread_create(&thr_request[i], NULL, request, &args);
    }

    pthread_create(&thr_proccess, NULL, procces_request, &args); //TODO: implementar parametros aqui

    for (int i=0; i<N_PLATES; i++) {
        pthread_join(thr_request[i], NULL);
    }

    return return_data;
}

int main() {
    return 0;
}