#include "test_runner.h"
#include <utility>
#include <cstddef>

using namespace std;

template<typename T>
class UniquePtr {
private:
    T *raw_ptr = nullptr;
public:
    UniquePtr() = default;

    explicit UniquePtr(T *ptr) : raw_ptr(ptr) {}

    UniquePtr(const UniquePtr &) = delete;

    UniquePtr(UniquePtr &&other) noexcept {
        if (this != &other) {
            raw_ptr = other.raw_ptr;
            other.raw_ptr = nullptr;
        }
    }

    UniquePtr &operator=(const UniquePtr &) = delete;

    UniquePtr &operator=(nullptr_t) {
        Reset(nullptr);
        return *this;
    }

    UniquePtr &operator=(UniquePtr &&other) {
        if (this != &other) {
            Reset(other.raw_ptr);
            other.raw_ptr = nullptr;
        }
        return *this;
    }

    ~UniquePtr() {
        delete raw_ptr;
    }

    T &operator*() const {
        return *raw_ptr;
    }

    T *operator->() const {
        return raw_ptr;
    }

    T *Release() {
        auto ptr = raw_ptr;
        raw_ptr = nullptr;
        return ptr;
    }

    void Reset(T *ptr) {
        if (raw_ptr != ptr) {
            delete raw_ptr;
            raw_ptr = ptr;
        }
    }

    void Swap(UniquePtr &other) {
        if (raw_ptr != other.raw_ptr) {
            swap(raw_ptr, other.raw_ptr);
        }
    }

    [[nodiscard]] T *Get() const {
        return raw_ptr;
    }
};


struct Item {
    static int counter;
    int value;

    Item(int v = 0) : value(v) {
        ++counter;
    }

    Item(const Item &other) : value(other.value) {
        ++counter;
    }

    ~Item() {
        --counter;
    }
};

int Item::counter = 0;


void TestLifetime() {
    Item::counter = 0;
    {
        UniquePtr<Item> ptr(new Item);
        ASSERT_EQUAL(Item::counter, 1);

        ptr.Reset(new Item);
        ASSERT_EQUAL(Item::counter, 1);
    }
    ASSERT_EQUAL(Item::counter, 0);

    {
        UniquePtr<Item> ptr(new Item);
        ASSERT_EQUAL(Item::counter, 1);

        auto rawPtr = ptr.Release();
        ASSERT_EQUAL(Item::counter, 1);

        delete rawPtr;
        ASSERT_EQUAL(Item::counter, 0);
    }
    ASSERT_EQUAL(Item::counter, 0);
}

void TestGetters() {
    UniquePtr<Item> ptr(new Item(42));
    ASSERT_EQUAL(ptr.Get()->value, 42);
    ASSERT_EQUAL((*ptr).value, 42);
    ASSERT_EQUAL(ptr->value, 42);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestLifetime);
    RUN_TEST(tr, TestGetters);
}
