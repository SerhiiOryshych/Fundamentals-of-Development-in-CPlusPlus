#include "sum_reverse_sort.h"
#include <algorithm>

int Sum(int x, int y) {
    return x + y;
}

string Reverse(string s) {
    string q;
    for (int i = s.size(); i > 0; i--) {
        q += s[i - 1];
    }
    return q;
}

void Sort(vector<int> &nums) {
    sort(nums.begin(), nums.end());
}
