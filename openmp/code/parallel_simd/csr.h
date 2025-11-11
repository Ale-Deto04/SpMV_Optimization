#ifndef __csr_h__
#define __csr_h__

#include <vector>
#include <iostream>
#include <string>

using namespace std;

class CompressedSparseRow {

private:

    size_t original_matrix_row_number, original_matrix_col_number;

    vector<int> csr_row;      //Contains the starting index for each row
    vector<int> csr_col;      //Contains the index of each non-null value for each row
    vector<double> csr_val;   //Contains the actual non-null values


public:

    // Constructor
    CompressedSparseRow(const string& file_name);

    // Methods
    vector<double> multiply_to_vector(const vector<double>& vec) const;

    // Getter
    size_t get_original_matrix_row_num() const;
    size_t get_original_matrix_col_num() const;

    // Operator override
    // friend ostream& operator<<(ostream& os, const CompressedSparseRow& csr);
};

#endif
