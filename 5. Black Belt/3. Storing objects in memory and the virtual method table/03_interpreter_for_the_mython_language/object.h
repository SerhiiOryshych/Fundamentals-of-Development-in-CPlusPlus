#pragma once

#include "object_holder.h"

#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace Ast {
    class Statement;
}

class TestRunner;

namespace Runtime {

    struct RuntimeError : std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    class Object {
    public:
        virtual ~Object() = default;

        virtual void Print(std::ostream &os) = 0;

        [[nodiscard]] virtual bool IsTrue() const = 0;
    };

    template<typename T>
    class ValueObject : public Object {
    public:
        ValueObject(T v) : value(v) {}

        void Print(std::ostream &os) override { os << value; }

        [[nodiscard]] const T &GetValue() const { return value; }

        [[nodiscard]] bool IsTrue() const override { return false; }

    protected:
        T value;
    };

    class String : public ValueObject<std::string> {
    public:
        using ValueObject<std::string>::ValueObject;

        [[nodiscard]] bool IsTrue() const override { return !value.empty(); }
    };

    class Number : public ValueObject<int> {
    public:
        using ValueObject<int>::ValueObject;

        [[nodiscard]] bool IsTrue() const override { return value != 0; }
    };

    class Bool : public ValueObject<bool> {
    public:
        using ValueObject<bool>::ValueObject;

        void Print(std::ostream &os) override;

        [[nodiscard]] bool IsTrue() const override { return value; }
    };

    struct Method {
        std::string name;
        std::vector<std::string> formal_params;
        std::unique_ptr<Ast::Statement> body;
    };

    class Class : public Object {
    public:
        explicit Class(std::string name, std::vector<Method> methods,
                       const Class *parent);

        [[nodiscard]] const Method *GetMethod(const std::string &name) const;

        [[nodiscard]] const std::string &GetName() const;

        void Print(std::ostream &os) override;

        [[nodiscard]] bool IsTrue() const override { return true; }

    private:
        const std::string name_;

        using MethodMap = std::unordered_map<std::string, Method>;
        std::unique_ptr<MethodMap> methods_;

        const Class *parent_;
    };

    class ClassInstance : public Object {
    public:
        explicit ClassInstance(const Class &cls);

        void Print(std::ostream &os) override;

        ObjectHolder Call(const std::string &method_name,
                          const std::vector<ObjectHolder> &actual_args);

        [[nodiscard]] bool HasMethod(const std::string &method,
                                     size_t argument_count) const;

        Closure &Fields();

        [[nodiscard]] const Closure &Fields() const;

        [[nodiscard]] bool IsTrue() const override { return true; }

    private:
        const Class &class_;
        Closure fields_;

        void UpdateFields();

        friend bool Equal(ObjectHolder lhs, ObjectHolder rhs);
    };

    void RunObjectsTests(TestRunner &test_runner);

} // namespace Runtime
