#pragma once

#include <istream>
#include <ostream>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <array>

using namespace std;

class InvertedIndex {
public:
    void Add(const string &document);

    [[nodiscard]] vector<pair<size_t, size_t>> *Lookup(const string &word);

    array<size_t, 50000> &GetDocIdCount() {
        return doc_id_count;
    }

    [[nodiscard]] size_t GetDocSize() const {
        return doc_size;
    }

private:
    size_t doc_size = 0;
    array<size_t, 50000> doc_id_count;
    map<string, vector<pair<size_t, size_t>>> index;
};

class SearchServer {
public:
    SearchServer() = default;

    explicit SearchServer(istream &document_input);

    void UpdateDocumentBase(istream &document_input);

    void AddQueriesStream(istream &query_input, ostream &search_results_output);

private:
    InvertedIndex index;
};
