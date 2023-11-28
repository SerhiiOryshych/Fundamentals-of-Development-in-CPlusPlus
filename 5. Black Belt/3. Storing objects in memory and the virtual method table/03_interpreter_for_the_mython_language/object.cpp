#include "object.h"
#include "statement.h"

#include <sstream>
#include <string_view>

using namespace std;

namespace Runtime {

    void ClassInstance::Print(std::ostream &os) {
        const auto *str_method = class_.GetMethod("__str__");
        if (str_method) {
            str_method->body->Execute(fields_)->Print(os);
        } else {
            os << this;
        }
    }

    bool ClassInstance::HasMethod(const std::string &method,
                                  size_t argument_count) const {
        const Method *m = class_.GetMethod(method);
        if (m) {
            return m->formal_params.size() == argument_count;
        }
        return false;
    }

    const Closure &ClassInstance::Fields() const { return fields_; }

    Closure &ClassInstance::Fields() { return fields_; }

    ClassInstance::ClassInstance(const Class &cls) : class_(cls) { UpdateFields(); }

    ObjectHolder ClassInstance::Call(const std::string &method_name,
                                     const std::vector<ObjectHolder> &actual_args) {
        if (HasMethod(method_name, actual_args.size())) {
            Closure method_args;
            const Method *method = class_.GetMethod(method_name);

            auto arg_iter = actual_args.begin();
            for (const auto &arg_name: method->formal_params) {
                method_args[arg_name] = *arg_iter++;
            }

            for (const auto &[field, value]: fields_) {
                method_args[field] = value;
            }

            return method->body->Execute(method_args);
        }

        throw RuntimeError("Bad method_name call: " + method_name);
    }

    void ClassInstance::UpdateFields() {
        fields_["self"] = ObjectHolder::Share(*this);
    }

    Class::Class(std::string name, std::vector<Method> methods, const Class *parent)
            : name_(std::move(name)), parent_(parent),
              methods_(make_unique<MethodMap>()) {
        for (auto &method: methods) {
            (*methods_)[method.name] = std::move(method);
        }
    }

    const Method *Class::GetMethod(const std::string &name) const {
        const auto it = methods_->find(name);
        if (it != methods_->end()) {
            return &it->second;
        }
        return parent_ ? parent_->GetMethod(name) : nullptr;
    }

    void Class::Print(ostream &os) { os << name_; }

    const std::string &Class::GetName() const { return name_; }

    void Bool::Print(std::ostream &os) { os << (value ? "True" : "False"); }

} /* namespace Runtime */
