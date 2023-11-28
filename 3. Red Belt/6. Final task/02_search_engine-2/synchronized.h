#pragma once

#include <mutex>

using namespace std;

template<typename T>
class Synchronized {
public:
    explicit Synchronized(T initial = T()) : value(std::move(initial)) {}

    struct Access {
        lock_guard<mutex> lg;
        T &ref_to_value;
    };

    Access GetAccess() {
        return {lock_guard<mutex>(mtx), value};
    }

private:
    mutex mtx;
    T value;
};