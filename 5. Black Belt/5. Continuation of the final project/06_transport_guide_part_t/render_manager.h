#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

#include <map>
#include <memory>
#include <functional>
#include <unordered_map>
#include <unordered_set>

#include "responses.h"
#include "db_item_name_id_map.h"

#include "xml.h"
#include "svg.h"

#include "transport_catalog.pb.h"
#include "transport_render.pb.h"

namespace Data_Structure {
    using MapRespType = std::shared_ptr<MapResponse>;
    using stop_n_companies = std::variant<struct Stop, YellowPages::Company>;
    struct RenderSettings {
        double width;
        double height;
        double outer_margin;
        double padding;
        double stop_radius;
        double line_width;
        double company_radius;
        double company_line_width;

        int stop_label_font_size;
        double stop_label_offset[2]{0};

        int bus_label_font_size;
        double bus_label_offset[2]{0};

        Svg::Color underlayer_color;
        double underlayer_width;

        std::vector<Svg::Color> color_palette;

        std::vector<std::string> layers;
    };

#define NO_IMPL { throw std::logic_error("No impl"); }

    struct ILayersStrategy {
        explicit ILayersStrategy(struct DataBaseSvgBuilder *myself_svg_builder) : db_svg(myself_svg_builder) {}

        virtual void Draw(DbItemIdNameMap &) NO_IMPL;

        virtual void
        DrawPartial(std::vector<std::string> &names_stops,
                    std::vector<std::pair<int, size_t>> &used_buses,
                    Svg::Document &doc,
                    DbItemIdNameMap &) const NO_IMPL;

        virtual void
        DrawPartial(const std::string &nearby_stop,
                    const std::string &company_name,
                    Svg::Document &doc,
                    DbItemIdNameMap &) const NO_IMPL;
    protected:
        struct DataBaseSvgBuilder *db_svg;
    };

    struct BusPolylinesDrawer : public ILayersStrategy {
    public:
        using ILayersStrategy::ILayersStrategy;

        void Draw(DbItemIdNameMap &) override;

        void DrawPartial(std::vector<std::string> &names_stops,
                         std::vector<std::pair<int, size_t>> &used_buses,
                         Svg::Document &doc,
                         DbItemIdNameMap &) const override;
    };

    struct StopsRoundDrawer : public ILayersStrategy {
    public:
        using ILayersStrategy::ILayersStrategy;

        void Draw(DbItemIdNameMap &) override;

        void DrawPartial(std::vector<std::string> &names_stops,
                         std::vector<std::pair<int, size_t>> &used_buses,
                         Svg::Document &doc,
                         DbItemIdNameMap &) const override;

    private:
        void RenderRoundLabel(Svg::Document &doc, const std::string &stop_name) const;
    };

    struct StopsTextDrawer : public ILayersStrategy {
    public:
        using ILayersStrategy::ILayersStrategy;

        void Draw(DbItemIdNameMap &) override;

        void DrawPartial(std::vector<std::string> &names_stops,
                         std::vector<std::pair<int, size_t>> &used_buses,
                         Svg::Document &doc,
                         DbItemIdNameMap &) const override;

    private:
        void RenderStopLabel(Svg::Document &doc, const std::string &stop_name) const;
    };

    struct BusTextDrawer : public ILayersStrategy {
    public:
        using ILayersStrategy::ILayersStrategy;

        void Draw(DbItemIdNameMap &db_item_id_name_map) override;

        void DrawPartial(std::vector<std::string> &names_stops,
                         std::vector<std::pair<int, size_t>> &used_buses,
                         Svg::Document &doc,
                         DbItemIdNameMap &) const override;

    private:
        void RenderBusLabel(Svg::Document &doc,
                            int bus_id,
                            const std::string &stop_name,
                            DbItemIdNameMap &db_item_id_name_map) const;
    };

    struct CompanyRoundDrawer : public ILayersStrategy {
    public:
        using ILayersStrategy::ILayersStrategy;

        void
        DrawPartial(const std::string &nearby_stop,
                    const std::string &company_name,
                    Svg::Document &doc,
                    DbItemIdNameMap &) const override;
    };

