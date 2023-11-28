#pragma once

#include <array>
#include <stdexcept>

using namespace std;

template<typename T, size_t N>
class StackVector {
public:
    explicit StackVector(size_t a_size = 0) : size_(a_size), capacity_(N) {
        if (size_ > capacity_) {
            throw invalid_argument("");
        }
    }

    T &operator[](size_t index) {
        if (index >= size_) {
            throw invalid_argument("");
        }
        return data_[index];
    }

    const T &operator[](size_t index) const {
        if (index >= size_) {
            throw invalid_argument("");
        }
        return data_[index];
    }

    T *begin() {
        return data_.begin();
    }

    T *end() {
        return data_.begin() + size_;
    }

    const T *begin() const {
        return data_.begin();
    }

    const T *end() const {
        return data_.begin() + size_;
    }

    [[nodiscard]] size_t Size() const {
        return size_;
    }

    [[nodiscard]] size_t Capacity() const {
        return capacity_;
    }

    void PushBack(const T &value) {
        if (size_ == capacity_) {
            throw overflow_error("");
        }

        data_[size_++] = value;
    }

    T PopBack() {
        if (size_ == 0) {
            throw underflow_error("");
        }
        return data_[--size_];
    }

private:
    size_t size_, capacity_;
    array<T, N> data_;
};
