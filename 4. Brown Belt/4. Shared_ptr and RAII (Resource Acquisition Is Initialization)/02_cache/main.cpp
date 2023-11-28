#include "Common.h"
#include "test_runner.h"

#include <atomic>
#include <future>
#include <numeric>
#include <random>
#include <sstream>
#include <unordered_map>

using namespace std;

// This implementation of the IBook interface allows you to track the amount of memory
// currently occupied by all loaded books. For testing your program, you can write another
// implementation, which will also ensure that the least used items are unloaded from the cache first.
// In fact, the Coursera testing system has just such a more advanced implementation.
class Book : public IBook {
public:
    Book(
            string name,
            string content,
            atomic<size_t> &memory_used_by_books
    )
            : name_(move(name)), content_(move(content)), memory_used_by_books_(memory_used_by_books) {
        memory_used_by_books_ += content_.size();
    }

    ~Book() {
        memory_used_by_books_ -= content_.size();
    }

    const string &GetName() const override {
        return name_;
    }

    const string &GetContent() const override {
        return content_;
    }

private:
    string name_;
    string content_;
    atomic<size_t> &memory_used_by_books_;
};

// This implementation of the IBooksUnpacker interface allows you to track the amount of memory
// currently occupied by all loaded books and request the number of calls to the UnpackBook() method.
// For testing your program, you can write another implementation. In fact, the Coursera testing system
// has just such a more advanced implementation.
class BooksUnpacker : public IBooksUnpacker {
public:
    unique_ptr<IBook> UnpackBook(const string &book_name) override {
        ++unpacked_books_count_;
        return make_unique<Book>(
                book_name,
                "Dummy content of the book " + book_name,
                memory_used_by_books_
        );
    }

    size_t GetMemoryUsedByBooks() const {
        return memory_used_by_books_;
    }

    int GetUnpackedBooksCount() const {
        return unpacked_books_count_;
    }

private:
    // The template class atomic allows for the safe use of a scalar type from
    // multiple threads. Otherwise, we would have a race condition.
    atomic<size_t> memory_used_by_books_ = 0;
    atomic<int> unpacked_books_count_ = 0;
};

struct Library {
    vector<string> book_names;
    unordered_map<string, unique_ptr<IBook>> content;
    size_t size_in_bytes = 0;

    explicit Library(vector<string> a_book_names, IBooksUnpacker &unpacker)
            : book_names(std::move(a_book_names)) {
        for (const auto &book_name: book_names) {
            auto &book_content = content[book_name];
            book_content = unpacker.UnpackBook(book_name);
            size_in_bytes += book_content->GetContent().size();
        }
    }
};


void TestUnpacker(const Library &lib) {
    BooksUnpacker unpacker;
    for (const auto &book_name: lib.book_names) {
        auto book = unpacker.UnpackBook(book_name);
        ASSERT_EQUAL(book->GetName(), book_name);
    }
}


void TestMaxMemory(const Library &lib) {
    auto unpacker = make_shared<BooksUnpacker>();
    ICache::Settings settings;
    settings.max_memory = lib.size_in_bytes / 2;
    auto cache = MakeCache(unpacker, settings);

    for (const auto &[name, book]: lib.content) {
        cache->GetBook(name);
        ASSERT(unpacker->GetMemoryUsedByBooks() <= settings.max_memory);
    }
}


void TestCaching(const Library &lib) {
    auto unpacker = make_shared<BooksUnpacker>();
    ICache::Settings settings;
    settings.max_memory = lib.size_in_bytes;
    auto cache = MakeCache(unpacker, settings);

    // If the same book is requested consecutively, it should definitely
    // be returned from the cache. Note that this simple test is
    // not sufficient to fully check the correctness of the implementation of the
    // replacement strategy for elements in the cache. For these purposes, you can write a test
    // yourself.
    cache->GetBook(lib.book_names[0]);
    cache->GetBook(lib.book_names[0]);
    cache->GetBook(lib.book_names[0]);
    ASSERT_EQUAL(unpacker->GetUnpackedBooksCount(), 1);
}


void TestSmallCache(const Library &lib) {
    auto unpacker = make_shared<BooksUnpacker>();
    ICache::Settings settings;
    settings.max_memory =
            unpacker->UnpackBook(lib.book_names[0])->GetContent().size() - 1;
    auto cache = MakeCache(unpacker, settings);

    cache->GetBook(lib.book_names[0]);
    ASSERT_EQUAL(unpacker->GetMemoryUsedByBooks(), size_t(0));
}


void TestAsync(const Library &lib) {
    static const int tasks_count = 10;
    static const int trials_count = 10000;

    auto unpacker = make_shared<BooksUnpacker>();
    ICache::Settings settings;
    settings.max_memory = lib.size_in_bytes - 1;
    auto cache = MakeCache(unpacker, settings);

    vector<future<void>> tasks;

    for (int task_num = 0; task_num < tasks_count; ++task_num) {
        tasks.push_back(async([&cache, &lib, task_num] {
            default_random_engine gen;
            uniform_int_distribution<size_t> dis(0, lib.book_names.size() - 1);
            for (int i = 0; i < trials_count; ++i) {
                const auto &book_name = lib.book_names[dis(gen)];
                ASSERT_EQUAL(
                        cache->GetBook(book_name)->GetContent(),
                        lib.content.find(book_name)->second->GetContent()
                );
            }
            stringstream ss;
            ss << "Task #" << task_num << " completed\n";
            cout << ss.str();
        }));
    }

    // The get method throws exceptions to the main thread.
    for (auto &task: tasks) {
        task.get();
    }
}


int main() {
    BooksUnpacker unpacker;
    const Library lib(
            // Names of books for local testing. In the Coursera testing system,
            // there will be a different set, much larger.
            {
                    "Sherlock Holmes",
                    "Don Quixote",
                    "Harry Potter",
                    "A Tale of Two Cities",
                    "The Lord of the Rings",
                    "Le Petit Prince",
                    "Alice in Wonderland",
                    "Dream of the Red Chamber",
                    "And Then There Were None",
                    "The Hobbit"
            },
            unpacker
    );

#define RUN_CACHE_TEST(tr, f) tr.RunTest([&lib] { f(lib); }, #f)

    TestRunner tr;
    RUN_CACHE_TEST(tr, TestUnpacker);
    RUN_CACHE_TEST(tr, TestMaxMemory);
    RUN_CACHE_TEST(tr, TestCaching);
    RUN_CACHE_TEST(tr, TestSmallCache);
    RUN_CACHE_TEST(tr, TestAsync);

#undef RUN_CACHE_TEST
    return 0;
}
