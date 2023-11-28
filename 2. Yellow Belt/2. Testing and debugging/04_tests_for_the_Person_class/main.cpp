#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

template<class T>
ostream &operator<<(ostream &os, const vector<T> &s) {
    os << "{";
    bool first = true;
    for (const auto &x: s) {
        if (!first) {
            os << ", ";
        }
        first = false;
        os << x;
    }
    return os << "}";
}

template<class T>
ostream &operator<<(ostream &os, const set<T> &s) {
    os << "{";
    bool first = true;
    for (const auto &x: s) {
        if (!first) {
            os << ", ";
        }
        first = false;
        os << x;
    }
    return os << "}";
}

template<class K, class V>
ostream &operator<<(ostream &os, const map<K, V> &m) {
    os << "{";
    bool first = true;
    for (const auto &kv: m) {
        if (!first) {
            os << ", ";
        }
        first = false;
        os << kv.first << ": " << kv.second;
    }
    return os << "}";
}

template<class T, class U>
void AssertEqual(const T &t, const U &u, const string &hint = {}) {
    if (t != u) {
        ostringstream os;
        os << "Assertion failed: " << t << " != " << u;
        if (!hint.empty()) {
            os << " hint: " << hint;
        }
        throw runtime_error(os.str());
    }
}

void Assert(bool b, const string &hint) {
    AssertEqual(b, true, hint);
}

class TestRunner {
public:
    template<class TestFunc>
    void RunTest(TestFunc func, const string &test_name) {
        try {
            func();
            cerr << test_name << " OK" << endl;
        } catch (exception &e) {
            ++fail_count;
            cerr << test_name << " fail: " << e.what() << endl;
        } catch (...) {
            ++fail_count;
            cerr << "Unknown exception caught" << endl;
        }
    }

    ~TestRunner() {
        if (fail_count > 0) {
            cerr << fail_count << " unit tests failed. Terminate" << endl;
            exit(1);
        }
    }

private:
    int fail_count = 0;
};

class Person {
public:
    // You can insert various implementations of the class here,
    // to check that your tests allow correct code
    // and catch incorrect code.
    void ChangeFirstName(int year, const string &first_name) {
    }

    void ChangeLastName(int year, const string &last_name) {
    }

    string GetFullName(int year) {
        return "full_name";
    }
};

void TestIncognito() {
    {
        Person p;
        AssertEqual(p.GetFullName(2023), "Incognito");
    }
    {
        Person p;
        p.ChangeFirstName(2024, "Serhii");
        AssertEqual(p.GetFullName(2023), "Incognito");
    }
    {
        Person p;
        p.ChangeLastName(2024, "Oryshych");
        AssertEqual(p.GetFullName(2023), "Incognito");
    }
    {
        Person p;
        p.ChangeFirstName(2024, "Serhii");
        p.ChangeLastName(2024, "Oryshych");
        AssertEqual(p.GetFullName(2023), "Incognito");
    }
}

void TestUnknownFirstName() {
    {
        Person p;
        p.ChangeLastName(2023, "Oryshych_2023");
        AssertEqual(p.GetFullName(2023), "Oryshych_2023 with unknown first name");
    }
    {
        Person p;
        p.ChangeLastName(2023, "Oryshych_2023");
        p.ChangeFirstName(2024, "Serhii_2024");
        AssertEqual(p.GetFullName(2023), "Oryshych_2023 with unknown first name");
    }
    {
        Person p;
        p.ChangeLastName(2023, "Oryshych_2023");
        p.ChangeFirstName(2024, "Serhii_2024");
        AssertEqual(p.GetFullName(2023), "Oryshych_2023 with unknown first name");
        p.ChangeLastName(2021, "Oryshych_2021");
        AssertEqual(p.GetFullName(2022), "Oryshych_2021 with unknown first name");
    }
    {
        Person p;
        p.ChangeFirstName(2024, "Serhii_2024");
        p.ChangeFirstName(2023, "Serhii_2023");
        p.ChangeLastName(2020, "Oryshych_2020");
        p.ChangeLastName(2021, "Oryshych_2021");
        AssertEqual(p.GetFullName(2022), "Oryshych_2021 with unknown first name");
    }
}

void TestUnknownLastName() {
    {
        Person p;
        p.ChangeFirstName(2023, "Serhii_2023");
        AssertEqual(p.GetFullName(2023), "Serhii_2023 with unknown last name");
    }
    {
        Person p;
        p.ChangeFirstName(2023, "Serhii_2023");
        p.ChangeLastName(2024, "Oryshych_2024");
        AssertEqual(p.GetFullName(2023), "Serhii_2023 with unknown last name");
    }
    {
        Person p;
        p.ChangeFirstName(2023, "Serhii_2023");
        p.ChangeLastName(2024, "Oryshych_2024");
        AssertEqual(p.GetFullName(2023), "Serhii_2023 with unknown last name");
        p.ChangeFirstName(2021, "Serhii_2021");
        AssertEqual(p.GetFullName(2022), "Serhii_2021 with unknown last name");
    }
    {
        Person p;
        p.ChangeLastName(2024, "Oryshych_2024");
        p.ChangeLastName(2023, "Oryshych_2023");
        p.ChangeFirstName(2020, "Serhii_2020");
        p.ChangeFirstName(2021, "Serhii_2021");
        AssertEqual(p.GetFullName(2022), "Serhii_2021 with unknown last name");
    }
}

void TestFirstNameAndLastName() {
    {
        Person p;
        p.ChangeFirstName(2023, "Serhii_2023");
        p.ChangeFirstName(2020, "Serhii_2020");
        p.ChangeFirstName(2021, "Serhii_2021");
        p.ChangeLastName(2024, "Oryshych_2024");
        p.ChangeLastName(2020, "Oryshych_2020");
        p.ChangeLastName(2021, "Oryshych_2021");
        AssertEqual(p.GetFullName(2021), "Serhii_2021 Oryshych_2021");
        AssertEqual(p.GetFullName(2022), "Serhii_2021 Oryshych_2021");
        AssertEqual(p.GetFullName(2023), "Serhii_2023 Oryshych_2021");
        AssertEqual(p.GetFullName(2024), "Serhii_2023 Oryshych_2024");
        AssertEqual(p.GetFullName(2025), "Serhii_2023 Oryshych_2024");
    }
}

int main() {
    TestRunner runner;
    runner.RunTest(TestIncognito, "TestIncognito");
    runner.RunTest(TestUnknownFirstName, "TestUnknownFirstName");
    runner.RunTest(TestUnknownLastName, "TestUnknownLastName");
    runner.RunTest(TestFirstNameAndLastName, "TestFirstNameAndLastName");
    return 0;
}
