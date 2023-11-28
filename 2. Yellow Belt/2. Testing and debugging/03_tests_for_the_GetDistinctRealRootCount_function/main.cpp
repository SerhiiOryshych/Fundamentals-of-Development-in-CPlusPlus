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

int GetDistinctRealRootCount(double a, double b, double c);

void TwoRoots() {
    AssertEqual(GetDistinctRealRootCount(1, 2, -3), 2);
    AssertEqual(GetDistinctRealRootCount(-23.4, -3.7, 356.893), 2);
    AssertEqual(GetDistinctRealRootCount(-23.4, 0, 356.893), 2);
    AssertEqual(GetDistinctRealRootCount(-23.4, -3.7, 0), 2);
    AssertEqual(GetDistinctRealRootCount(-23.4, -3.7, 0), 2);
}

void OneRoot() {
    AssertEqual(GetDistinctRealRootCount(-23.4, 0, 0), 1);
    AssertEqual(GetDistinctRealRootCount(1, -6, 9), 1);
    AssertEqual(GetDistinctRealRootCount(0, -567.37, 47.099), 1);
    AssertEqual(GetDistinctRealRootCount(0, -567.37, 0), 1);
}

void ZeroRoots() {
    AssertEqual(GetDistinctRealRootCount(-473738.0037, -4.4673, -463.283), 0);
    AssertEqual(GetDistinctRealRootCount(-473738.0037, 0, -463.283), 0);
    AssertEqual(GetDistinctRealRootCount(0, 0, -463.283), 0);
    AssertEqual(GetDistinctRealRootCount(0, 0, 463.283), 0);
    AssertEqual(GetDistinctRealRootCount(2, 6, 9), 0);
}

int main() {
    TestRunner runner;
    runner.RunTest(TwoRoots, "TwoRoots");
    runner.RunTest(OneRoot, "OneRoot");
    runner.RunTest(ZeroRoots, "ZeroRoots");
    return 0;
}
