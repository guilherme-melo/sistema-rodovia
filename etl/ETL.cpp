#include <pthread.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <mutex>
#include <iostream>
#include <condition_variable>
#include "ETL.hpp"

#define QUEUE_CAPACITY 10

using namespace std;

/** General semaphore with N permissions **/

void Semaphore::up() {
std::unique_lock<std::mutex> lk(m);
cv.wait(lk, [this] { return avail > 0; });
avail--;
lk.unlock();
}

void Semaphore::down() {
    m.lock();
    avail++;
    m.unlock();
    cv.notify_one();
}

//class from API Legado

Legado::Legado(int capacity) {
    this->queue_capacity=QUEUE_CAPACITY;
};

void Legado::query_vehicle(string plate) {
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
}

string Legado::get_name() {
    return this->car_name;
}

string Legado::get_model() {
    return this->car_model;
}

string Legado::get_year() {
    return this->car_year;
}

//From here, we begin our implementation of sleeping barber

//Client function
void * request(void *arguments) {
    struct arg_struct *args = (struct arg_struct *) arguments;
    vector<string> car_data;
    args->mutex_->down();
    if ((args->legado_)->actual_capacity < args->legado_->queue_capacity) {

        //Check-in
        args->legado_->actual_capacity += 1;
        args->requests_->up(); //Adiciona que tem uma request pra ser processada
        args->mutex_->up(); //Libera a classe para que outras requests também solicitem

        args->legado_api_->down();

        //Get informations about this car
        car_data[0] = args->legado_->car_plate;
        car_data[1] = args->legado_->car_name;
        car_data[2] = args->legado_->car_model;
        car_data[3] = args->legado_->car_year;
        args->return_data_->at(args->id_thread_) = car_data; //Save the informations putting the data into the global vector in the right position
    } else { //Fila ja estava cheia, saindo e continuando sem fazer nada
        args->mutex_->up(); //Para esses que não tiveram os dados, o array vai ficar vazio.
    }
}

//Barber function
void * procces_request(void *arguments) {
    struct arg_struct *args = (struct arg_struct *) arguments;
    while (true) {
        args->requests_->down();
        args->mutex_->down();
        args->legado_->actual_capacity -= 1;

        args->legado_api_->up();
        args->mutex_->up();

        //Analyse
        args->legado_->query_vehicle(args->plate);
    }
}

//THREAD 4 with sleeping barber
vector<vector<string>> get_info(string data[]) {
    cout <<"aa";
    
    //Get vector with unique plates from the data
    vector<string> plates;
    for (int i=0; i<data->size(); i++) {
        plates.push_back(data[i].substr(0,7));
    }

    //Get number of plates in this data (it will be the number of threads);
    int N_PLATES = plates.size();

    vector<vector<string>> return_data;

    Legado* legado = new Legado(QUEUE_CAPACITY);

    Semaphore* requests = new Semaphore(0);
    Semaphore* legado_api = new Semaphore(0);
    Semaphore* mutex = new Semaphore(1);

    arg_struct args = {
        "init",
        0,
        legado,
        requests,
        legado_api,
        mutex,
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
    string plates[] ={"39AD932", "MGE5X36"};
    string line;
    ifstream file("./data/29_mockdata.txt");
    if (file.is_open()){
        while(getline(file, line)) {
        }
    }

    //Thread 1

    //Thread 2

    //Thread 3

    //Thread 4
    vector<vector<string>> info_add_plates = get_info(plates);
    for (int i=0; i<info_add_plates.size(); i++) {
        for (string info : info_add_plates[i]) {
            cout << info << endl;
        }
    }
    cout << "Fim do arquivo" << endl;

}