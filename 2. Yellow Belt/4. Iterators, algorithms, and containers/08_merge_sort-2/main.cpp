#include <iostream>
#include <vector>

using namespace std;

template<typename RandomIt>
void MergeSort(RandomIt range_begin, RandomIt range_end) {
    int range_size = range_end - range_begin;

    if (range_size < 2) {
        return;
    }

    vector<typename RandomIt::value_type> elements(range_begin, range_end);

    MergeSort(elements.begin(), elements.begin() + range_size / 3);
    MergeSort(elements.begin() + range_size / 3, elements.begin() + 2 * range_size / 3);
    MergeSort(elements.begin() + 2 * range_size / 3, elements.end());

    vector<typename RandomIt::value_type> result;
    merge(elements.begin(), elements.begin() + range_size / 3, elements.begin() + range_size / 3,
          elements.begin() + 2 * range_size / 3, back_inserter(result));
    merge(result.begin(), result.end(), elements.begin() + 2 * range_size / 3,
          elements.end(), range_begin);
}

int main() {
    vector<int> v = {6, 4, 7, 6, 4, 4, 0, 1, 5};
    MergeSort(begin(v), end(v));
    for (int x: v) {
        cout << x << " ";
    }
    cout << endl;
    return 0;
}
