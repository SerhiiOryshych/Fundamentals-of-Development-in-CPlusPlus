#include "phone_book.h"
#include <algorithm>

bool operator<(const Contact &a, const Contact &b) { return a.name < b.name; }

PhoneBook::PhoneBook(const std::vector<Contact> &contacts)
        : contacts_(contacts) {
    std::sort(contacts_.begin(), contacts_.end());
}

PhoneBook::ContactRange
PhoneBook::FindByNamePrefix(std::string_view name_prefix) const {
    if (!name_prefix.empty()) {
        const Contact finder1{std::string(name_prefix)};

        std::string tmp = std::string(name_prefix);
        tmp[tmp.size() - 1] = char(tmp[tmp.size() - 1] + 1);
        const Contact finder2{tmp};

        const auto lower1 =
                std::lower_bound(contacts_.begin(), contacts_.end(), finder1);
        const auto lower2 =
                std::lower_bound(contacts_.begin(), contacts_.end(), finder2);

        return {lower1, lower2};
    } else {
        return {contacts_.begin(), contacts_.end()};
    }
}

void PhoneBook::SaveTo(std::ostream &output) const {
    PhoneBookSerialize::ContactList list;
    for (const auto &contact: contacts_) {
        PhoneBookSerialize::Contact *serialize_contact = list.add_contact();
        *serialize_contact->mutable_name() = contact.name;

        if (contact.birthday) {
            serialize_contact->mutable_birthday()->set_day(contact.birthday->day);
            serialize_contact->mutable_birthday()->set_month(contact.birthday->month);
            serialize_contact->mutable_birthday()->set_year(contact.birthday->year);
        }

        for (const auto &phone: contact.phones) {
            serialize_contact->add_phone_number(phone);
        }
    }

    list.SerializePartialToOstream(&output);
}

PhoneBook DeserializePhoneBook(std::istream &input) {
    std::vector<Contact> contacts;
    PhoneBookSerialize::ContactList list;
    if (list.ParseFromIstream(&input)) {
        contacts.reserve(list.contact_size());

        for (const auto &serialized_contact: *list.mutable_contact()) {
            Contact contact;
            contact.name = serialized_contact.name();

            if (serialized_contact.has_birthday()) {
                contact.birthday = {serialized_contact.birthday().year(),
                                    serialized_contact.birthday().month(),
                                    serialized_contact.birthday().day()};
            }

            for (const auto &serialized_phone: serialized_contact.phone_number()) {
                contact.phones.push_back(serialized_phone);
            }

            contacts.push_back(contact);
        }
    }

    return PhoneBook{contacts};
}
