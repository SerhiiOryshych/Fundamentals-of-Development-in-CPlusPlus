#include "test_runner.h"
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;


struct Email {
    string from;
    string to;
    string body;
};

class Worker {
public:
    virtual ~Worker() = default;

    virtual void Process(unique_ptr<Email> email) = 0;

    virtual void Run() {
        throw logic_error("Unimplemented");
    }

protected:
    void PassOn(unique_ptr<Email> email) const {
        if (next_worker) {
            next_worker->Process(move(email));
        }
    }

public:
    void SetNext(unique_ptr<Worker> next) {
        next_worker = move(next);
    }

private:
    unique_ptr<Worker> next_worker;
};


class Reader : public Worker {
public:
    Reader(istream &st) : stream(st) {}

    void Run() override {
        Email email;
        while (getline(stream, email.from) && getline(stream, email.to) && getline(stream, email.body)) {
            Process(make_unique<Email>(email));
        }
    }

    void Process(unique_ptr<Email> email) override {
        PassOn(move(email));
    }

private:
    istream &stream;
};


class Filter : public Worker {
public:
    using Function = function<bool(const Email &)>;

public:
    Filter(Function func) : func(func) {}

    void Process(unique_ptr<Email> email) override {
        if (func(*email.get())) {
            PassOn(move(email));
        }
    }

private:
    Function func;
};


class Copier : public Worker {
public:
    Copier(string to) : copy_to(move(to)) {}

    void Process(unique_ptr<Email> email) override {
        Email copy_email = *email.get();

        PassOn(move(email));

        if (copy_to != copy_email.to) {
            copy_email.to = copy_to;
            PassOn(make_unique<Email>(copy_email));
        }
    }

private:
    string copy_to;
};


class Sender : public Worker {
public:
    Sender(ostream &st) : stream(st) {}

    void Process(unique_ptr<Email> email) override {
        stream << email->from << "\n" << email->to << "\n" << email->body << "\n";
        PassOn(move(email));
    }

private:
    ostream &stream;
};

class PipelineBuilder {
public:
    explicit PipelineBuilder(istream &in) {
        reader = make_unique<Reader>(in);
        last_worker = reader.get();
    }

    PipelineBuilder &FilterBy(Filter::Function filter) {
        auto filter_worker = make_unique<Filter>(filter);
        auto filter_worker_prt = filter_worker.get();
        if (last_worker) {
            last_worker->SetNext(move(filter_worker));
        }
        last_worker = filter_worker_prt;
        return *this;
    }

    PipelineBuilder &CopyTo(string recipient) {
        auto copier_worker = make_unique<Copier>(move(recipient));
        auto copier_worker_ptr = copier_worker.get();
        if (last_worker) {
            last_worker->SetNext(move(copier_worker));
        }
        last_worker = copier_worker_ptr;
        return *this;
    }

    PipelineBuilder &Send(ostream &out) {
        auto send_worker = make_unique<Sender>(out);
        auto send_worker_ptr = send_worker.get();
        if (last_worker) {
            last_worker->SetNext(move(send_worker));
        }
        last_worker = send_worker_ptr;
        return *this;
    }

    unique_ptr<Worker> Build() {
        return move(reader);
    }

private:
    unique_ptr<Reader> reader;
    Worker *last_worker = nullptr;
};


void TestSanity() {
    string input = (
            "erich@example.com\n"
            "richard@example.com\n"
            "Hello there\n"

            "erich@example.com\n"
            "ralph@example.com\n"
            "Are you sure you pressed the right button?\n"

            "ralph@example.com\n"
            "erich@example.com\n"
            "I do not make mistakes of that kind\n"
    );
    istringstream inStream(input);
    ostringstream outStream;

    PipelineBuilder builder(inStream);
    builder.FilterBy([](const Email &email) {
        return email.from == "erich@example.com";
    });
    builder.CopyTo("richard@example.com");
    builder.Send(outStream);
    auto pipeline = builder.Build();

    pipeline->Run();

    string expectedOutput = (
            "erich@example.com\n"
            "richard@example.com\n"
            "Hello there\n"

            "erich@example.com\n"
            "ralph@example.com\n"
            "Are you sure you pressed the right button?\n"

            "erich@example.com\n"
            "richard@example.com\n"
            "Are you sure you pressed the right button?\n"
    );

    ASSERT_EQUAL(expectedOutput, outStream.str());
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestSanity);
    return 0;
}
