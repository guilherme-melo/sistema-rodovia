#include <pthread.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <mutex>
#include <iostream>
#include <condition_variable>

#define QUEUE_CAPACITY 10

using namespace std;

class Semaphore {
    const size_t num_permissions;
    size_t avail;
    std::mutex m;
    std::condition_variable cv;
    public:
        explicit Semaphore(const size_t& num_permissions = 1) : num_permissions(num_permissions), avail(num_permissions) { }
        Semaphore(const Semaphore& s) : num_permissions(s.num_permissions), avail(s.avail) { }
        void up();
        void down();
        size_t available() const {
            return avail;
        }
};

class Legado {
    public:
        string car_plate;
        string car_name;
        string car_model;
        string car_year;
        int actual_capacity; //WAITING do codigo do barbeiro
        int queue_capacity;
        Legado(int capacity);

        void query_vehicle(string plate);

        string get_name();
        string get_model();
        string get_year();

};

//Struct for passing multiple arguments into threads
struct arg_struct {
    string plate;
    int id_thread_;
    Legado* legado_ ;
    Semaphore* requests_;
    Semaphore* legado_api_;
    Semaphore* mutex_;
    vector<vector<string>>* return_data_;
};

void * request(void *arguments);

void * procces_request(void *arguments);

vector<vector<string>> get_info(vector<string> data);