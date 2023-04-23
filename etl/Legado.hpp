#include <pthread.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <mutex>
#include <iostream>
#include <condition_variable>
#include <queue>
#include <tuple>


#ifndef Legado_HPP
#define Legado_HPP

using namespace std;

class Legado {
//Legado is the legacy system that we are integrating with its analogue to a sleeping barbers problem with a single barber and QUEUE_CAPACITY waiting chairs
    private:
        string plate;
        string model;
        string year;
        string name;
    public:
        Legado();
        Legado(int queueCapacity);

        mutex isExecutingMutex;//mutex to control access to isExecuting
        mutex queueMutex;//mutex to control access to queue

        void queryAPI(string plate);

        void request(string plate, vector<vector<string>> &list_data, int i, mutex &m);


        //FIFO queue of strings
        int queueCount;
        int queueCapacity;

        string getPlate();
        string getModel();
        string getYear();
        string getName();
};

#endif