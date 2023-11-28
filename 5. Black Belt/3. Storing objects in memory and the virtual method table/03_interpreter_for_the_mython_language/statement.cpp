#include "statement.h"
#include "object.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace Ast {

    using Runtime::Closure;

    ObjectHolder Assignment::Execute(Closure &closure) {
        const auto &new_value = right_value->Execute(closure);
        closure[var_name] = new_value;
        return new_value;
    }

    Assignment::Assignment(string var, unique_ptr <Statement> rv)
            : var_name(std::move(var)), right_value(std::move(rv)) {}

    VariableValue::VariableValue(string var_name) {
        dotted_ids.push_back(std::move(var_name));
    }

    VariableValue::VariableValue(vector <string> dotted_ids) {
        this->dotted_ids = std::move(dotted_ids);
    }

    ObjectHolder VariableValue::Execute(Closure &closure) {
        return ExecuteChain(0, closure);
    }

    ObjectHolder VariableValue::ExecuteChain(size_t index,
                                             Runtime::Closure &closure) {
        if (closure.find(dotted_ids[index]) == closure.end())
            throw Runtime::RuntimeError("No such variable!");

        if (index + 1 == dotted_ids.size()) {
            return closure[dotted_ids[index]];
        } else {
            return ExecuteChain(
                    index + 1,
                    closure[dotted_ids[index]].TryAs<Runtime::ClassInstance>()->Fields());
        }
    }

    unique_ptr <Print> Print::Variable(string var) {
        return make_unique<Print>(make_unique<VariableValue>(std::move(var)));
    }

    Print::Print(unique_ptr <Statement> argument) {
        args.push_back(std::move(argument));
    }

    Print::Print(vector <unique_ptr<Statement>> args) : args(std::move(args)) {}

    ObjectHolder Print::Execute(Closure &closure) {
        if (!output) {
            return Runtime::ObjectHolder::None();
        }

        bool first = true;
        for (auto &statement: args) {
            if (!first) {
                *output << ' ';
            }
            ObjectHolder objectHolder = statement->Execute(closure);
            if (objectHolder) {
                objectHolder->Print(*output);
            } else {
                *output << "None";
            }
            first = false;
        }
        *output << '\n';

        return Runtime::ObjectHolder::None();
    }

    ostream *Print::output = &cout;

    void Print::SetOutputStream(ostream &output_stream) { output = &output_stream; }

    MethodCall::MethodCall(unique_ptr <Statement> object, string method,
                           vector <unique_ptr<Statement>> args)
            : object(std::move(object)), method(std::move(method)),
              args(std::move(args)) {}

    ObjectHolder MethodCall::Execute(Closure &closure) {
        vector<ObjectHolder> actual_args;
        for (const auto &arg: args) {
            actual_args.push_back(arg->Execute(closure));
        }

        auto *class_instance =
                object->Execute(closure).TryAs<Runtime::ClassInstance>();
        if (class_instance) {
            if (class_instance->HasMethod(method, actual_args.size())) {
                return class_instance->Call(method, actual_args);
            }
        }

        throw Runtime::RuntimeError("Bad Method Call: " + method);
    }

    ObjectHolder Stringify::Execute(Closure &closure) {
        ostringstream out;
        argument->Execute(closure)->Print(out);
        return Runtime::ObjectHolder::Own(Runtime::String(out.str()));
    }

    optional <ObjectHolder> BinaryOperation::ExecuteBinaryOperationOnClasses(
            Runtime::ClassInstance *lhs, const Runtime::ObjectHolder &rhs,
            const string &operation) {
        if (lhs->HasMethod(operation, 1)) {
            return lhs->Call(operation, {rhs});
        }

        return nullopt;
    }

    ObjectHolder Add::Execute(Closure &closure) {
        ObjectHolder lhs_h = lhs->Execute(closure);
        ObjectHolder rhs_h = rhs->Execute(closure);

        if (const auto lhs_number = lhs_h.TryAs<Runtime::Number>();
                lhs_number && rhs_h.TryAs<Runtime::Number>()) {
            return Runtime::ObjectHolder::Own(Runtime::Number(
                    lhs_number->GetValue() + rhs_h.TryAs<Runtime::Number>()->GetValue()));
        }

        if (const auto lhs_string = lhs_h.TryAs<Runtime::String>();
                lhs_string && rhs_h.TryAs<Runtime::String>()) {
            return Runtime::ObjectHolder::Own(Runtime::String(
                    lhs_string->GetValue() + rhs_h.TryAs<Runtime::String>()->GetValue()));
        }

        if (const auto lhs_instance = lhs_h.TryAs<Runtime::ClassInstance>();
                lhs_instance) {
            const auto result =
                    ExecuteBinaryOperationOnClasses(lhs_instance, rhs_h, "__add__");
            if (result) {
                return *result;
            }
        }

        throw Runtime::RuntimeError("Bad Addition!");
    }

    ObjectHolder Sub::Execute(Closure &closure) {
        ObjectHolder lhs_h = lhs->Execute(closure);
        ObjectHolder rhs_h = rhs->Execute(closure);

        if (auto lhs_number = lhs_h.TryAs<Runtime::Number>();
                lhs_number && rhs_h.TryAs<Runtime::Number>()) {
            return Runtime::ObjectHolder::Own(Runtime::Number(
                    lhs_number->GetValue() - rhs_h.TryAs<Runtime::Number>()->GetValue()));
        }

        if (auto lhs_instance = lhs_h.TryAs<Runtime::ClassInstance>()) {
            const auto result =
                    ExecuteBinaryOperationOnClasses(lhs_instance, rhs_h, "__sub__");
            if (result) {
                return *result;
            }
        }

        throw Runtime::RuntimeError("Bad Subtraction!");
    }

    ObjectHolder Mult::Execute(Runtime::Closure &closure) {
        ObjectHolder lhs_h = lhs->Execute(closure);
        ObjectHolder rhs_h = rhs->Execute(closure);

        if (auto lhs_number = lhs_h.TryAs<Runtime::Number>();
                lhs_number && rhs_h.TryAs<Runtime::Number>()) {
            return Runtime::ObjectHolder::Own(Runtime::Number(
                    lhs_number->GetValue() * rhs_h.TryAs<Runtime::Number>()->GetValue()));
        }

        if (auto lhs_instance = lhs_h.TryAs<Runtime::ClassInstance>()) {
            const auto result =
                    ExecuteBinaryOperationOnClasses(lhs_instance, rhs_h, "__mul__");
            if (result) {
                return *result;
            }
        }

        throw Runtime::RuntimeError("Bad Multiplication!");
    }

    ObjectHolder Div::Execute(Runtime::Closure &closure) {
        ObjectHolder lhs_h = lhs->Execute(closure);
        ObjectHolder rhs_h = rhs->Execute(closure);

        if (auto lhs_number = lhs_h.TryAs<Runtime::Number>();
                lhs_number && rhs_h.TryAs<Runtime::Number>()) {
            if (rhs_h.TryAs<Runtime::Number>()->GetValue() == 0) {
                throw Runtime::RuntimeError("Zero Division!");
            }
            return Runtime::ObjectHolder::Own(Runtime::Number(
                    lhs_number->GetValue() / rhs_h.TryAs<Runtime::Number>()->GetValue()));
        }

        if (auto lhs_instance = lhs_h.TryAs<Runtime::ClassInstance>()) {
            const auto result =
                    ExecuteBinaryOperationOnClasses(lhs_instance, rhs_h, "__div__");
            if (result) {
                return *result;
            }
        }

        throw Runtime::RuntimeError("Bad Division!");
    }

    ObjectHolder Compound::Execute(Closure &closure) {
        for (auto &statement: statements) {
            if (dynamic_cast<Return *>(statement.get())) {
                return statement->Execute(closure);
            }

            if (dynamic_cast<IfElse *>(statement.get()) ||
                dynamic_cast<MethodCall *>(statement.get())) {
                ObjectHolder result = statement->Execute(closure);
                if (result) {
                    return result;
                }
            } else {
                statement->Execute(closure);
            }
        }

        return Runtime::ObjectHolder::None();
    }

    ObjectHolder Return::Execute(Closure &closure) {
        return statement->Execute(closure);
    }

    ClassDefinition::ClassDefinition(ObjectHolder class_)
            : cls(std::move(class_)),
              class_name(cls.TryAs<Runtime::Class>()->GetName()) {}

    ObjectHolder ClassDefinition::Execute(Runtime::Closure &closure) {
        closure[class_name] = cls;
        return Runtime::ObjectHolder::None();
    }

    FieldAssignment::FieldAssignment(VariableValue object, string field_name,
                                     unique_ptr <Statement> rv)
            : object(std::move(object)), field_name(std::move(field_name)),
              right_value(std::move(rv)) {}

    ObjectHolder FieldAssignment::Execute(Runtime::Closure &closure) {
        ObjectHolder &field = object.Execute(closure)
                .TryAs<Runtime::ClassInstance>()
                ->Fields()[field_name];
        field = right_value->Execute(closure);
        return field;
    }

    IfElse::IfElse(unique_ptr <Statement> condition, unique_ptr <Statement> if_body,
                   unique_ptr <Statement> else_body)
            : condition(std::move(condition)), if_body(std::move(if_body)),
              else_body(std::move(else_body)) {}

    ObjectHolder IfElse::Execute(Runtime::Closure &closure) {
        ObjectHolder condition_object = condition->Execute(closure);

        if (condition_object) {
            if (condition_object.Get()->IsTrue()) {
                if (if_body) {
                    return if_body->Execute(closure);
                }
            } else {
                if (else_body) {
                    return else_body->Execute(closure);
                }
            }
        } else {
            if (else_body) {
                return else_body->Execute(closure);
            }
        }

        return ObjectHolder::None();
    }

    ObjectHolder Or::Execute(Runtime::Closure &closure) {
        ObjectHolder lhs_h = lhs->Execute(closure);
        ObjectHolder rhs_h = rhs->Execute(closure);

        if (!lhs_h) {
            lhs_h = ObjectHolder::Own(Runtime::Bool(false));
        }
        if (!rhs_h) {
            rhs_h = ObjectHolder::Own(Runtime::Bool(false));
        }

        return Runtime::ObjectHolder::Own(
                Runtime::Bool(lhs_h->IsTrue() || rhs_h->IsTrue()));
    }

    ObjectHolder And::Execute(Runtime::Closure &closure) {
        ObjectHolder lhs_h = lhs->Execute(closure);
        ObjectHolder rhs_h = rhs->Execute(closure);

        if (!lhs_h) {
            lhs_h = ObjectHolder::Own(Runtime::Bool(false));
        }
        if (!rhs_h) {
            rhs_h = ObjectHolder::Own(Runtime::Bool(false));
        }

        return Runtime::ObjectHolder::Own(
                Runtime::Bool(lhs_h->IsTrue() && rhs_h->IsTrue()));
    }

    ObjectHolder Not::Execute(Runtime::Closure &closure) {
        ObjectHolder argument_h = argument->Execute(closure);

        if (!argument_h) {
            argument_h = ObjectHolder::Own(Runtime::Bool(false));
        }

        return Runtime::ObjectHolder::Own(Runtime::Bool(!argument_h->IsTrue()));
    }

    Comparison::Comparison(Comparator cmp, unique_ptr <Statement> lhs,
                           unique_ptr <Statement> rhs)
            : comparator(std::move(cmp)), left(std::move(lhs)), right(std::move(rhs)) {}

    ObjectHolder Comparison::Execute(Runtime::Closure &closure) {
        return Runtime::ObjectHolder::Own(Runtime::Bool(
                comparator(left->Execute(closure), right->Execute(closure))));
    }

    NewInstance::NewInstance(const Runtime::Class &class_,
                             vector <unique_ptr<Statement>> args)
            : class_(class_), args(std::move(args)) {}

    NewInstance::NewInstance(const Runtime::Class &class_)
            : NewInstance(class_, {}) {}

    ObjectHolder NewInstance::Execute(Runtime::Closure &closure) {
        auto *new_instance = new Runtime::ClassInstance(class_);
        if (new_instance->HasMethod("__init__", args.size())) {
            vector<ObjectHolder> actual_args;
            for (const auto &statement: args) {
                actual_args.push_back(statement->Execute(closure));
            }
            new_instance->Call("__init__", actual_args);
        }

        return ObjectHolder::Share(*new_instance);
    }

} /* namespace Ast */
