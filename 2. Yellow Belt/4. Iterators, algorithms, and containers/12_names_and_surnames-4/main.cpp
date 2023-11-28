#include <map>
#include <iostream>

using namespace std;

class Person {
public:
    void ChangeFirstName(int year, const string &first_name) {
        change_first_name_log[-year] = first_name;
    }

    void ChangeLastName(int year, const string &last_name) {
        change_last_name_log[-year] = last_name;
    }

    string GetFullName(int year) {
        year *= -1;

        auto firstNameIt = change_first_name_log.lower_bound(year);
        auto firstName = firstNameIt != change_first_name_log.end() ? firstNameIt->second : "";

        auto lastNameIt = change_last_name_log.lower_bound(year);
        auto lastName = lastNameIt != change_last_name_log.end() ? lastNameIt->second : "";

        if (!firstName.empty() && !lastName.empty()) {
            return firstName + " " + lastName;
        }

        if (!firstName.empty()) {
            return firstName + " with unknown last name";
        }

        if (!lastName.empty()) {
            return lastName + " with unknown first name";
        }

        return "Incognito";
    }

private:
    map<int, string> change_first_name_log, change_last_name_log;
};

int main() {
    Person person;

    person.ChangeFirstName(1965, "Polina");
    person.ChangeLastName(1967, "Sergeeva");
    for (int year: {1900, 1965, 1990}) {
        cout << person.GetFullName(year) << endl;
    }

    person.ChangeFirstName(1970, "Appolinaria");
    for (int year: {1969, 1970}) {
        cout << person.GetFullName(year) << endl;
    }

    person.ChangeLastName(1968, "Volkova");
    for (int year: {1969, 1970}) {
        cout << person.GetFullName(year) << endl;
    }

    return 0;
}
