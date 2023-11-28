#include <string>
#include <vector>

using namespace std;

#define PREFIX(x) prefix_##x
#define LINE(x) PREFIX(x)
#define UNIQ_ID LINE(__LINE__)

int main() {
    int UNIQ_ID = 0;
    string UNIQ_ID = "hello";
    vector<string> UNIQ_ID = {"hello", "world"};
    vector<int> UNIQ_ID = {1, 2, 3, 4};
}
