/*

SEQUENTIAL CODE FOR SPARSE ROW MATRIX MULTIPLICATION

Date: 31/10/2025

*/

#include <iostream>
#include <vector>
#include <random>
#include <chrono>  
#include <string>
#include <ctime>
#include <time.h>
#include "csr.h"

using namespace std;

int main(int argc, char** argv) {

    if (argc <= 1) {
        cerr << "[ERR]: You must provide a file" << endl;
        cout << "Usage: " << string(argv[0]) << " file [-v]" << endl;
        exit(1);
    }

    // Checking for verbose mode
    bool verbose_on = false;
    if(argc >= 3) {
        if (string(argv[2]) == "-v") {
            verbose_on = true;
        } else {
            cout << "[WARN]: Unknown flag " << argv[2] << endl;
            cout << "Usage: ./" << string(argv[0]) << " file [-v]" << endl;
        }
    }

    // Random number generator
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    mt19937 rng(seed); 
    uniform_int_distribution<int> dist(1, 10); // Starting easy with [1,10] range


    if(verbose_on) {
        cout << "[INFO]: Starting simulation..." << endl;
        cout << "[INFO]: Debug mode enabled" << endl;
    }

    string file_name(argv[1]);

    if(verbose_on) {
        cout << "[INFO]: Importing file <" << file_name << ">" << endl;
        cout << "[INFO]: Compressing matrix..." << endl;
    }

    // Importing sparse-row matrix from file
    CompressedSparseRow csr (file_name);

    /* FOR DEBUG ONLY */
    //cout << "[DEBUG]: Printing compressed sparse row matrix" << endl;
    //cout << csr << endl;

    // Generating random vector
    size_t vector_size = csr.get_original_matrix_col_num();

    vector<double> vec(vector_size);

    if(verbose_on) {
        cout << "[INFO]: Generating vector of size " << vector_size << endl;
    }
    for (size_t i = 0; i < vector_size; i++) {
        vec[i] = dist(rng);
    }

    /* FOR DEGUB ONLY */
    // if(verbose_on) {
    //    cout << "[INFO]: vector [ ";
    //    for (double item : vec) {
    //        cout << item << " ";
    //    }
    //    cout << "]" << endl;
    // }

    // Multiplication
    if(verbose_on) {
        cout << "[INFO]: Starting computation..." << endl;
    }

    struct timespec start, end;

    clock_t cpu_start = clock();
    clock_gettime(CLOCK_MONOTONIC, &start);

    vector<double> result  = csr.multiply_to_vector(vec);

    clock_t cpu_end = clock();
    clock_gettime(CLOCK_MONOTONIC, &end);

    if(verbose_on) {
        cout << "[INFO]: Computation completed" << endl;
    }

    /* FOR DEBUG ONLY */
    // Showing multiplication result
    // if(verbose_on) {
    //     cout << "[INFO]:Result vector:" << endl;
    //     for (double item : result) {
    //         cout << item << " ";
    //     }
    //     cout << endl;
    // }

    double cpu_time = static_cast<double>(cpu_end - cpu_start) / CLOCKS_PER_SEC * 1e3;
    double real_time = (end.tv_sec - start.tv_sec) * 1e3 + (end.tv_nsec - start.tv_nsec) / 1e6;

    if(verbose_on) {
        cout << "[INFO]: Simulation metrics:" << endl;
        cout << "\tCPU  TIME: " << cpu_time << " ms" << endl;
        cout << "\tREAL TIME: " << real_time << "ms" << endl;
    } else {
        cout << file_name << ":" << cpu_time << ":" << real_time << endl;
    }
    
    if(verbose_on) {
        cout << "[INFO]: Simulation completed" << endl;
    }

    return 0;
}