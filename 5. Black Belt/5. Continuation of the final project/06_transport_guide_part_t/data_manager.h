#ifndef DATA_STRUCTURE_H
#define DATA_STRUCTURE_H

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <string>
#include <variant>

#include <unordered_map>

#include "route_manager.h"
#include "render_manager.h"
#include "yellow_pages_manager.h"
#include "db_item_name_id_map.h"

#include "distance.h"
#include "responses.h"

namespace Data_Structure {
    struct Bus {
        Bus() = delete;

        Bus(int bus_id_new,
            std::vector<int> stops_new,
            bool is_roundtrip_new) : bus_id(bus_id_new),
                                     stops(std::move(stops_new)),
                                     is_roundtrip(is_roundtrip_new) {}

        int bus_id;
        std::vector<int> stops;
        bool is_roundtrip;
    };

    struct Stop {
        Stop() = delete;

        Stop(int stop_id_new,
             Distance dist_new,
             std::map<int, int> adjacent_stops_new) : stop_id(stop_id_new),
                                                      dist(dist_new),
                                                      adjacent_stops(std::move(adjacent_stops_new)) {}

        int stop_id;
        Distance dist;
        std::map<int, int> adjacent_stops;
    };

    struct Datetime {
        size_t day;
        size_t hours;
        size_t minutes;
        double part_of_minute;
    };

    double ToMinute(Datetime const &);

    Datetime ToDatetime(double minutes, size_t day);

    int ComputeStopsDistance(const Stop &lhs,
                             const Stop &rhs,
                             DbItemIdNameMap &);

    int ComputeRouteDistance(std::vector<int> const &stops,
                             const std::unordered_map<int, Stop> &,
                             DbItemIdNameMap &);

    double ComputeTimeToWalking(double meters, double walk_speed);

    double ComputeGeoDistance(std::vector<int> const &stops,
                              const std::unordered_map<int, Stop> &,
                              DbItemIdNameMap &);

    std::set<std::string> GetBearingPoints(const std::unordered_map<std::string, stop_n_companies> &,
                                           const std::unordered_map<int, Bus> &,
                                           DbItemIdNameMap &);

    using DBItem = std::variant<Stop, Bus>;
    using StopRespType = std::shared_ptr<StopResponse>;
    using BusRespType = std::shared_ptr<BusResponse>;
    using MapRespType = std::shared_ptr<MapResponse>;

    using stop_n_companies = std::variant<Stop, YellowPages::Company>;

    struct DataBase {
    private:
        std::unordered_map<int, Bus> pure_buses;
        std::unordered_map<int, Stop> pure_stops;

        std::unordered_map<int, StopRespType> stops;
        std::unordered_map<int, BusRespType> buses;

        std::unique_ptr<DataBaseRouter> router;
        std::unique_ptr<DataBaseSvgBuilder> svg_builder;
        std::unique_ptr<DataBaseYellowPages> yellow_pages_db;

        TimeDatabase time_database;
    public:
        std::unique_ptr<DbItemIdNameMap> db_item_id_name_map;

        explicit DataBase(std::istream &is);

        [[deprecated]] DataBase(const std::vector<DBItem> &,
                                RoutingSettings routing_settings_,
                                DbItemIdNameMap &);

        DataBase(const std::vector<DBItem> &,
                 RoutingSettings routing_settings_,
                 RenderSettings render_settings,
                 DbItemIdNameMap &db_item_id_name_map);

        DataBase(const std::vector<DBItem> &,
                 YellowPages::Database,
                 RoutingSettings routing_settings_,
                 RenderSettings render_settings,
                 DbItemIdNameMap &&db_item_id_name_map_new);

        [[nodiscard]] ResponseType FindBus(const std::string &title) const;

        [[nodiscard]] ResponseType FindStop(const std::string &title) const;

        [[nodiscard]] ResponseType FindRoute(const std::string &from, const std::string &to) const;

        [[nodiscard]] ResponseType BuildMap() const;

        [[nodiscard]] ResponseType FindCompanies(const std::vector<std::shared_ptr<Query>> &queries) const;

        [[nodiscard]] ResponseType FindRouteToCompanies(const std::string &from, const Datetime &cur_time,
                                                        const std::vector<std::shared_ptr<Query>> &queries) const;

        RoutingSettings GetSettings() const {
            return router->GetSettings();
        }

        void Serialize(std::ostream &os) const;

    private:
        void Deserialize(std::istream &is);

        static ResponseType GenerateBad();

        void Init(std::vector<DBItem> const &, DbItemIdNameMap &);

        void ExtraTime(YellowPages::Company const &, Datetime const &,
                       double *min_time, double cur_time, double &result) const;
    };
}

#endif //DATA_STRUCTURE_H
