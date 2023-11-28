#include <iostream>
#include <vector>
#include <string>
#include <map>

using namespace std;

struct Person {
    string name;
    int age, income;
    bool is_male;
};

vector<Person> ReadPeople(istream &input) {
    int count;
    input >> count;

    vector<Person> result(count);
    for (Person &p: result) {
        char gender;
        input >> p.name >> p.age >> p.income >> gender;
        p.is_male = gender == 'M';
    }

    return result;
}

struct Stats {
    Stats() {
        const vector<Person> people = ReadPeople(cin);

        people_sorted_by_age = [&people] {
            auto sorted_by_age = people;
            sort(begin(sorted_by_age), end(sorted_by_age), [](const Person &lhs, const Person &rhs) {
                return lhs.age < rhs.age;
            });
            return sorted_by_age;
        }();

        total_income = [&people] {
            auto sorted_by_income = people;
            sort(begin(sorted_by_income), end(sorted_by_income), [](const Person &lhs, const Person &rhs) {
                return lhs.income > rhs.income;
            });
            vector<long long> result(sorted_by_income.size() + 1);
            for (int i = 0; i < sorted_by_income.size(); i++) {
                result[i + 1] = result[i] + sorted_by_income[i].income;
            }
            return result;
        }();

        male_most_popular_name = [&people] {
            map<string, int> result;
            for (const auto &p: people) {
                if (p.is_male) {
                    result[p.name]++;
                }
            }

            string name;
            int count = 0;
            for (const auto &item: result) {
                if (item.second > count) {
                    name = item.first;
                    count = item.second;
                }
            }
            return name;
        }();

        female_most_popular_name = [&people] {
            map<string, int> result;
            for (const auto &p: people) {
                if (!p.is_male) {
                    result[p.name]++;
                }
            }

            string name;
            int count = 0;
            for (const auto &item: result) {
                if (item.second > count) {
                    name = item.first;
                    count = item.second;
                }
            }
            return name;
        }();
    }

    vector<Person> people_sorted_by_age;
    vector<long long> total_income;
    string male_most_popular_name;
    string female_most_popular_name;
};

void AgeCommand(const Stats &stats) {
    int adult_age;
    cin >> adult_age;

    auto adult_begin = lower_bound(
            begin(stats.people_sorted_by_age), end(stats.people_sorted_by_age), adult_age,
            [](const Person &lhs, int age) {
                return lhs.age < age;
            }
    );

    cout << "There are " << std::distance(adult_begin, end(stats.people_sorted_by_age))
         << " adult people for maturity age " << adult_age << endl;
}

void PopularNameCommand(const Stats &stats) {
    char gender;
    cin >> gender;

    const string most_popular_name = gender == 'M' ? stats.male_most_popular_name : stats.female_most_popular_name;

    if (most_popular_name.empty()) {
        cout << "No people of gender " << gender << endl;
    } else {
        cout << "Most popular name among people of gender " << gender << " is " << most_popular_name << endl;
    }
}

void WealthyCommand(const Stats &stats) {
    int count;
    cin >> count;
    cout << "Top-" << count << " people have total income " << stats.total_income[count] << endl;
}

int main() {
    const Stats stats;

    for (string command; cin >> command;) {
        if (command == "AGE") {
            AgeCommand(stats);
        } else if (command == "WEALTHY") {
            WealthyCommand(stats);
        } else if (command == "POPULAR_NAME") {
            PopularNameCommand(stats);
        }
    }
}
