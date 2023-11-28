#include <iostream>
#include <vector>

using namespace std;

template<typename RandomIt>
void MergeSort(RandomIt range_begin, RandomIt range_end) {
    if (range_begin + 1 == range_end) {
        return;
    }

    vector<typename RandomIt::value_type> elements(range_begin, range_end);

    auto elements_medium = elements.begin() + (elements.end() - elements.begin()) / 2;
    MergeSort(elements.begin(), elements_medium);
    MergeSort(elements_medium, elements.end());
    merge(elements.begin(), elements_medium, elements_medium, elements.end(), range_begin);
}

int main() {
    vector<int> v = {6, 4, 7, 6, 4, 4, 0, 1};
    MergeSort(begin(v), end(v));
    for (int x: v) {
        cout << x << " ";
    }
    cout << endl;
    return 0;
}
