#include "animals.h"
#include "test_runner.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <memory>

using namespace std;

using Zoo = vector<unique_ptr<Animal>>;

// This function takes an input stream and reads a description of animals from it.
// If the next word of this text is Tiger, Wolf, or Fox, the function should place the corresponding animal in the zoo.
// Otherwise, it should stop reading and throw a runtime_error exception.
Zoo CreateZoo(istream &in) {
    Zoo zoo;
    string word;
    while (in >> word) {
        if (word == "Tiger") {
            zoo.push_back(make_unique<Tiger>());
        } else if (word == "Wolf") {
            zoo.push_back(make_unique<Wolf>());
        } else if (word == "Fox") {
            zoo.push_back(make_unique<Fox>());
        } else {
            throw runtime_error("Unknown animal!");
        }
    }
    return zoo;
}

// This function should iterate over all the animals in the zoo in the order of their creation
// and write to the output stream the result of the virtual function voice for each of them.
// Separate the voices of different animals with the newline character '\n'.
void Process(const Zoo &zoo, ostream &out) {
    for (const auto &animal: zoo) {
        out << animal->Voice() << "\n";
    }
}

void TestZoo() {
    istringstream input("Tiger Wolf Fox Tiger");
    ostringstream output;
    Process(CreateZoo(input), output);

    const string expected =
            "Rrrr\n"
            "Wooo\n"
            "Tyaf\n"
            "Rrrr\n";

    ASSERT_EQUAL(output.str(), expected);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestZoo);
}
