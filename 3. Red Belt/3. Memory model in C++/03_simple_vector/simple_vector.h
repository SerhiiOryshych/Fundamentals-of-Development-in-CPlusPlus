#pragma once

#include <cstdlib>
#include <algorithm>

using namespace std;

template<typename T>
class SimpleVector {
public:
    SimpleVector() : data_(nullptr), end_(nullptr), capacity_(0) {}

    explicit SimpleVector(size_t size) {
        data_ = new T[size];
        end_ = data_ + size;
        capacity_ = size;
    }

    ~SimpleVector() {
        delete[] data_;
    }

    T &operator[](size_t index) {
        return data_[index];
    }

    T *begin() {
        return data_;
    }

    T *end() {
        return end_;
    }

    [[nodiscard]] size_t Size() const {
        if (data_ == nullptr) {
            return 0;
        }
        return end_ - data_;
    }

    [[nodiscard]] size_t Capacity() const {
        return capacity_;
    }

    void PushBack(const T &value) {
        if (Size() == Capacity()) {
            capacity_ = max(capacity_ * 2, (size_t) (1));
            T *new_data_ = new T[capacity_];
            if (data_ != nullptr) {
                for (auto x = data_; x < end_; x++) {
                    new_data_[x - data_] = *x;
                }
                end_ = new_data_ + (end_ - data_);
            } else {
                end_ = new_data_;
            }
            delete[] data_;
            data_ = new_data_;
        }

        *end_ = value;
        end_++;
    }

private:
    T *data_;
    T *end_;
    size_t capacity_;
};