    struct CompanyPolylinesDrawer : public ILayersStrategy {
    public:
        using ILayersStrategy::ILayersStrategy;

        void
        DrawPartial(const std::string &nearby_stop,
                    const std::string &company_name,
                    Svg::Document &doc,
                    DbItemIdNameMap &) const override;
    };

    struct CompanyTextDrawer : public ILayersStrategy {
    public:
        using ILayersStrategy::ILayersStrategy;

        void
        DrawPartial(const std::string &nearby_stop,
                    const std::string &company_name,
                    Svg::Document &doc,
                    DbItemIdNameMap &) const override;
    };

    struct DataBaseSvgBuilder {
    public:
        explicit DataBaseSvgBuilder(RenderSettings render_set);

        explicit DataBaseSvgBuilder(RenderProto::RenderSettings const &,
                                    const std::unordered_map<int, struct Stop> &,
                                    const std::unordered_map<int, struct Bus> &,
                                    const std::unordered_map<std::string, const YellowPages::Company *> &companies,
                                    DbItemIdNameMap &);

        [[nodiscard]] MapRespType RenderMap() const;

        [[nodiscard]] MapRespType RenderRoute(std::vector<RouteResponse::ItemPtr> const &,
                                              DbItemIdNameMap &dbItemIdNameMap);

        [[nodiscard]] MapRespType
        RenderPathToCompany(std::vector<RouteResponse::ItemPtr> const &items,
                            std::string const &company,
                            DbItemIdNameMap &db_item_id_name_map);

        void Serialize(TCProto::TransportCatalog &tc) const;

        static bool IsConnected(std::string const &lhs, std::string const &rhs,
                                std::unordered_map<std::string, std::unordered_set<std::string>> const &db_s);

        friend BusPolylinesDrawer;
        friend StopsRoundDrawer;
        friend StopsTextDrawer;
        friend BusTextDrawer;
        friend CompanyRoundDrawer;
        friend CompanyPolylinesDrawer;
        friend CompanyTextDrawer;
    private:
        void Init(const std::unordered_map<int, Bus> &,
                  DbItemIdNameMap &);

        void CalculateCoordinates(const std::unordered_map<std::string, stop_n_companies> &,
                                  const std::unordered_map<int, Bus> &,
                                  DbItemIdNameMap &db_item_id_name_map);

        std::map<std::string, Svg::Point>
        CoordinateUniformDistribution(const std::unordered_map<std::string, stop_n_companies> &,
                                      const std::unordered_map<int, Bus> &,
                                      DbItemIdNameMap &);

        void BuildNeighborhoodConnections(const std::unordered_map<int, Bus> &buses,
                                          DbItemIdNameMap &);

        void BuildNeighborhoodCompaniesWithStops(const std::unordered_map<std::string, stop_n_companies> &points,
                                                 DbItemIdNameMap &);

        static auto SortingByCoordinates(std::map<std::string, Svg::Point> const &uniform,
                                         const std::unordered_map<std::string, stop_n_companies> &);

        std::pair<std::map<std::string, int>, int>
        GluingCoordinates(std::vector<std::pair<double, std::string>> const &sorted_by_coord);

        std::pair<std::map<std::string, Svg::Point>, std::map<std::string, Svg::Point>>
        CoordinateCompression(const std::unordered_map<std::string, stop_n_companies> &,
                              const std::unordered_map<int, Bus> &,
                              DbItemIdNameMap &db_item_id_name_map);

        RenderSettings renderSettings;
        Svg::Document doc;

        std::map<std::string, std::pair<Bus, Svg::Color>> bus_dict;
        std::map<std::string, Svg::Point> stops_coordinates;
        std::map<std::string, Svg::Point> company_coordinates;
        std::map<std::string, std::shared_ptr<ILayersStrategy>> layersStrategy;
        std::map<std::string, std::shared_ptr<ILayersStrategy>> CompanyRouteLayersStrategy;

        std::map<std::string, std::unordered_set<std::string>> db_connected;

        void Deserialize(const RenderProto::RenderSettings &);

        Svg::Document GenerateRouteSVG(std::vector<RouteResponse::ItemPtr> const &);
    };
}

#endif //RENDER_MANAGER_H
