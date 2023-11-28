#pragma once

#include "synchronized.h"
#include <istream>
#include <ostream>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <future>
#include <array>

using namespace std;

class InvertedIndex {
public:
    void Add(const string &document);

    [[nodiscard]] vector<pair<size_t, size_t>> *Lookup(const string &word);

    [[nodiscard]] size_t GetDocSize() const {
        return doc_size;
    }

private:
    size_t doc_size = 0;
    map<string, vector<pair<size_t, size_t>>> index;
};

class SearchServer {
public:
    SearchServer() = default;

    ~SearchServer();

    explicit SearchServer(istream &document_input);

    void UpdateDocumentBase(istream &document_input);

    void AddQueriesStream(istream &query_input, ostream &search_results_output);

private:
    mutex index_swap_mutex;
    vector<future<void>> async_tasks;
    Synchronized<InvertedIndex> index;
};
