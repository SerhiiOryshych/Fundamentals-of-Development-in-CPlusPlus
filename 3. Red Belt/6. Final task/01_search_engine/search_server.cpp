#include "search_server.h"
#include "iterator_range.h"

#include <algorithm>
#include <iterator>
#include <sstream>

vector<string> SplitIntoWords(const string &line) {
    istringstream words_input(line);
    return {istream_iterator<string>(words_input), istream_iterator<string>()};
}

SearchServer::SearchServer(istream &document_input) {
    UpdateDocumentBase(document_input);
}

void SearchServer::UpdateDocumentBase(istream &document_input) {
    InvertedIndex new_index;

    for (string current_document; getline(document_input, current_document);) {
        new_index.Add(move(current_document));
    }

    index = move(new_index);
}

void SearchServer::AddQueriesStream(
        istream &query_input, ostream &search_results_output
) {
    for (string current_query; getline(query_input, current_query);) {
        const auto words = SplitIntoWords(current_query);

        size_t doc_size = index.GetDocSize();
        array<size_t, 50000> &doc_id_count = index.GetDocIdCount();
        fill(doc_id_count.begin(), doc_id_count.begin() + doc_size, 0);
        for (const auto &word: words) {
            auto lookup_res = index.Lookup(word);
            if (lookup_res == nullptr) {
                continue;
            }

            for (const auto &[doc_id, cnt]: *lookup_res) {
                doc_id_count[doc_id] += cnt;
            }
        }

        vector<pair<size_t, size_t>> search_results;
        for (int i = 0; i < doc_size; i++) {
            search_results.emplace_back(i, doc_id_count[i]);
        }

        partial_sort(
                begin(search_results),
                min(begin(search_results) + 5, end(search_results)),
                end(search_results),
                [](const pair<size_t, size_t> &lhs, const pair<size_t, size_t> &rhs) {
                    int64_t lhs_doc_id = lhs.first;
                    auto lhs_hit_count = lhs.second;
                    int64_t rhs_doc_id = rhs.first;
                    auto rhs_hit_count = rhs.second;
                    return make_pair(lhs_hit_count, -lhs_doc_id) > make_pair(rhs_hit_count, -rhs_doc_id);
                }
        );

        search_results_output << current_query << ':';
        for (auto [doc_id, hit_count]: Head(search_results, 5)) {
            if (hit_count == 0) {
                break;
            }
            search_results_output << " {"
                                  << "docid: " << doc_id << ", "
                                  << "hitcount: " << hit_count << '}';
        }
        search_results_output << endl;
    }
}

void InvertedIndex::Add(const string &document) {
    map<string, size_t> words_cnt;
    for (const auto &word: SplitIntoWords(document)) {
        words_cnt[word]++;
    }

    doc_size++;
    const size_t doc_id = doc_size - 1;
    for (const auto &p: words_cnt) {
        index[p.first].emplace_back(doc_id, p.second);
    }
}

vector<pair<size_t, size_t>> *InvertedIndex::Lookup(const string &word) {
    if (auto it = index.find(word); it != index.end()) {
        return &it->second;
    } else {
        return nullptr;
    }
}
