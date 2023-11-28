#include "test_runner.h"

#include <cassert>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

class EmptyContext {
};

template<typename ParentContext>
class JsonContext;

template<typename ParentContext>
class ArrayContext;

template<typename ParentContext>
class ObjectKeyContext;

template<typename ParentContext>
class ObjectValueContext;

void PrintJsonString(ostream &, string_view);

ArrayContext<EmptyContext> PrintJsonArray(ostream &);

ObjectKeyContext<EmptyContext> PrintJsonObject(ostream &);

template<typename ParentContext>
class JsonContext {
public:
    JsonContext(ostream &out, ParentContext &parent)
            : out_(out), parent_(parent) {}

protected:
    ostream &out_;
    ParentContext &parent_;
};

template<typename ParentContext>
class ArrayContext : public JsonContext<ParentContext> {
public:
    using Self = ArrayContext<ParentContext>;

    ArrayContext(ostream &out, ParentContext &parent)
            : JsonContext<ParentContext>(out, parent) {
        this->out_ << '[';
    }

    ~ArrayContext() { EndArray(); }

    ArrayContext &Number(int64_t value) {
        MaybePrintComma();
        this->out_ << value;
        return *this;
    }

    ArrayContext &String(string_view value) {
        MaybePrintComma();
        PrintJsonString(this->out_, value);
        return *this;
    }

    ArrayContext &Boolean(bool value) {
        MaybePrintComma();
        this->out_ << (value ? "true" : "false");
        return *this;
    }

    ArrayContext &Null() {
        MaybePrintComma();
        this->out_ << "null";
        return *this;
    }

    ArrayContext<Self> BeginArray() {
        MaybePrintComma();
        return ArrayContext<Self>(this->out_, *this);
    }

    ParentContext &EndArray() {
        if (!array_ended_) {
            this->out_ << ']';
            array_ended_ = true;
        }
        return this->parent_;
    }

    ObjectKeyContext<Self> BeginObject() {
        MaybePrintComma();
        return ObjectKeyContext<Self>(this->out_, *this);
    }

private:
    bool first_printed_ = false;
    bool array_ended_ = false;

    void MaybePrintComma() {
        if (first_printed_) {
            this->out_ << ',';
        }
        first_printed_ = true;
    }
};

template<typename ParentContext>
class ObjectKeyContext : public JsonContext<ParentContext> {
public:
    using Self = ObjectKeyContext<ParentContext>;

    ObjectKeyContext(ostream &out, ParentContext &parent)
            : JsonContext<ParentContext>(out, parent) {
        this->out_ << '{';
    }

    ~ObjectKeyContext() { EndObject(); }

    ObjectValueContext<Self> Key(string_view value) {
        MaybePrintComma();
        PrintJsonString(this->out_, value);
        return ObjectValueContext<Self>(this->out_, *this);
    }

    ParentContext &EndObject() {
        if (!object_ended_) {
            this->out_ << '}';
        }
        object_ended_ = true;
        return this->parent_;
    }

private:
    bool first_printed_ = false;
    bool object_ended_ = false;

    void MaybePrintComma() {
        if (first_printed_) {
            this->out_ << ',';
        }
        first_printed_ = true;
    }
};

template<typename ParentContext>
class ObjectValueContext : public JsonContext<ParentContext> {
public:
    ObjectValueContext(ostream &out, ParentContext &parent)
            : JsonContext<ParentContext>(out, parent) {
        this->out_ << ':';
    }

    ~ObjectValueContext() {
        if (!value_filled_) {
            Null();
        }
    }

    ParentContext &Number(int64_t value) {
        value_filled_ = true;
        this->out_ << value;
        return this->parent_;
    }

    ParentContext &String(string_view value) {
        value_filled_ = true;
        PrintJsonString(this->out_, value);
        return this->parent_;
    }

    ParentContext &Boolean(bool value) {
        value_filled_ = true;
        this->out_ << (value ? "true" : "false");
        return this->parent_;
    }

    ParentContext &Null() {
        if (!value_filled_) {
            this->out_ << "null";
        }
        value_filled_ = true;
        return this->parent_;
    }

    ArrayContext<ParentContext> BeginArray() {
        value_filled_ = true;
        return ArrayContext<ParentContext>(this->out_, this->parent_);
    }

    ObjectKeyContext<ParentContext> BeginObject() {
        value_filled_ = true;
        return ObjectKeyContext<ParentContext>(this->out_, this->parent_);
    }

private:
    bool value_filled_ = false;
};

void PrintJsonString(ostream &out, string_view str) {
    ostringstream os;
    os << quoted(str);
    out << os.str();
}

ArrayContext<EmptyContext> PrintJsonArray(ostream &out) {
    static EmptyContext empty;
    return ArrayContext<EmptyContext>{out, empty};
}

ObjectKeyContext<EmptyContext> PrintJsonObject(ostream &out) {
    static EmptyContext empty;
    return ObjectKeyContext<EmptyContext>{out, empty};
}

void TestArray() {
    ostringstream output;

    {
        auto json = PrintJsonArray(output);
        json.Number(5).Number(6).BeginArray().Number(7).EndArray().Number(8).String(
                "bingo!");
    }

    ASSERT_EQUAL(output.str(), R"([5,6,[7],8,"bingo!"])");
}

void TestObject() {
    ostringstream output;

    {
        auto json = PrintJsonObject(output);
        json.Key("id1")
                .Number(1234)
                .Key("id2")
                .Boolean(false)
                .Key("")
                .Null()
                .Key("\"")
                .String("\\");
    }

    ASSERT_EQUAL(output.str(), R"({"id1":1234,"id2":false,"":null,"\"":"\\"})");
}

void TestAutoClose() {
    ostringstream output;

    {
        auto json = PrintJsonArray(output);
        json.BeginArray().BeginObject();
    }

    ASSERT_EQUAL(output.str(), R"([[{}]])");
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestArray);
    RUN_TEST(tr, TestObject);
    RUN_TEST(tr, TestAutoClose);

/*
    PrintJsonString(cout, "Hello, \"world\"");
*/

/*
    PrintJsonArray(cout)
            .Null()
            .String("Hello")
            .Number(123)
            .Boolean(false)
            .EndArray();
*/

/*
    PrintJsonArray(cout)
            .Null()
            .String("Hello")
            .Number(123)
            .Boolean(false);
*/

/*
    PrintJsonArray(cout)
            .String("Hello")
            .BeginArray()
            .String("World");
*/

/*
    PrintJsonObject(cout)
            .Key("foo")
            .BeginArray()
            .String("Hello")
            .EndArray()
            .Key("foo")
            .BeginObject()
            .Key("foo");
*/

    return 0;
}
