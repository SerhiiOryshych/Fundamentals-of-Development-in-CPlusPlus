#include "test_runner.h"
#include <future>
#include <vector>

using namespace std;

int64_t CalculateMatrixSum(const vector<vector<int>> &matrix) {
    const size_t SIZE = matrix.size();

    auto get_part_sum = [&matrix, SIZE](size_t start_row, size_t end_row) {
        long long s = 0;
        for (size_t i = start_row; i < end_row; i++) {
            for (size_t j = 0; j < SIZE; j++) {
                s += matrix[i][j];
            }
        }
        return s;
    };

    size_t first_row = 0;
    size_t page_size = SIZE / 4 + 1;
    vector<future<long long>> part_sums;
    while (first_row < SIZE) {
        part_sums.push_back(async([first_row, page_size, SIZE, &get_part_sum] {
            return get_part_sum(first_row, min(first_row + page_size, SIZE));
        }));
        first_row += page_size;
    }

    long long global_sum = 0;
    for (auto &part_sum: part_sums) {
        global_sum += part_sum.get();
    }
    return global_sum;
}

void TestCalculateMatrixSum() {
    const vector<vector<int>> matrix = {
            {1,  2,  3,  4},
            {5,  6,  7,  8},
            {9,  10, 11, 12},
            {13, 14, 15, 16}
    };
    ASSERT_EQUAL(CalculateMatrixSum(matrix), 136);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestCalculateMatrixSum);
}
