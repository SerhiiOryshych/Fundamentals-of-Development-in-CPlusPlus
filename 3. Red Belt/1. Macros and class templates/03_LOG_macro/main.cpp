#include "test_runner.h"
#include <sstream>
#include <string>

using namespace std;

class Logger {
public:
    explicit Logger(ostream &output_stream) : os(output_stream) {
    }

    void SetLogLine(bool value) { log_line = value; }

    void SetLogFile(bool value) { log_file = value; }

    void Log(const string &message);

    void LogFileLine(const string &file, int line) const;

private:
    ostream &os;
    bool log_line = false;
    bool log_file = false;
};

void Logger::LogFileLine(const string &file, int line) const {
    if (log_file) {
        os << file;
        if (log_line) {
            os << ":" << line << " ";
        } else {
            os << " ";
        }
    } else {
        if (log_line) {
            os << "Line " << line << " ";
        }
    }
}

void Logger::Log(const string &message) {
    os << message << endl;
}

#define LOG(logger, message)                \
    logger.LogFileLine(__FILE__, __LINE__); \
    logger.Log(message)

void TestLog() {
/* To write unit tests in this task, we need to fix specific
 * line numbers in the expected value (see the variable 'expected' below).
 * If we add some code above the function TestLog, these line numbers change,
 * and our test starts to fail. This is inconvenient.
 * To avoid this, we use a special macro #line (http://en.cppreference.com/w/cpp/preprocessor/line),
 * which allows redefining the line number, as well as the file name.
 * Thanks to it, the line numbers inside the function TestLog will be fixed regardless of what code we add before it
 */
#line 1 "logger.cpp"

    ostringstream logs;
    Logger l(logs);
    LOG(l, "hello");

    l.SetLogFile(true);
    LOG(l, "hello");

    l.SetLogLine(true);
    LOG(l, "hello");

    l.SetLogFile(false);
    LOG(l, "hello");

    string expected = "hello\n";
    expected += "logger.cpp hello\n";
    expected += "logger.cpp:10 hello\n";
    expected += "Line 13 hello\n";
    ASSERT_EQUAL(logs.str(), expected);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestLog);
}
