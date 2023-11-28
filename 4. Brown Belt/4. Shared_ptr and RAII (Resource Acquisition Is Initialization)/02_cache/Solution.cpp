#include "Common.h"

#include <mutex>
#include <utility>
#include <unordered_map>

using namespace std;

class LruCache : public ICache {
public:
    LruCache(
            shared_ptr<IBooksUnpacker> books_unpacker,
            const Settings &settings
    ) : books_unpacker(books_unpacker), settings(settings) {}

    BookPtr GetBook(const string &book_name) override {
        return GetCacheValue(book_name);
    }

private:
    struct CacheValue {
        size_t rang = 0, size = 0;
        shared_ptr<IBook> book;
    };

    mutex m;
    unordered_map<string, CacheValue> cache;
    size_t memory_used = 0, max_cache_rang = 0;

    shared_ptr<IBooksUnpacker>
            books_unpacker;
    const Settings settings;

    BookPtr GetCacheValue(const string &book_name) {
        lock_guard<mutex> lg(m);

        auto it = cache.find(book_name);
        if (it != cache.end()) {
            it->second.rang = ++max_cache_rang;
            return it->second.book;
        }

        return AddBookToCache(book_name);
    }

    BookPtr AddBookToCache(const string &book_name) {
        auto book = books_unpacker->UnpackBook(book_name);
        CacheValue cacheValue = {++max_cache_rang, book->GetContent().size(), shared_ptr<IBook>(move(book))};

        while (memory_used + cacheValue.size > settings.max_memory && !cache.empty()) {
            auto min_rang_it = cache.begin();
            for (auto it = cache.begin(); it != cache.end(); it++) {
                if (min_rang_it->second.rang > it->second.rang) {
                    min_rang_it = it;
                }
            }
            memory_used -= min_rang_it->second.size;
            cache.erase(min_rang_it);
        }

        if (memory_used + cacheValue.size <= settings.max_memory) {
            cache[book_name] = cacheValue;
            memory_used += cacheValue.size;
        }

        return move(cacheValue.book);
    }
};


unique_ptr<ICache> MakeCache(
        shared_ptr<IBooksUnpacker> books_unpacker,
        const ICache::Settings &settings
) {
    return make_unique<LruCache>(move(books_unpacker), settings);
}
