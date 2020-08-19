#pragma once

#include "descriptions.h"
#include "json.h"
#include "transport_router.h"
#include "utils.h"
#include "svg.h"

#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <memory>


namespace Responses {
    struct Stop {
        std::set<std::string> bus_names;
    };

    struct Bus {
        size_t stop_count = 0;
        size_t unique_stop_count = 0;
        int road_route_length = 0;
        double geo_route_length = 0.0;
    };
}

struct SvgParams {
    SvgParams() = default;
    SvgParams(const Json::Dict& settings);

    double width;
    double  height;
    double padding;
    double stop_radius;
    double line_width;
    int stop_label_font_size;
    Svg::Point stop_label_offset;
    Svg::Color underlayer_color;
    double underlayer_width;
    std::vector<Svg::Color> color_palette;
};

class TransportCatalog {
private:
    using Bus = Responses::Bus;
    using Stop = Responses::Stop;

public:
    TransportCatalog(std::vector<Descriptions::InputQuery>& data,
        const Json::Dict& routing_settings_json,
        const Json::Dict& render_settings_json);

    const Stop* GetStop(const std::string& name) const;
    const Bus* GetBus(const std::string& name) const;

    std::optional<TransportRouter::RouteInfo> FindRoute(const std::string& stop_from, const std::string& stop_to) const;

    std::string RenderMap() const;

private:
    static int ComputeRoadRouteLength(
        const std::vector<std::string>& stops,
        const Descriptions::StopsDict& stops_dict
    );

    static double ComputeGeoRouteDistance(
        const std::vector<std::string>& stops,
        const Descriptions::StopsDict& stops_dict
    );

    std::unordered_map<std::string, Stop> stops_;
    std::unordered_map<std::string, Bus> buses_;
    std::unique_ptr<TransportRouter> router_;
    SvgParams svg_params;
    
    Descriptions::BusesDict buses_dict;
    Descriptions::StopsDict stops_dict;
};

class SvgBuilder {
public:
    SvgBuilder(const SvgParams& svg_params, const Descriptions::StopsDict& stops);

    void AddBuses(const Descriptions::BusesDict& buses_dict,
        const Descriptions::StopsDict& stops_dict);
    void AddStops(const Descriptions::StopsDict& stops_dict);
    void AddNames(const Descriptions::StopsDict& stops_dict);
    
    std::string GetResult() const;

private:
    Svg::Point GetPoint(Sphere::Point pt) {
        double x = (pt.longitude - min_lon) * zoom_coef + svg_params.padding;
        double y = (max_lat - pt.latitude) * zoom_coef + svg_params.padding;

        return { x, y };
    }

    const SvgParams& svg_params;
    Svg::Document document;

    double min_lat = 90, max_lat = -90,
        min_lon = 180, max_lon = -180;
    double zoom_coef;
};
