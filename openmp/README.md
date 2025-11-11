# SpMV optimization with OpenMP

## Overview

This project focuses on the optimization and performance benchmarking of the SpMV algorithm using **OpenMP** on a **shared-memory system**.
The goal is to evaluate the effects of different parallelization strategies on computational time and cache behavior.

### Repository structure

- `source`: contains the `C++` source code
- `data`: contains templates, tables and results

--- 

## Implementation

### Program structure

The SpMV algorithm is implemented across three main source files:

- `main.cpp`
- `csr.h`
- `csr.cpp` 


`main.cpp` manages the overall program workflow. It reads a symmetric sparse matrix from a text file in Matrix Market (`.mtx`) format, converts it into a Compressed Sparse Row (CSR) representation, and performs the matrix–vector multiplication using a randomly generated dense vector.
It reports both CPU time and real time measured with the `clock()` and `clock_gettime()` functions for performance evaluation.

`csr.h` and `csr.cpp` implement the CompressedSparseRow class, which represents the core of the program.
The class defines the CSR data structure and provides the implementation of the SpMV operation, including both the sequential and OpenMP-parallelized variants.

### Parallelization

OpenMP has been used to parallelize the `multiply_to_vector()` function implemented in the `csr.cpp` file.

Two level of parallelization have been adopted:

1. `#pragma omp parallel for`: parallelize the outer `for` loop with three different scheduling strategies: `static`, `dynamic` and `guided`

2. `#pragma omp simd`: applies SIMD vectorization to the inner `for` loop to further exploit data-level parallelism.

---

## Methodology

### Matrices

The program has been tested with the following symmetric matrices from [SuiteSparse Matrix Collection](https://sparse.tamu.edu):

| Author | Matrix | Alias | Dimension | Non-zeros |
|--------|--------|-------|-----------|-----------|
|MAWI|[mawi_201512012345](https://sparse.tamu.edu/MAWI/mawi_201512012345)|little.mtx|18.571.154|38.040.320|
|MAWI|[mawi_201512020000](https://sparse.tamu.edu/MAWI/mawi_201512020000)|small.mtx|35.991.342|74.485.420|
|MAWI|[mawi_201512020030](https://sparse.tamu.edu/MAWI/mawi_201512020030)|medium.mtx|68.863.315|143.414.960|
|MAWI|[mawi_201512020130](https://sparse.tamu.edu/MAWI/mawi_201512020130)|big.mtx|128.568.730|270.234.840|
|MAWI|[mawi_201512020330](https://sparse.tamu.edu/MAWI/mawi_201512020330)|huge.mtx|226.196.185|480.047.894|

All matrices represent undirected weighted graphs.  
Their sizes increase *exponentially* to effectively evaluate performance as the problem scales.


### Benchmarking procedure

Each matrix was executed **10 times** with the sequential version of the program to establish a baseline.  

For the OpenMP parallel versions, the program was executed **10 times** for every combination of scheduling policy, SIMD usage, and number of threads.   

Specifically, the tests covered:

- **Scheduling policies**: `static`, `dynamic`, `guided`  
- **SIMD options**: with and without `#pragma omp simd`  
- **Number of threads**: 4, 8, 16, 32, 64  

The `pbs_template.pbs` used for the HPC cluster, located in the `data/` folder, automates these runs. It contains a Bash script that executes the program 10 times per matrix for the specified configuration.

`perf stat` has been used to collect hardware performance counters with minimal overhead (estimated <1%).

*Since the HPC cluster is a shared resource, all tests were launched simultaneously to ensure consistent and comparable results across different configurations.*

---

## Usage

### Setup

Clone or download the repository and navigate to the folder corresponding to the version you want to run:

- `sequential/`: contains the sequential implementation  
- `parallel_no_simd/`: contains the OpenMP parallel version without SIMD  
- `parallel_simd/`: contains the OpenMP parallel version with SIMD  

Each folder includes the following source files: `main.cpp`, `csr.h`, `csr.cpp`.

### Compile

Compile the sequential code:

```bash
cd sequential/
g++ -std=c++11 -o sequential.elf -O3 -march=native *.cpp
```

Compile the parallel code:

```bash
cd parallel_no_simd/
g++ -std=c++11 -fopenmp -o parallel.elf -O3 -march=native *.cpp
```

*For the `parallel_simd/` code repeat the same operations*

### Execute

Run the sequential program:

```bash
./sequential.elf /path/to/matrix
```

Run the parallel program:

```bash
# SET THE PROPER ENVIRONMENT VARIABLES
export OMP_NUM_THREADS=8        # Threads number
export OMP_SCHEDULE="static"    # Scheduling strategy

./parallel.elf /path/to/matrix
```

### Output

The program prints performance data directly to `stdout` in the format:

`<matrix_name>:<cpu_time>:<real_time>`

This simple format makes it easy to parse and collect results for further analysis.

A verbose flag `-v` can be used for a more detailed output.

```bash
./sequential /path/to/matrix -v
[INFO]: Starting simulation...
[INFO]: Debug mode enabled
[INFO]: Importing file </path/to/matrix>
[INFO]: Compressing matrix...
[INFO]: Generating vector of size 1138
[INFO]: Starting computation...
[INFO]: Computation completed
[INFO]: Simulation metrics:
        CPU  TIME: 0.034 ms
        REAL TIME: 0.032307ms
[INFO]: Simulation completed
```

---

### Disclaimers

- The performance measurements obtained on the HPC cluster were highly variable.  
  Identical runs, even separated by short time intervals, could produce drastically different results.  
  Therefore, the dataset being used and discussed in this project refers specifically to the timing obtained **under the resources and allocation assigned during execution**.

- The program is designed to work only with **symmetric matrices** in **Matrix Market text format**.  
  Only matrices in the **coordinate format with three entries per line** (row, column, value) are supported. Matrices in other formats, such as those with only two values per line or in binary format, are **not compatible** with this implementation.
