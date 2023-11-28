#include <iostream>
#include <deque>
#include <map>
#include <set>

using namespace std;

struct Event {
    long long time;
    string hotel_name;
    int client_id, room_count;
};

class Manager {
public:
    void Book(long long time, const string &hotel_name, int client_id, int room_count) {
        RemoveEvents(time);
        events_.push_back({time, hotel_name, client_id, room_count});

        const auto it = hotel_clients_[hotel_name].lower_bound({client_id, 0});
        int client_cnt = 0;
        if (it != hotel_clients_[hotel_name].end() && it->first == client_id) {
            client_cnt = it->second;
            hotel_clients_[hotel_name].erase(it);
        }
        hotel_clients_[hotel_name].insert({client_id, client_cnt + 1});

        hotel_rooms_[hotel_name] += room_count;
    }

    [[nodiscard]] int Clients(const string &hotel_name) const {
        if (hotel_clients_.count(hotel_name) == 0) {
            return 0;
        }
        return hotel_clients_.at(hotel_name).size();
    }

    [[nodiscard]] int Rooms(const string &hotel_name) const {
        if (hotel_rooms_.count(hotel_name) == 0) {
            return 0;
        }
        return hotel_rooms_.at(hotel_name);
    }

private:
    deque<Event> events_;
    map<string, set<pair<int, int>>> hotel_clients_;
    map<string, int> hotel_rooms_;

    void RemoveEvents(long long time) {
        while (!events_.empty() && events_.front().time + 86400 <= time) {
            const auto event = events_.front();
            auto client_cnt = hotel_clients_[event.hotel_name].lower_bound({event.client_id, 0})->second;
            hotel_clients_[event.hotel_name].erase({event.client_id, client_cnt});
            if (client_cnt > 1) {
                hotel_clients_[event.hotel_name].insert({event.client_id, client_cnt - 1});
            }
            hotel_rooms_[event.hotel_name] -= event.room_count;
            events_.pop_front();
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Manager manager;

    int q;
    cin >> q;
    while (q--) {
        string type;
        cin >> type;
        if (type == "BOOK") {
            long long time;
            string hotel_name;
            int client_id, room_count;
            cin >> time >> hotel_name >> client_id >> room_count;
            manager.Book(time, hotel_name, client_id, room_count);
        } else if (type == "CLIENTS") {
            string hotel_name;
            cin >> hotel_name;
            cout << manager.Clients(hotel_name) << endl;
        } else {
            string hotel_name;
            cin >> hotel_name;
            cout << manager.Rooms(hotel_name) << endl;
        }
    }
}
