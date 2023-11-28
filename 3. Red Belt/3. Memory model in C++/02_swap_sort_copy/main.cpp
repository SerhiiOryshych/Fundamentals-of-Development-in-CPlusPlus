#include "test_runner.h"

#include <algorithm>
#include <numeric>

using namespace std;

template<typename T>
void Swap(T *first, T *second) {
    const T second_value_copy = *second;
    *second = *first;
    *first = second_value_copy;
}

template<typename T>
void SortPointers(vector<T *> &pointers) {
    sort(pointers.begin(), pointers.end(), [](const T *first, const T *second) { return *first < *second; });
}

template<typename T>
void ReversedCopy(T *source, size_t count, T *destination) {
    auto source_begin = source;
    auto source_end = source + count;

    auto destination_begin = destination;
    auto destination_end = destination + count;

    auto common_begin = max(source_begin, destination_begin);
    auto common_end = min(source_end, destination_end);

    for (auto i = source; i < source + count; i++) {
        if (common_begin <= i && i < common_end) continue;

        destination[count - (i - source) - 1] = *i;
    }

    if (common_begin < common_end) {
        int len = common_end - common_begin;
        for (T *i = common_begin; i < common_begin + len / 2; i++) {
            Swap(i, destination + count - (i - source) - 1);
        }
    }
}

void TestSwap() {
    int a = 1;
    int b = 2;
    Swap(&a, &b);
    ASSERT_EQUAL(a, 2);
    ASSERT_EQUAL(b, 1);

    string h = "world";
    string w = "hello";
    Swap(&h, &w);
    ASSERT_EQUAL(h, "hello");
    ASSERT_EQUAL(w, "world");
}

void TestSortPointers() {
    int one = 1;
    int two = 2;
    int three = 3;

    vector<int *> pointers;
    pointers.push_back(&two);
    pointers.push_back(&three);
    pointers.push_back(&one);

    SortPointers(pointers);

    ASSERT_EQUAL(pointers.size(), 3u);
    ASSERT_EQUAL(*pointers[0], 1);
    ASSERT_EQUAL(*pointers[1], 2);
    ASSERT_EQUAL(*pointers[2], 3);
}

void TestReverseCopy() {
    const size_t count = 7;

    int *source = new int[count];
    int *dest = new int[count];

    for (size_t i = 0; i < count; ++i) {
        source[i] = i + 1;
    }
    ReversedCopy(source, count, dest);
    const vector<int> expected1 = {7, 6, 5, 4, 3, 2, 1};
    ASSERT_EQUAL(vector<int>(dest, dest + count), expected1);

    // Memory areas may overlap.
    ReversedCopy(source, count - 1, source + 1);
    const vector<int> expected2 = {1, 6, 5, 4, 3, 2, 1};
    ASSERT_EQUAL(vector<int>(source, source + count), expected2);

    delete[] dest;
    delete[] source;
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestSwap);
    RUN_TEST(tr, TestSortPointers);
    RUN_TEST(tr, TestReverseCopy);
    return 0;
}
