#include <mutex>
#include <pthread.h>
#include <queue>
#include <string>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <sstream>

#define QUEUE_CAPACITY 5

using namespace std;

typedef int semaphore;

class Legado {
    public: 
        string car_plate;
        string car_name;
        string car_model;
        string car_year;
        int actual_capacity; 
        int queue_capacity; //WAITING do codigo do barbeiro
        queue<int> q;

        Legado(int capacity) {
            this->queue_capacity=capacity;
            this->actual_capacity = 0;
        };

        int query_vehicle(string plate) {

            if (this->q.size() == queue_capacity) {
                throw std::invalid_argument("Fila cheia!");
            }    
            string name;
            string model;
            string year;

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

void request(int *id_thread, Legado** legado, semaphore* requests, semaphore* legado_api, semaphore* mutex, vector<vector<string>>* &return_data) {
    vector<string> car_data;
    down(&mutex);
    if (legado->actual_capacity < QUEUE_CAPACITY) {

        //Check-in
        legado->actual_capacity += 1;
        up(&requests); //Adiciona que tem uma request pra ser processada
        up(&mutex); //Libera a classe para que outras requests também solicitem
        
        down(&legado_api);
        
        //Get informations about this car
        car_data[0] = legado->car_plate;
        car_data[1] = legado->car_name;
        car_data[2] = legado->car_model;
        car_data[3] = legado->car_year;

        return_data[id_thread] = car_data; //Save the informations putting the data into the global vector in the right position
    } else { //Fila ja estava cheia, saindo e continuando sem fazer nada
        up(&mutex);
    }    
}

void procces_request(string plate, Legado* legado, semaphore requests, semaphore legado_api, semaphore mutex) {
    
    while (true) {
        down(&requests);
        down(&mutex);
        legado->actual_capacity -= 1;

        up(&legado_api);
        up(&mutex);

        //Analyse
        legado->query_vehicle(plate);
    }
}




//THREAD 4
int get_info(vector<string> data) {
    
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

    pthread_t thr_request[N_PLATES], thr_procces;

    //Creating the request threads
    for(int i=0; i<N_PLATES; i++) {
        pthread_create(&thr_request[i], NULL, &request, (&i, &legado, &requests, &legado_api, &mutex, &return_data));
    }

    pthread_create(&thr_procces, NULL, procces_request, NULL); //TODO: implementar parametros aqui

    for (int i=0; i<N_PLATES; i++) {
        pthread_join(thr_requests[i], NULL);
    }

    return return_data;
}



int main() {
    return 0;
}