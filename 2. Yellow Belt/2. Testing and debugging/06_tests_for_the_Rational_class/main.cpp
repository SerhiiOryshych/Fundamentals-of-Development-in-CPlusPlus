#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
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

class Rational {
public:
    Rational();

    Rational(int numerator, int denominator) {
    }

    int Numerator() const {
        return 0;
    }

    int Denominator() const {
        return 1;
    }
};

void Test1() {
    Rational r;
    AssertEqual(r.Numerator(), 0);
    AssertEqual(r.Denominator(), 1);
}

void Test2() {
    {
        Rational r = {2, 4};
        AssertEqual(r.Numerator(), 1);
        AssertEqual(r.Denominator(), 2);
    }
    {
        Rational r = {9, 6};
        AssertEqual(r.Numerator(), 3);
        AssertEqual(r.Denominator(), 2);
    }
}

void Test3() {
    {
        Rational r = {-2, 4};
        AssertEqual(r.Numerator(), -1);
        AssertEqual(r.Denominator(), 2);
    }
    {
        Rational r = {9, -6};
        AssertEqual(r.Numerator(), -3);
        AssertEqual(r.Denominator(), 2);
    }
}

void Test4() {
    {
        Rational r = {-18, -12};
        AssertEqual(r.Numerator(), 3);
        AssertEqual(r.Denominator(), 2);
    }
}

void Test5() {
    {
        Rational r = {0, 10};
        AssertEqual(r.Numerator(), 0);
        AssertEqual(r.Denominator(), 1);
    }
    {
        Rational r = {0, -1};
        AssertEqual(r.Numerator(), 0);
        AssertEqual(r.Denominator(), 1);
    }
    {
        Rational r = {0, -12};
        AssertEqual(r.Numerator(), 0);
        AssertEqual(r.Denominator(), 1);
    }
}

int main() {
    TestRunner runner;
    runner.RunTest(Test1, "Test1");
    runner.RunTest(Test2, "Test2");
    runner.RunTest(Test3, "Test3");
    runner.RunTest(Test4, "Test4");
    runner.RunTest(Test5, "Test5");
    return 0;
}
