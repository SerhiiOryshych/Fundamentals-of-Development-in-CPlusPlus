#include <vector>
#include <iostream>

using namespace std;

template<typename T>
class Deque {
public:
    Deque() = default;

    [[nodiscard]] bool Empty() const {
        return (front.size() + back.size() == 0);
    }

    [[nodiscard]] ::size_t Size() const {
        return front.size() + back.size();
    }

    T &operator[](size_t index) {
        // checkOutOfRange(index);
        return GetValueAtIndex(index);
    }

    const T &operator[](size_t index) const {
        // checkOutOfRange(index);
        return GetValueAtIndex(index);
    }

    T &At(size_t index) {
        checkOutOfRange(index);
        return GetValueAtIndex(index);
    }

    const T &At(size_t index) const {
        checkOutOfRange(index);
        return GetValueAtIndex(index);
    }

    const T &Front() const {
        if (front.empty()) {
            return back[0];
        }
        return front[front.size() - 1];
    }

    T &Front() {
        if (front.empty()) {
            return back[0];
        }
        return front[front.size() - 1];
    }

    const T &Back() const {
        if (back.empty()) {
            return front[0];
        }
        return back[back.size() - 1];
    }

    T &Back() {
        if (back.empty()) {
            return front[0];
        }
        return back[back.size() - 1];
    }

    void PushFront(const T &x) {
        front.push_back(x);
    }

    void PushBack(const T &x) {
        back.push_back(x);
    }

private:
    T &GetValueAtIndex(size_t index) {
        if (index < front.size()) {
            return front[front.size() - index - 1];
        } else {
            return back[index - front.size()];
        }
    }

    void checkOutOfRange(size_t index) {
        if (index >= front.size() + back.size()) {
            throw out_of_range("");
        }
    }

    vector<T> front, back;
};
