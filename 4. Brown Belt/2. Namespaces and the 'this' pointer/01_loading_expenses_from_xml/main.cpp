#include "xml.h"
#include <iostream>

using namespace std;

struct Spending {
    string category;
    int amount;
};

vector<Spending> LoadFromXml(istream &input) {
    const auto document = Load(input);
    const auto node = document.GetRoot();
    const auto children = node.Children();

    vector<Spending> response;
    for (auto const &child: children) {
        response.push_back({child.AttributeValue<string>("category"), child.AttributeValue<int>("amount")});
    }

    return response;
}
