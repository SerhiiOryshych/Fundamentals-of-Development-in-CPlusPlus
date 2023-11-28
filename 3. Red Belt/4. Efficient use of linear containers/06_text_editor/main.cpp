#include <string>
#include <iterator>
#include <list>
#include <cmath>
#include "test_runner.h"

using namespace std;

class Editor {
public:
    Editor() : pos(data.begin()) {}

    void Left() {
        if (pos == data.begin()) {
            return;
        }
        pos--;
    }

    void Right() {
        if (pos == data.end()) {
            return;
        }
        pos++;
    }

    void Insert(char token) {
        data.insert(pos, token);
    }

    void Cut(size_t tokens = 1) {
        const size_t dist_to_end = distance(pos, data.end());
        buffer = {pos, next(pos, min(tokens, dist_to_end))};
        pos = data.erase(pos, next(pos, min(tokens, dist_to_end)));
    }

    void Copy(size_t tokens = 1) {
        buffer = {pos, next(pos, tokens)};
    }

    void Paste() {
        data.insert(pos, buffer.begin(), buffer.end());
    }

    [[nodiscard]] string GetText() const {
        return {data.begin(), data.end()};
    }

private:
    using list_it = list<char>::iterator;
    list<char> data, buffer;
    list_it pos;
};

void TypeText(Editor &editor, const string &text) {
    for (char c: text) {
        editor.Insert(c);
    }
}

void TestEditing() {
    {
        Editor editor;

        const size_t text_len = 12;
        const size_t first_part_len = 7;
        TypeText(editor, "hello, world");
        for (size_t i = 0; i < text_len; ++i) {
            editor.Left();
        }
        editor.Cut(first_part_len);
        for (size_t i = 0; i < text_len - first_part_len; ++i) {
            editor.Right();
        }
        TypeText(editor, ", ");
        editor.Paste();
        editor.Left();
        editor.Left();
        editor.Cut(3);

        ASSERT_EQUAL(editor.GetText(), "world, hello");
    }
    {
        Editor editor;

        TypeText(editor, "misprnit");
        editor.Left();
        editor.Left();
        editor.Left();
        editor.Cut(1);
        editor.Right();
        editor.Paste();

        ASSERT_EQUAL(editor.GetText(), "misprint");
    }
}

void TestReverse() {
    Editor editor;

    const string text = "esreveR";
    for (char c: text) {
        editor.Insert(c);
        editor.Left();
    }

    ASSERT_EQUAL(editor.GetText(), "Reverse");
}

void TestNoText() {
    Editor editor;
    ASSERT_EQUAL(editor.GetText(), "");

    editor.Left();
    editor.Left();
    editor.Right();
    editor.Right();
    editor.Copy(0);
    editor.Cut(0);
    editor.Paste();

    ASSERT_EQUAL(editor.GetText(), "");
}

void TestEmptyBuffer() {
    Editor editor;

    editor.Paste();
    TypeText(editor, "example");
    editor.Left();
    editor.Left();
    editor.Paste();
    editor.Right();
    editor.Paste();
    editor.Copy(0);
    editor.Paste();
    editor.Left();
    editor.Cut(0);
    editor.Paste();

    ASSERT_EQUAL(editor.GetText(), "example");
}

void Test() {
    Editor editor;
    TypeText(editor, "1234567");
    editor.Left();
    editor.Left();
    editor.Cut(1);
    editor.Paste();
    editor.Paste();
    editor.Paste();
    ASSERT_EQUAL(editor.GetText(), "123456667");
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestEditing);
    RUN_TEST(tr, TestReverse);
    RUN_TEST(tr, TestNoText);
    RUN_TEST(tr, TestEmptyBuffer);
    RUN_TEST(tr, Test);
    return 0;
}
