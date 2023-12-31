#include <iostream>
#include <fstream>
#include <string_view>

#include "requests.h"
#include "responses.h"
#include "data_manager.h"

using namespace std;

int main(int argc, const char *argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (argc != 2) {
        cerr << "Usage: transport_catalog_part_o [make_base|process_requests]\n";
        return 5;
    }

    const string_view mode(argv[1]);

    if (mode == "make_base") {
        const auto doc = Json::Load(std::cin);
        const auto &input_map = doc.GetRoot();

        DbItemIdNameMap db_item_id_name_map;
        const DS::DataBase db{
                ReadBaseRequests(input_map["base_requests"], db_item_id_name_map),
                ReadYellowPagesData(input_map["yellow_pages"], db_item_id_name_map),
                ReadRoutingSettings(input_map["routing_settings"]),
                ReadRenderSettings(input_map["render_settings"]),
                std::move(db_item_id_name_map)
        };

        ofstream out(input_map["serialization_settings"]["file"].AsString(), std::ios::out | std::ios::binary);
        if (!out.is_open())
            throw std::logic_error("can't open file " + input_map["serialization_settings"]["file"].AsString());
        db.Serialize(out);
        out.close();

    } else if (mode == "process_requests") {
        const auto doc = Json::Load(std::cin);
        const auto &input_map = doc.GetRoot();

        ifstream inp(input_map["serialization_settings"]["file"].AsString(),
                     std::ios::in | std::ios::binary);
        if (!inp.is_open())
            throw std::logic_error("can't open file " + input_map["serialization_settings"]["file"].AsString());
        const DS::DataBase db(inp);
        inp.close();

        PrintResponses(
                ReadStatRequests(db, input_map["stat_requests"], *db.db_item_id_name_map),
                std::cout
        );
        std::cout << std::endl;
    }

    return 0;
}
