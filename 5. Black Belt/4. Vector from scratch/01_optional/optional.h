#include <utility>

struct BadOptionalAccess {
};

template<typename T>
class Optional {
private:
    alignas(T) unsigned char data[sizeof(T)];
    bool defined = false;

    T *Get() {
        return reinterpret_cast<T *>(&data);
    }

    const T *Get() const {
        return reinterpret_cast<const T *>(&data);
    }

    void Assign(const Optional &source) {
        if (defined) {
            if (source.defined) {
                **this = *source;
            } else {
                Reset();
            }
        } else {
            if (source.defined) {
                new(data) T(*source.Get());
                defined = true;
            }
        }
    }

    void Assign(Optional &&source) {
        if (defined) {
            if (source.defined) {
                **this = std::move(*source);
            } else {
                Reset();
            }
        } else {
            if (source.defined) {
                new(data) T(std::move(*source.Get()));
            }
        }
        defined = source.defined;
    }

public:
    Optional() = default;

    Optional(const T &elem) {
        new(data) T(elem);
        defined = true;
    }

    Optional(T &&elem) {
        new(data) T(std::move(elem));
        defined = true;
    }

    Optional(const Optional &other) {
        Assign(other);
    }

    Optional(Optional &&other) {
        Assign(std::move(other));
    }

    Optional &operator=(const T &elem) {
        if (defined) {
            **this = elem;
        } else {
            new(data) T(elem);
        }
        defined = true;
        return *this;
    }

    Optional &operator=(T &&elem) {
        if (defined) {
            **this = std::move(elem);
        } else {
            new(data) T(std::move(elem));
        }
        defined = true;
        return *this;
    }

    Optional &operator=(const Optional &other) {
        Assign(other);
        return *this;
    }

    Optional &operator=(Optional &&other) noexcept {
        Assign(std::move(other));
        return *this;
    }

    [[nodiscard]] bool HasValue() const {
        return defined;
    }

    T &operator*() {
        return *Get();
    }

    const T &operator*() const {
        return *Get();
    }

    T *operator->() {
        return Get();
    }

    const T *operator->() const {
        return Get();
    }

    T &Value() {
        if (defined) {
            return *Get();
        } else {
            throw BadOptionalAccess{};
        }
    }

    const T &Value() const {
        if (defined) {
            return *Get();
        } else {
            throw BadOptionalAccess{};
        }
    }

    void Reset() {
        if (defined) {
            (*this)->~T();
        }
        defined = false;
    }

    ~Optional() {
        Reset();
    }
};
