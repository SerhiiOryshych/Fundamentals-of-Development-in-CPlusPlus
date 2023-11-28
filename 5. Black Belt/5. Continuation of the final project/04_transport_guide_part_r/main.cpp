#include "descriptions.h"
#include "json.h"
#include "requests.h"
#include "transport_catalog.h"
#include "database.pb.h"

#include <google/protobuf/util/json_util.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string_view>

using namespace std;

string ReadFileData(const string &file_name) {
    ifstream file(file_name, ios::binary | ios::ate);
    const ifstream::pos_type end_pos = file.tellg();
    file.seekg(0, ios::beg);

    string data(end_pos, '\0');
    file.read(&data[0], end_pos);
    return data;
}

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: transport_catalog_part_r [make_base|process_requests]\n";
        return 5;
    }

    const string_view mode(argv[1]);

    const auto input_doc = Json::Load(cin);
    const auto &input_map = input_doc.GetRoot().AsMap();

    if (mode == "process_requests") {

        const string &file_name = input_map.at("serialization_settings").AsMap().at("file").AsString();
        const auto db = TransportCatalog::Deserialize(ReadFileData(file_name));

        Json::PrintValue(
                Requests::ProcessAll(db, input_map.at("stat_requests").AsArray()),
                cout
        );
        cout << endl;

    } else if (mode == "make_base") {

        std::ostringstream yellow_pages_stream;
        // ugly, but fast to code
        Json::PrintValue(input_map.at("yellow_pages").AsMap(), yellow_pages_stream);
        YellowPages::Database yellow_pages_db;
        google::protobuf::util::JsonStringToMessage(yellow_pages_stream.str(), &yellow_pages_db);

        const TransportCatalog db(
                Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray()),
                std::move(yellow_pages_db),
                input_map.at("routing_settings").AsMap(),
                input_map.at("render_settings").AsMap()
        );

        const string &file_name = input_map.at("serialization_settings").AsMap().at("file").AsString();
        ofstream file(file_name);
        file << db.Serialize();

    }

    return 0;
}
