#include "csr.h"
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

CompressedSparseRow::CompressedSparseRow(const string& file_name) {   

    bool getting_info = true;
    unsigned int row_num, col_num, val_num, tmp_r, tmp_c;
    double tmp_v;
    string tuple;

    // Importing matrix from file

    ifstream file(file_name);
    if (!file.is_open()) {
        cerr << "[ERR]: Could not open the file" << endl;
        exit(1);
    }

    vector<vector<pair<int, double>>> temp_rows; // vector[row] = vector<col, val>

    while (getline(file, tuple)) {

        if (tuple.empty() || tuple[0] == '%') {
            continue;
        }

        //checks whether it's information line or not
        if (getting_info) {
            if (sscanf(tuple.c_str(), "%u %u %u", &row_num, &col_num, &val_num ) != 3) {
                cerr << "[ERR]: Format error in fetching matrix info" << endl;
                file.close();
                exit(1);
            }
            
            this->original_matrix_row_number = row_num;
            this->original_matrix_col_number = col_num;

            temp_rows.resize(row_num);
    
            getting_info = false;

        } else {
            if (sscanf(tuple.c_str(), "%u %u %lf", &tmp_r, &tmp_c, &tmp_v) != 3) {
                cerr << "[ERR]: Format error in fetching matrix values" << endl;
                file.close();
                exit(1);
            }
            
            temp_rows[tmp_r - 1].emplace_back(tmp_c - 1, tmp_v);


            // Symmetric fill
            if (tmp_r != tmp_c) {
                temp_rows[tmp_c - 1].emplace_back(tmp_r - 1, tmp_v);
            }
        }
    }

    file.close();

    // CSR conversion

    size_t full_matrix_total_values = 0;
    for (size_t i = 0; i < original_matrix_row_number; i++) {
        full_matrix_total_values += temp_rows[i].size();
    }

    csr_row.resize(original_matrix_row_number + 1);
    csr_col.resize(full_matrix_total_values);
    csr_val.resize(full_matrix_total_values);

    int index = 0;
    csr_row[0] = 0;

    // Scan vector
    for (size_t i = 0; i < original_matrix_row_number; i++) {

        sort(temp_rows[i].begin(), temp_rows[i].end());

        for (const auto& pair : temp_rows[i]) {
            csr_col[index] = pair.first;
            csr_val[index] = pair.second;
            index++;
        }
        
        // set new interval
        csr_row[i + 1] = index;
    }
}

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

size_t CompressedSparseRow::get_original_matrix_col_num() const {
    return original_matrix_col_number;
}

size_t CompressedSparseRow::get_original_matrix_row_num() const {
    return original_matrix_row_number;
}


/* FOR DEBUG ONLY*/
// ostream& operator<<(ostream& os, const CompressedSparseRow& csr) {
//
//     os << "Original Row Number: " << csr.original_matrix_row_number << endl;
//     os << "Original Col Number: " << csr.original_matrix_col_number << endl;
//    
//     os << "CSR row: ";
//     for (const auto& integer : csr.csr_row) {
//         os << integer << " ";
//     }
//     os << endl;
//
//     os << "CSR col: ";
//     for (const auto& integer : csr.csr_col) {
//         os << integer << " ";
//     }
//     os << endl;
//
//     os << "CSR val: ";
//     for (const auto& val : csr.csr_val) {
//         os << val << " ";
//     }
//     os << endl;
//
//     return os;
// }