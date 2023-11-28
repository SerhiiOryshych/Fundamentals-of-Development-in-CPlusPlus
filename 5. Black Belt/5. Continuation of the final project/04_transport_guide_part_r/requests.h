#pragma once

#include "companies_index.h"
#include "json.h"
#include "transport_catalog.h"

#include <string>
#include <variant>

namespace Requests {
    struct Stop {
        std::string name;

        [[nodiscard]] Json::Dict Process(const TransportCatalog &db) const;
    };

    struct Bus {
        std::string name;

        [[nodiscard]] Json::Dict Process(const TransportCatalog &db) const;
    };

    struct Route {
        std::string stop_from;
        std::string stop_to;

        [[nodiscard]] Json::Dict Process(const TransportCatalog &db) const;
    };

    struct Map {
        static Json::Dict Process(const TransportCatalog &db);
    };

    struct FindCompanies {
        YellowPages::CompanyQuery query;

        [[nodiscard]] Json::Dict Process(const TransportCatalog &db) const;
    };

    std::variant<Stop, Bus, Route, Map, FindCompanies> Read(const Json::Dict &attrs);

    Json::Array ProcessAll(const TransportCatalog &db, const Json::Array &requests);
}
