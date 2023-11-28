#include <cstdint>
#include <cstdlib>
#include <utility>
#include <algorithm>

using namespace std;

template<typename T>
class SimpleVector {
public:
    SimpleVector() : data_(nullptr), size_(0), capacity_(0) {}

    explicit SimpleVector(size_t size) : size_(size), capacity_(size) {
        data_ = new T[size_];
    }

    ~SimpleVector() {
        if (data_ == nullptr) {
            return;
        }
        delete[] data_;
    }

    T &operator[](size_t index) {
        return data_[index];
    }

    T *begin() {
        return data_;
    }

    T *begin() const {
        return data_;
    }

    T *end() {
        return data_ + size_;
    }

    T *end() const {
        return data_ + size_;
    }

    [[nodiscard]] size_t Size() const {
        return size_;
    }

    [[nodiscard]] size_t Capacity() const {
        return capacity_;
    }

    void PushBack(T value) {
        if (Size() == Capacity()) {
            capacity_ = max(capacity_ * 2, (size_t) (1));
            T *new_data_ = new T[capacity_];
            if (data_ != nullptr) {
                move(begin(), end(), new_data_);
                delete[] data_;
            }
            data_ = move(new_data_);
        }

        *(data_ + size_++) = move(value);
    }

    SimpleVector(SimpleVector &&other) noexcept {
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.data_ = nullptr;
    }

    SimpleVector(const SimpleVector &other) {
        data_ = new T[other.size_];
        copy(other.begin(), other.end(), data_);
        size_ = other.size_;
        capacity_ = other.capacity_;
    }

    SimpleVector &operator=(SimpleVector &&other) noexcept {
        delete[] data_;
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.data_ = nullptr;
        return *this;
    }

    SimpleVector &operator=(const SimpleVector &other) {
        delete[] data_;
        data_ = new T[other.Capacity()];
        size_ = other.Size();
        capacity_ = other.Capacity();
        copy(other.begin(), other.end(), begin());
        return *this;
    }

private:
    T *data_;
    size_t size_, capacity_;
};
