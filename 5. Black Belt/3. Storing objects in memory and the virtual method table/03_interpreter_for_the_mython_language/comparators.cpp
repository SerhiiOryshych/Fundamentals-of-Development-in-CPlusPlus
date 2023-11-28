#include "comparators.h"
#include "object.h"
#include "object_holder.h"

#include <functional>
#include <sstream>

using namespace std;

namespace Runtime {

    bool Equal(ObjectHolder lhs, ObjectHolder rhs) {
        if (auto lhs_class = lhs.TryAs<Class>(); lhs_class && rhs.TryAs<Class>()) {
            return lhs_class->GetName() == rhs.TryAs<Class>()->GetName();
        }

        if (auto lhs_instance = lhs.TryAs<ClassInstance>();
                lhs_instance && rhs.TryAs<ClassInstance>()) {
            if (lhs_instance->class_.GetName() ==
                rhs.TryAs<ClassInstance>()->class_.GetName()) {
                std::ostringstream lhs_stream, rhs_stream;
                lhs->Print(lhs_stream);
                rhs->Print(rhs_stream);
                return lhs_stream.str() == rhs_stream.str();
            }
            return false;
        }

        std::ostringstream lhs_stream, rhs_stream;
        lhs->Print(lhs_stream);
        rhs->Print(rhs_stream);
        return lhs_stream.str() == rhs_stream.str();
    }

    bool Less(ObjectHolder lhs, ObjectHolder rhs) {
        if (auto lhs_number = lhs.TryAs<Number>();
                lhs_number && rhs.TryAs<Number>()) {
            return lhs_number->GetValue() < rhs.TryAs<Number>()->GetValue();
        }

        if (auto lhs_string = lhs.TryAs<String>();
                lhs_string && rhs.TryAs<String>()) {
            return lhs_string->GetValue() < rhs.TryAs<String>()->GetValue();
        }

        throw Runtime::RuntimeError("Bad Comparison!");
    }

} /* namespace Runtime */
