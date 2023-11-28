#include <iostream>
#include <list>
#include <map>

using namespace std;

class Manager {
public:
    void Add(int x, int y) {
        if (position.count(y) == 0) {
            queue.push_back(x);
            position.insert({x, prev(queue.end())});
        } else {
            position.insert({x, queue.insert(position.at(y), x)});
        }
    }

    void Log() {
        for (auto x: queue) {
            cout << x << " ";
        }
        cout << endl;
    }

private:
    list<int> queue;
    map<int, typename list<int>::iterator> position;
};

int main() {
    int n;
    cin >> n;

    Manager manager;

    while (n--) {
        int x, y;
        cin >> x >> y;
        manager.Add(x, y);
    }

    manager.Log();
}
