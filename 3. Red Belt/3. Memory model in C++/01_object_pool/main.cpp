#include "test_runner.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <queue>
#include <stdexcept>
#include <set>

using namespace std;

template<class T>
class ObjectPool {
public:
    T *Allocate() {
        T *p;
        if (!deallocated_.empty()) {
            p = deallocated_.front();
            deallocated_.pop();
        } else {
            p = new T();
        }
        allocated_.insert(p);
        return p;
    }

    T *TryAllocate() {
        if (deallocated_.empty()) {
            return nullptr;
        }
        T *p = deallocated_.front();
        allocated_.insert(p);
        deallocated_.pop();
        return p;
    }

    void Deallocate(T *object) {
        if (allocated_.count(object) == 0) {
            throw invalid_argument("");
        }

        deallocated_.push(object);
        allocated_.erase(object);
    }

    ~ObjectPool() {
        while (!allocated_.empty()) {
            auto it = *allocated_.begin();
            allocated_.erase(it);
            delete it;
        }

        while (!deallocated_.empty()) {
            auto obj = deallocated_.front();
            deallocated_.pop();
            delete obj;
        }
    }

private:
    set<T *> allocated_;
    queue<T *> deallocated_;
};

void TestObjectPool() {
    ObjectPool<string> pool;

    auto p1 = pool.Allocate();
    auto p2 = pool.Allocate();
    auto p3 = pool.Allocate();

    *p1 = "first";
    *p2 = "second";
    *p3 = "third";

    pool.Deallocate(p2);
    ASSERT_EQUAL(*pool.Allocate(), "second");

    pool.Deallocate(p3);
    pool.Deallocate(p1);
    ASSERT_EQUAL(*pool.Allocate(), "third");
    ASSERT_EQUAL(*pool.Allocate(), "first");

    pool.Deallocate(p1);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestObjectPool);
    return 0;
}
