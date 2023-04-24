#include "Legado.hpp"
#include <thread>

using namespace std;

typedef vector<tuple<string, int, int, bool, string, string, string>> carsData;

//Legado Class
Legado::Legado() {
    queueCount = 0;
    queueCapacity = 10;
}

Legado::Legado(int qC) {
    queueCount = 0;
    queueCapacity = qC;
}

string Legado::getPlate() {
    return this->plate;
}

string Legado::getModel() {
    return this->model;
}

string Legado::getYear() {
    return this->year;
}

string Legado::getName() {
    return this->name;
}

void Legado::queryAPI(string plate) {
    fstream file("legado_data.csv");
    vector<string> data;
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            if (line.substr(0, 5)==plate) {
                stringstream s(line);
                string word;
                while (getline(s, word, ',')) {
                    if (data.size() < 4)//checks if data vector isnt full -> should have at most 4 elements
                        data.push_back(word);
                }
                break;
            }
        }
    }
    if (data.size() < 4) {
        data.push_back("");
        data.push_back("");
        data.push_back("");
        data.push_back("");
    }
    this->plate = data[0];
    this->name = data[1];
    this->model = data[2];
    this->year = data[3];
}

void Legado::request(string plate, vector<vector<string>> &list_data, int i, mutex &m) {
    vector<string> data; //Create the vector that will be returned
    this->queueMutex.lock(); //Lock the queue mutex as we are going to access and modify the queue
    if (this->queueCount < this->queueCapacity) {
        cout << "Requesting data for plate: " << plate << endl;
        this->queueCount++;
        this->queueMutex.unlock();

        this->isExecutingMutex.lock();//Only one thread can be executing the queryAPI function at a time
        this->queryAPI(plate); //Simulates the legacy system querying the API
        data.push_back(getModel());
        data.push_back(getYear());
        data.push_back(getName());
        this->isExecutingMutex.unlock(); //Unlock the mutex so that other threads can execute the queryAPI function

        this->queueMutex.lock(); //Locks the queue mutex so we can remove the done request from the queue
        this->queueCount--;
        this->queueMutex.unlock();
    }
    else {
        this->queueMutex.unlock(); //Unlock the queue mutex as we are done accessing and modifying the queue
        data.push_back("");//If the queue is full then we return an empty vector
        data.push_back("");
        data.push_back("");
    }
        m.lock(); //Lock the mutex so we can access and modify the list_data vector
        //cout << "Returning data for plate: " << i << endl;
        list_data[i] = data; //Add the data to the list_data vector
        m.unlock(); //Unlock the mutex so other threads can access and modify the list_data vector
    }




