#pragma once

#include "contact.pb.h"
#include "iterator_range.h"

#include <iosfwd>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

struct Date {
    int year, month, day;
};

struct Contact {
    std::string name;
    std::optional<Date> birthday;
    std::vector<std::string> phones;
};

class PhoneBook {
public:
    using ContactIterator = std::vector<Contact>::const_iterator;
    using ContactRange = IteratorRange<ContactIterator>;

    explicit PhoneBook(const std::vector<Contact> &contacts);

    [[nodiscard]] IteratorRange<ContactIterator>
    FindByNamePrefix(std::string_view name_prefix) const;

    void SaveTo(std::ostream &output) const;

private:
    std::vector<Contact> contacts_;
};

PhoneBook DeserializePhoneBook(std::istream &input);
