#include "test_runner.h"
#include <algorithm>
#include <memory>
#include <vector>

using namespace std;

template<typename RandomIt>
void MergeSort(RandomIt range_begin, RandomIt range_end) {
    int size = range_end - range_begin;
    if (size < 2) {
        return;
    }

    vector<typename RandomIt::value_type> elements(
            make_move_iterator(range_begin),
            make_move_iterator(range_end)
    );

    int len_1_3 = size / 3;
    MergeSort(elements.begin(), elements.begin() + len_1_3);
    MergeSort(elements.begin() + len_1_3, elements.begin() + 2 * len_1_3);
    MergeSort(elements.begin() + 2 * len_1_3, elements.end());

    vector<typename RandomIt::value_type> result;
    merge(
            move_iterator(elements.begin()), move_iterator(elements.begin() + len_1_3),
            move_iterator(elements.begin() + len_1_3), move_iterator(elements.begin() + 2 * len_1_3),
            back_inserter(result)
    );
    merge(
            move_iterator(result.begin()), move_iterator(result.end()),
            move_iterator(elements.begin() + 2 * len_1_3), move_iterator(elements.end()),
            range_begin
    );
}

void TestIntVector() {
    vector<int> numbers = {6, 1, 3, 9, 1, 9, 8, 12, 1};
    MergeSort(begin(numbers), end(numbers));
    ASSERT(is_sorted(begin(numbers), end(numbers)));
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestIntVector);
    return 0;
}
