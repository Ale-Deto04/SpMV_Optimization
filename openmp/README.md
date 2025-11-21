# SpMV optimization with OpenMP

## Overview

This project focuses on the optimization and performance benchmarking of the SpMV algorithm using **OpenMP** on a **shared-memory system**.
The goal is to evaluate the effects of different parallelization strategies on computational time and cache behavior.

### Repository structure

```bash
openmp/
├── data/
│   ├── pbs/
│   ├── perf_output/
│   └── real_time_table.xlsx
├── images/
│   └── graphs/
├── README.md
└── source/
    ├── parallel_no_simd/
    ├── parallel_simd/
    └── sequential/
```

- `source`: contains the `C++` source code
- `data`: contains templates, tables and results
- `images`: contains media

--- 

## Implementation

### Program structure

The SpMV algorithm is implemented across three main source files:

- `main.cpp`
- `csr.h`
- `csr.cpp` 


`main.cpp` manages the overall program workflow. It reads a symmetric sparse matrix from a text file in Matrix Market (`.mtx`) format, converts it into a Compressed Sparse Row (CSR) representation, and performs the matrix–vector multiplication using a randomly generated dense vector.
It reports both CPU time and real time measured with the `clock()` and `clock_gettime()` functions for performance evaluation.

`csr.h` and `csr.cpp` implement the `CompressedSparseRow` class, which represents the core of the program.
The class defines the CSR data structure and provides the implementation of the SpMV operation, including both the sequential and OpenMP-parallelized variants.

### Parallelization

OpenMP has been used to parallelize the `multiply_to_vector()` method implemented in the `csr.cpp` file:

```cpp
vector<double> CompressedSparseRow::multiply_to_vector(const vector<double>& vec) const {

    vector<double> result(original_matrix_row_number);

    for (size_t i = 0; i < original_matrix_row_number; i++) {

        double partial_sum = 0;

        for (int j = csr_row[i]; j < csr_row[i + 1]; j++) {
            partial_sum += vec[csr_col[j]] * csr_val[j];
        }

        result[i] = partial_sum;
    }

    return result;
}
```

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

Matrices sizes increase **exponentially** to effectively evaluate performance as the problem scales.


### Benchmarking procedure

Each matrix was executed **10 times** with the sequential version of the program to establish a baseline.  

For the OpenMP parallel versions, the program was executed **10 times** for every combination of scheduling policy, SIMD usage, and number of threads.   

Specifically, the tests covered:

- **Scheduling policies**: `static`, `dynamic`, `guided`  
- **SIMD options**: with and without `#pragma omp simd`  
- **Number of threads**: 4, 8, 16, 32, 64  

The `sequential_pbs_template.pbs` and `parallel_pbs_template.pbs` used for the HPC cluster, located in the `data/pbs/` folder, automates these runs. They contain a Bash script that executes the program 10 times per matrix for the specified configuration.

`perf stat` has been used to collect hardware performance counters with minimal overhead (estimated <1%).

Since the HPC cluster is a shared resource, all tests were launched simultaneously to ensure consistent and comparable results across different configurations.

---

## Usage

### Minimum requirements
- **Compiler:** g++ 4.8 or higher  
- **C++ Standard:** C++11 or later

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

Compile the parallel code (valid for `parallel_simd/` as well):

```bash
cd parallel_no_simd/
g++ -std=c++11 -fopenmp -o parallel.elf -O3 -march=native *.cpp
```

### Execute

Run the sequential program:

```bash
./sequential.elf /path/to/matrix
```

Run the parallel program:

```bash
# SET THE PROPER ENVIRONMENT VARIABLES e.g.
export OMP_NUM_THREADS=8        # Threads number
export OMP_SCHEDULE="static"    # Scheduling strategy

./parallel.elf /path/to/matrix
```

### Execute on UniTn HPC cluster

#### Sequential execution

1. Download and edit the `data/sequential_pbs_template.pbs`. Set the proper `#PBS` parameters according to your resources requirements, specifically:
- `# Job name` (`line 4`): job name on the cluster
- `# Output and error files path` (`lines 7 - 8`): path to the `stdout` and `stderr` output file
- `# Queue name` (`line 11`): `short_cpuQ` by default
- `# Maximum wall time` (`line 14`): in the format `HH:MM:SS`
- `# Number of nodes, cpus and memory` (`line 17`): one cpu by default

2. Finally edit the absolute paths to matrices (`line 62`) and **move or copy the pbs to the source code folder**. 

3. Go to the source code folder and execute:

```bash
qsub sequential_pbs_template.pbs
```

The script automatically compiles and runs the program on the HPC cluster 10 times per matrix. Results will appear in the current directory once the execution is completed.

**Notice**: `perf` output will appear on the `stderr`files.

#### Parallel execution

Use the `parallel_pbs_template.pbs` for sequential code and follow the steps above.

Edit environment variables `THREADS` (`line 26`) and `SCHEDULING` (`line 27`) before step 2 in order to explicitly define the omp threads number and the scheduling policy used by the program.

**Notice**: ensure that `THREADS` does not exceed the number of CPUs requested (`ncpus`) in the `# Number of nodes, cpus and memory` query.

### Output

The program prints performance data directly to `stdout` in the format:

`<matrix_name>:<cpu_time>:<real_time>`

This simple format makes it easy to parse and collect results for further analysis.

A verbose flag `-v` can be used for a more detailed output e.g.

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
