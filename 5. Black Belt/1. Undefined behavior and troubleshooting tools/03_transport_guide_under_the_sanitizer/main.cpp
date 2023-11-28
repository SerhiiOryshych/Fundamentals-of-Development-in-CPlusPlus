#include "descriptions.h"
#include "json.h"
#include "requests.h"
#include "sphere.h"
#include "transport_catalog.h"
#include "utils.h"

#include <iostream>
#include <sstream>

using namespace std;

int main() {
//    stringstream input("{\n"
//                       "  \"routing_settings\": {\n"
//                       "    \"bus_wait_time\": 6,\n"
//                       "    \"bus_velocity\": 40\n"
//                       "  },\n"
//                       "  \"base_requests\": [\n"
//                       "    {\n"
//                       "      \"type\": \"Bus\",\n"
//                       "      \"name\": \"297\",\n"
//                       "      \"stops\": [\n"
//                       "        \"Biryulyovo Zapadnoye\",\n"
//                       "        \"Biryulyovo Tovarnaya\",\n"
//                       "        \"Universam\",\n"
//                       "        \"Biryulyovo Zapadnoye\"\n"
//                       "      ],\n"
//                       "      \"is_roundtrip\": true\n"
//                       "    },\n"
//                       "    {\n"
//                       "      \"type\": \"Bus\",\n"
//                       "      \"name\": \"635\",\n"
//                       "      \"stops\": [\n"
//                       "        \"Biryulyovo Tovarnaya\",\n"
//                       "        \"Universam\",\n"
//                       "        \"Prazhskaya\"\n"
//                       "      ],\n"
//                       "      \"is_roundtrip\": false\n"
//                       "    },\n"
//                       "    {\n"
//                       "      \"type\": \"Stop\",\n"
//                       "      \"road_distances\": {\n"
//                       "        \"Biryulyovo Tovarnaya\": 2600\n"
//                       "      },\n"
//                       "      \"longitude\": 37.6517,\n"
//                       "      \"name\": \"Biryulyovo Zapadnoye\",\n"
//                       "      \"latitude\": 55.574371\n"
//                       "    },\n"
//                       "    {\n"
//                       "      \"type\": \"Stop\",\n"
//                       "      \"road_distances\": {\n"
//                       "        \"Prazhskaya\": 4650,\n"
//                       "        \"Biryulyovo Tovarnaya\": 1380,\n"
//                       "        \"Biryulyovo Zapadnoye\": 2500\n"
//                       "      },\n"
//                       "      \"longitude\": 37.645687,\n"
//                       "      \"name\": \"Universam\",\n"
//                       "      \"latitude\": 55.587655\n"
//                       "    },\n"
//                       "    {\n"
//                       "      \"type\": \"Stop\",\n"
//                       "      \"road_distances\": {\n"
//                       "        \"Universam\": 890\n"
//                       "      },\n"
//                       "      \"longitude\": 37.653656,\n"
//                       "      \"name\": \"Biryulyovo Tovarnaya\",\n"
//                       "      \"latitude\": 55.592028\n"
//                       "    },\n"
//                       "    {\n"
//                       "      \"type\": \"Stop\",\n"
//                       "      \"road_distances\": {},\n"
//                       "      \"longitude\": 37.603938,\n"
//                       "      \"name\": \"Prazhskaya\",\n"
//                       "      \"latitude\": 55.611717\n"
//                       "    }\n"
//                       "  ],\n"
//                       "  \"stat_requests\": [\n"
//                       "    {\n"
//                       "      \"type\": \"Bus\",\n"
//                       "      \"name\": \"297\",\n"
//                       "      \"id\": 1\n"
//                       "    },\n"
//                       "    {\n"
//                       "      \"type\": \"Bus\",\n"
//                       "      \"name\": \"635\",\n"
//                       "      \"id\": 2\n"
//                       "    },\n"
//                       "    {\n"
//                       "      \"type\": \"Stop\",\n"
//                       "      \"name\": \"Universam\",\n"
//                       "      \"id\": 3\n"
//                       "    },\n"
//                       "    {\n"
//                       "      \"type\": \"Route\",\n"
//                       "      \"from\": \"Biryulyovo Zapadnoye\",\n"
//                       "      \"to\": \"Universam\",\n"
//                       "      \"id\": 4\n"
//                       "    },\n"
//                       "    {\n"
//                       "      \"type\": \"Route\",\n"
//                       "      \"from\": \"Biryulyovo Zapadnoye\",\n"
//                       "      \"to\": \"Prazhskaya\",\n"
//                       "      \"id\": 5\n"
//                       "    }\n"
//                       "  ]\n"
//                       "}");
//    const auto input_doc = Json::Load(input);
    const auto input_doc = Json::Load(cin);
    // const auto input_doc = Json::Load(cin);
    const auto &input_map = input_doc.GetRoot().AsMap();

    const TransportCatalog db(
            Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray()),
            input_map.at("routing_settings").AsMap()
    );

    Json::PrintValue(
            Requests::ProcessAll(db, input_map.at("stat_requests").AsArray()),
            cout
    );
    cout << endl;

    return 0;
}
