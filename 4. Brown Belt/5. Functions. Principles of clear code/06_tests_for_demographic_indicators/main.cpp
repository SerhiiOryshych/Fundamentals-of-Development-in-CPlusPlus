#include "test_runner.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

enum class Gender {
    FEMALE,
    MALE
};

struct Person {
    int age;
    Gender gender;
    bool is_employed;
};

bool operator==(const Person &lhs, const Person &rhs) {
    return lhs.age == rhs.age
           && lhs.gender == rhs.gender
           && lhs.is_employed == rhs.is_employed;
}

void TestOperatorEqual() {
    {
        Person p1 = {12, Gender::FEMALE, true};
        Person p2 = {12, Gender::FEMALE, true};
        Assert(p1 == p2, "TestOperatorEqual_1");
    }
    {
        Person p1 = {13, Gender::FEMALE, true};
        Person p2 = {12, Gender::FEMALE, true};
        Assert(!(p1 == p2), "TestOperatorEqual_2");
    }
    {
        Person p1 = {12, Gender::MALE, true};
        Person p2 = {12, Gender::FEMALE, true};
        Assert(!(p1 == p2), "TestOperatorEqual_3");
    }
    {
        Person p1 = {12, Gender::FEMALE, false};
        Person p2 = {12, Gender::FEMALE, true};
        Assert(!(p1 == p2), "TestOperatorEqual_1");
    }
}

ostream &operator<<(ostream &stream, const Person &person) {
    return stream << "Person(age=" << person.age
                  << ", gender=" << static_cast<int>(person.gender)
                  << ", is_employed=" << boolalpha << person.is_employed << ")";
}

struct AgeStats {
    int total;
    int females;
    int males;
    int employed_females;
    int unemployed_females;
    int employed_males;
    int unemployed_males;
};

template<typename InputIt>
int ComputeMedianAge(InputIt range_begin, InputIt range_end) {
    if (range_begin == range_end) {
        return 0;
    }
    vector<typename iterator_traits<InputIt>::value_type> range_copy(
            range_begin,
            range_end
    );
    auto middle = begin(range_copy) + range_copy.size() / 2;
    nth_element(
            begin(range_copy), middle, end(range_copy),
            [](const Person &lhs, const Person &rhs) {
                return lhs.age < rhs.age;
            }
    );
    return middle->age;
}

void TestComputeMedianAge() {
    {
        vector<Person> persons = {};
        AssertEqual(ComputeMedianAge(persons.begin(), persons.end()), 0);
    }
    {
        vector<Person> persons = {
                {1, Gender::MALE, true},
                {2, Gender::MALE, true},
                {3, Gender::MALE, true}
        };
        AssertEqual(ComputeMedianAge(persons.begin(), persons.end()), 2);
    }
    {
        vector<Person> persons = {
                {1, Gender::MALE, true},
                {2, Gender::MALE, true},
                {3, Gender::MALE, true},
                {4, Gender::MALE, true}
        };
        AssertEqual(ComputeMedianAge(persons.begin(), persons.end()), 3);
    }
}

vector<Person> ReadPersons(istream &in_stream = cin) {
    int person_count;
    in_stream >> person_count;
    vector<Person> persons;
    persons.reserve(person_count);
    for (int i = 0; i < person_count; ++i) {
        int age, gender, is_employed;
        in_stream >> age >> gender >> is_employed;
        Person person{
                age,
                static_cast<Gender>(gender),
                is_employed == 1
        };
        persons.push_back(person);
    }
    return persons;
}

void TestReadPersons() {
    stringstream input("4 \n 12 0 1 \n 21 1 0 \n 34 0 0 \n 56 1 1");
    vector<Person> expected = {{12, Gender::FEMALE, true},
                               {21, Gender::MALE,   false},
                               {34, Gender::FEMALE, false},
                               {56, Gender::MALE,   true}};
    AssertEqual(ReadPersons(input), expected, "TestReadPersons");
}

AgeStats ComputeStats(vector<Person> persons) {
    //                 persons
    //                |       |
    //          females        males
    //         |       |      |     |
    //      empl.  unempl. empl.   unempl.

    auto females_end = partition(
            begin(persons), end(persons),
            [](const Person &p) {
                return p.gender == Gender::FEMALE;
            }
    );
    auto employed_females_end = partition(
            begin(persons), females_end,
            [](const Person &p) {
                return p.is_employed;
            }
    );
    auto employed_males_end = partition(
            females_end, end(persons),
            [](const Person &p) {
                return p.is_employed;
            }
    );

    return {
            ComputeMedianAge(begin(persons), end(persons)),
            ComputeMedianAge(begin(persons), females_end),
            ComputeMedianAge(females_end, end(persons)),
            ComputeMedianAge(begin(persons), employed_females_end),
            ComputeMedianAge(employed_females_end, females_end),
            ComputeMedianAge(females_end, employed_males_end),
            ComputeMedianAge(employed_males_end, end(persons))
    };
}

void TestComputeStats() {
    {
        vector<Person> persons = {
                {1, Gender::FEMALE, true},
                {9, Gender::MALE,   false},
                {4, Gender::MALE,   false},
                {8, Gender::FEMALE, false},
                {2, Gender::FEMALE, true},
                {7, Gender::MALE,   true},
                {5, Gender::MALE,   true},
                {3, Gender::FEMALE, true},
                {3, Gender::MALE,   false},
                {6, Gender::MALE,   false},
                {8, Gender::FEMALE, false},
                {2, Gender::FEMALE, true},
                {1, Gender::MALE,   true},
                {7, Gender::MALE,   true}
        };
        AgeStats expected_stats = {
                5, 3, 6, 2, 8, 7, 6
        };
        auto stats = ComputeStats(persons);
        AssertEqual(stats.total, expected_stats.total);
        AssertEqual(stats.females, expected_stats.females);
        AssertEqual(stats.males, expected_stats.males);
        AssertEqual(stats.employed_females, expected_stats.employed_females);
        AssertEqual(stats.unemployed_females, expected_stats.unemployed_females);
        AssertEqual(stats.employed_males, expected_stats.employed_males);
        AssertEqual(stats.unemployed_males, expected_stats.unemployed_males);
    }
}

void PrintStats(const AgeStats &stats,
                ostream &out_stream = cout) {
    out_stream << "Median age = "
               << stats.total << endl
               << "Median age for females = "
               << stats.females << endl
               << "Median age for males = "
               << stats.males << endl
               << "Median age for employed females = "
               << stats.employed_females << endl
               << "Median age for unemployed females = "
               << stats.unemployed_females << endl
               << "Median age for employed males = "
               << stats.employed_males << endl
               << "Median age for unemployed males = "
               << stats.unemployed_males << endl;
}

void TestPrintStats() {
    AgeStats stats = {
            1, 2, 3, 4, 5, 6, 7
    };
    stringstream expected_output(
            "Median age = 1\nMedian age for females = 2\nMedian age for males = 3\nMedian age for employed females = 4\nMedian age for unemployed females = 5\nMedian age for employed males = 6\nMedian age for unemployed males = 7\n");
    stringstream output;
    PrintStats(stats, output);
    AssertEqual(output.str(), expected_output.str());
}

int main() {
    TestRunner testRunner;
    RUN_TEST(testRunner, TestOperatorEqual);
    RUN_TEST(testRunner, TestReadPersons);
    RUN_TEST(testRunner, TestComputeMedianAge);
    RUN_TEST(testRunner, TestComputeStats);
    RUN_TEST(testRunner, TestPrintStats);
}
