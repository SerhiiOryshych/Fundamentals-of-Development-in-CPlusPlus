#include <algorithm>
#include <cstddef>
#include <memory>
#include <new>
#include <utility>

template<typename T>
struct RawMemory {
    T *buf = nullptr;
    size_t cp = 0;

    RawMemory() = default;

    explicit RawMemory(size_t n) {
        buf = Allocate(n);
        cp = n;
    }

    RawMemory(const RawMemory &) = delete;

    RawMemory(RawMemory &&other) noexcept { Swap(other); }

    static T *Allocate(size_t n) {
        return static_cast<T *>(operator new(n * sizeof(T)));
    }

    static void Deallocate(T *buf) { operator delete(buf); }

    ~RawMemory() { Deallocate(buf); }

    T *operator+(size_t i) { return buf + i; }

    const T *operator+(size_t i) const { return buf + i; }

    T &operator[](size_t i) { return buf[i]; }

    const T &operator[](size_t i) const { return buf[i]; }

    RawMemory &operator=(const RawMemory &) = delete;

    RawMemory &operator=(RawMemory &&other) noexcept {
        Swap(other);
        return *this;
    }

    void Swap(RawMemory &other) {
        std::swap(buf, other.buf);
        std::swap(cp, other.cp);
    }

    const T *Begin() const { return buf; }

    T *Begin() { return buf; }
};

template<typename T>
class Vector {
private:
    RawMemory<T> data;
    size_t sz = 0;

    static void Construct(void *buf) { new(buf) T(); }

    static void Construct(void *buf, const T &elem) { new(buf) T(elem); }

    static void Construct(void *buf, T &&elem) { new(buf) T(std::move(elem)); }

    static void Destroy(T *buf) { buf->~T(); }

public:
    Vector() = default;

    explicit Vector(size_t n) : data(n) {
        std::uninitialized_value_construct_n(data.buf, n);
        sz = n;
    }

    Vector(const Vector &other) : data(other.sz) {
        std::uninitialized_copy_n(other.data.buf, other.sz, data.buf);
        sz = other.sz;
    }

    void Swap(Vector &other) {
        data.Swap(other.data);
        std::swap(sz, other.sz);
    }

    Vector(Vector &&other) noexcept { Swap(other); }

    ~Vector() { std::destroy_n(data.buf, sz); }

    Vector &operator=(const Vector &other) {
        if (other.sz > data.cp) {
            Vector tmp(other);
            Swap(tmp);
        } else {
            for (size_t i = 0; i < sz && i < other.sz; ++i) {
                data[i] = other[i];
            }
            if (sz < other.sz) {
                std::uninitialized_copy_n(other.data.buf + sz, other.sz - sz,
                                          data.buf + sz);
            } else if (sz > other.sz) {
                std::destroy_n(data.buf + other.sz, sz - other.sz);
            }
            sz = other.sz;
        }
        return *this;
    }

    Vector &operator=(Vector &&other) noexcept {
        Swap(other);
        return *this;
    }

    void Reserve(size_t n) {
        if (n > data.cp) {
            RawMemory<T> data2(n);
            std::uninitialized_move_n(data.buf, sz, data2.buf);
            std::destroy_n(data.buf, sz);
            data.Swap(data2);
        }
    }

    void Resize(size_t n) {
        Reserve(n);
        if (sz < n) {
            std::uninitialized_value_construct_n(data + sz, n - sz);
        } else if (sz > n) {
            std::destroy_n(data + n, sz - n);
        }
        sz = n;
    }

    void PushBack(const T &elem) {
        if (sz == data.cp) {
            Reserve(sz == 0 ? 1 : sz * 2);
        }
        new(data + sz) T(elem);
        ++sz;
    }

    void PushBack(T &&elem) {
        if (sz == data.cp) {
            Reserve(sz == 0 ? 1 : sz * 2);
        }
        new(data + sz) T(std::move(elem));
        ++sz;
    }

    template<typename... Args>
    T &EmplaceBack(Args &&...args) {
        if (sz == data.cp) {
            Reserve(sz == 0 ? 1 : sz * 2);
        }
        auto elem = new(data + sz) T(std::forward<Args>(args)...);
        ++sz;
        return *elem;
    }

    void PopBack() {
        std::destroy_at(data + sz - 1);
        --sz;
    }

    [[nodiscard]] size_t Size() const noexcept { return sz; }

    [[nodiscard]] size_t Capacity() const noexcept { return data.cp; }

    const T &operator[](size_t i) const { return data[i]; }

    T &operator[](size_t i) { return data[i]; }

    using iterator = T *;
    using const_iterator = const T *;

    iterator begin() noexcept { return data.Begin(); }

    iterator end() noexcept { return data + sz; }

    const_iterator begin() const noexcept { return data.Begin(); }

    const_iterator end() const noexcept { return data + sz; }

    // The implementation here should be the same as for the constant versions of begin/end.
    const_iterator cbegin() const noexcept { return data.Begin(); }

    const_iterator cend() const noexcept { return data + sz; }

    // Inserts an element before pos.
    // Returns an iterator to the inserted element.
    iterator Insert(const_iterator pos, const T &elem) {
        const size_t offset = pos - begin();
        RawMemory<T> data2(sz == Capacity() ? (sz == 0 ? 1 : sz * 2) : Capacity());
        std::uninitialized_move_n(begin(), offset, data2.Begin());
        new(data2 + offset) T(elem);
        if (sz > offset) {
            std::uninitialized_move_n(begin() + offset, sz - offset,
                                      data2.Begin() + offset + 1);
        }
        data.Swap(data2);
        ++sz;
        return begin() + offset;
    }

    iterator Insert(const_iterator pos, T &&elem) {
        const size_t offset = pos - begin();
        RawMemory<T> data2(sz == Capacity() ? (sz == 0 ? 1 : sz * 2) : Capacity());
        std::uninitialized_move_n(begin(), offset, data2.Begin());
        new(data2 + offset) T(std::move(elem));
        if (sz > offset) {
            std::uninitialized_move_n(begin() + offset, sz - offset,
                                      data2.Begin() + offset + 1);
        }
        data.Swap(data2);
        ++sz;
        return begin() + offset;
    }

    // Constructs an element using the provided constructor arguments before pos.
    // Returns an iterator to the inserted element.
    template<typename... Args>
    iterator Emplace(const_iterator pos, Args &&...args) {
        return Insert(pos, T(std::forward<Args>(args)...));
    }

    // Removes the element at position pos.
    // Returns an iterator to the element following the removed one.
    iterator Erase(const_iterator pos) {
        const size_t offset = pos - begin();
        if (offset + 1 < sz) {
            std::move(data + offset + 1, data + sz, data + offset);
        }
        PopBack();
        return begin() + offset;
    }
};
