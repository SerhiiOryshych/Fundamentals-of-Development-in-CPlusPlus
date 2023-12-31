#ifndef ROUTE_STRUCTURE_H
#define ROUTE_STRUCTURE_H

#include <unordered_map>
#include <utility>

#include "graph.h"
#include "router.h"
#include "responses.h"
#include "db_item_name_id_map.h"

#include "transport_catalog.pb.h"
#include "transport_router.pb.h"

namespace Data_Structure {
    using RouteRespType = std::shared_ptr<RouteResponse>;
    struct RoutingSettings {
        double bus_wait_time;
        int bus_velocity;
        double pedestrian_velocity;
    };

    struct DataBaseRouter {
    private:
        const struct RoutingSettings routing_settings;
        Graph::DirectedWeightedGraph<double> graph_map;
        std::shared_ptr<Graph::Router<double>> router;

        std::unordered_map<Graph::EdgeID, RouteResponse::ItemPtr> edge_by_bus;
        struct vertices_path {
            Graph::VertexID inp;
            Graph::VertexID out;
        };
        std::unordered_map<int, vertices_path> waiting_stops;
    public:
        struct proxy_route {
        private:
            std::shared_ptr<Graph::Router<double>> main_router;
            std::optional<Graph::Router<double>::RouteInfo> rf;
        public:
            proxy_route(std::shared_ptr<Graph::Router<double>> router,
                        std::optional<Graph::Router<double>::RouteInfo> rinf) : main_router(std::move(router)),
                                                                                rf(rinf) {}

            proxy_route(const proxy_route &) = delete;

            proxy_route &operator=(const proxy_route &) = delete;

            proxy_route(proxy_route &&) = default;

            proxy_route &operator=(proxy_route &&) = default;

            [[nodiscard]] auto GetRoute() const;

            [[nodiscard]] std::optional<Graph::Router<double>::RouteInfo> const &GetInfo() const;

            [[nodiscard]] bool IsValid() const;

            ~proxy_route();
        };

        DataBaseRouter(const std::unordered_map<int, struct Stop> &,
                       const std::unordered_map<int, struct Bus> &,
                       RoutingSettings routing_settings_,
                       DbItemIdNameMap &);

        explicit DataBaseRouter(RouterProto::Router const &router_mes, DbItemIdNameMap &db_item_id_name_map);

        RouteRespType CreateRoute(std::string const &from,
                                  std::string const &to,
                                  DbItemIdNameMap &);

        std::optional<double> GetRouteWeight(std::string const &from,
                                             std::string const &to,
                                             DbItemIdNameMap &);

        RoutingSettings GetSettings() const;

        void Serialize(TCProto::TransportCatalog &, DbItemIdNameMap &dbItemIdNameMap) const;

    private:
        void FillGraphWithStops(const std::unordered_map<int, Stop> &,
                                DbItemIdNameMap &);

        void
        FillGraphWithBuses(const std::unordered_map<int, Bus> &,
                           const std::unordered_map<int, Stop> &,
                           DbItemIdNameMap &);
    };
}

#endif //ROUTE_STRUCTURE_H
