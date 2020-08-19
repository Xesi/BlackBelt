#include "transport_catalog.h"
#include "svg.h"
#include "descriptions.h"
#include "json.h"

#include <sstream>
#include <memory>
#include <algorithm>

#include <fstream> // DELETE
#include <locale>

using namespace std;

TransportCatalog::TransportCatalog(vector<Descriptions::InputQuery>& data,
    const Json::Dict& routing_settings_json,
    const Json::Dict& render_settings_json) 
    : svg_params(render_settings_json) {
    auto stops_end = partition(begin(data), end(data), [](const auto& item) {
        return holds_alternative<Descriptions::Stop>(item);
    });

    //Descriptions::StopsDict stops_dict;
    for (const auto& item : Range{ begin(data), stops_end }) {
        const auto& stop = get<Descriptions::Stop>(item);
        stops_dict[stop.name] = &stop;
        stops_.insert({ stop.name, {} });
    }

    //Descriptions::BusesDict buses_dict;
    for (const auto& item : Range{ stops_end, end(data) }) {
        const auto& bus = get<Descriptions::Bus>(item);

        buses_dict[bus.name] = &bus;
         buses_[bus.name] = Bus{
          bus.stops.size(),
          ComputeUniqueItemsCount(AsRange(bus.stops)),
          ComputeRoadRouteLength(bus.stops, stops_dict),
          ComputeGeoRouteDistance(bus.stops, stops_dict)
        };

        for (const string& stop_name : bus.stops) {
            stops_.at(stop_name).bus_names.insert(bus.name);
        }
    }

    router_ = make_unique<TransportRouter>(stops_dict, buses_dict, routing_settings_json);
}

const TransportCatalog::Stop* TransportCatalog::GetStop(const string& name) const {
    return GetValuePointer(stops_, name);
}

const TransportCatalog::Bus* TransportCatalog::GetBus(const string& name) const {
    return GetValuePointer(buses_, name);
}

optional<TransportRouter::RouteInfo> TransportCatalog::FindRoute(const string& stop_from, const string& stop_to) const {
    return router_->FindRoute(stop_from, stop_to);
}

int TransportCatalog::ComputeRoadRouteLength(
    const vector<string>& stops,
    const Descriptions::StopsDict& stops_dict
) {
    int result = 0;
    for (size_t i = 1; i < stops.size(); ++i) {
        result += Descriptions::ComputeStopsDistance(*stops_dict.at(stops[i - 1]), *stops_dict.at(stops[i]));
    }
    return result;
}

double TransportCatalog::ComputeGeoRouteDistance(
    const vector<string>& stops,
    const Descriptions::StopsDict& stops_dict
) {
    double result = 0;
    for (size_t i = 1; i < stops.size(); ++i) {
        result += Sphere::Distance(
            stops_dict.at(stops[i - 1])->position, stops_dict.at(stops[i])->position
        );
    }
    return result;
}

Svg::Color GetColor(const Json::Node& node) {
    if (std::holds_alternative<std::string>(node.GetBase())) {
        return Svg::Color(node.AsString());
    }
    else {
        auto& vec = node.AsArray();
        uint8_t a = vec[0].AsInt();
        uint8_t b = vec[1].AsInt();
        uint8_t c = vec[2].AsInt();
        if (vec.size() == 3) return Svg::Color(Svg::Rgb{ a,b,c });
        else {
            double alpha = vec[3].AsDouble();
            return Svg::Color(Svg::Rgba{ a,b,c,alpha });
        }
    }
}

SvgParams::SvgParams(const Json::Dict& settings) {
    width = settings.at("width").AsDouble();
    height = settings.at("height").AsDouble();
    padding = settings.at("padding").AsDouble();
    stop_radius = settings.at("stop_radius").AsDouble();
    line_width = settings.at("line_width").AsDouble();
    stop_label_font_size = settings.at("stop_label_font_size").AsInt();
    const auto& vec_with_pt = settings.at("stop_label_offset").AsArray();
    stop_label_offset = Svg::Point{ vec_with_pt[0].AsDouble(), vec_with_pt[1].AsDouble() };
    underlayer_color = GetColor(settings.at("underlayer_color"));
    underlayer_width = settings.at("underlayer_width").AsDouble();
    auto vec = settings.at("color_palette").AsArray();
    for (size_t ind = 0; ind < vec.size(); ++ind) {
        color_palette.push_back(GetColor(vec[ind]));
    }
    
    bus_label_font_size = settings.at("bus_label_font_size").AsInt();
    
    const auto& vec_with_pt2 = settings.at("bus_label_offset").AsArray();
    bus_label_offset = Svg::Point{ vec_with_pt2[0].AsDouble(), vec_with_pt2[1].AsDouble() };
}

SvgBuilder::SvgBuilder(const SvgParams& svg_params, const Descriptions::StopsDict& stops)
: svg_params(svg_params) {
    for (const auto& [name, stop] : stops) {
        min_lat = std::min(min_lat, stop->position.latitude);
        max_lat = std::max(max_lat, stop->position.latitude);
        min_lon = std::min(min_lon, stop->position.longitude);
        max_lon = std::max(max_lon, stop->position.longitude);
    }
    double inf = 2e18;

    double width_zoom_coef = ((max_lon == min_lon) ? inf :
        (svg_params.width - 2 * svg_params.padding) / (max_lon - min_lon));

    double height_zoom_coef = ((max_lat == min_lat) ? inf :
        (svg_params.height - 2 * svg_params.padding) / (max_lat - min_lat));

    zoom_coef = std::min(width_zoom_coef, height_zoom_coef);
    if (zoom_coef == inf) zoom_coef = 0;
}

void SvgBuilder::AddBuses(const Descriptions::BusesDict& buses_dict,
    const Descriptions::StopsDict& stops_dict) {

    size_t count_colors = svg_params.color_palette.size();
    size_t color_id = 0;
    for (const auto& [name, bus] : buses_dict) {
        Svg::Polyline route;
        route.SetStrokeColor(svg_params.color_palette[color_id++]);
        if (color_id == count_colors) color_id = 0;

        route.SetStrokeWidth(svg_params.line_width);
        route.SetStrokeLineCap("round");
        route.SetStrokeLineJoin("round");
        for (const auto& stop : bus->stops) {
            Sphere::Point pt = stops_dict.at(stop)->position;
            route.AddPoint(GetPoint({pt.latitude, pt.longitude}));
        }
        document.Add(route);
    }
}
void SvgBuilder::AddStops(const Descriptions::StopsDict& stops_dict) {
    for (const auto& stop : stops_dict) {
        Svg::Circle circle;

        Sphere::Point pt = stop.second->position;
        circle.SetCenter(GetPoint({ pt.latitude, pt.longitude }));
        circle.SetRadius(svg_params.stop_radius);
        circle.SetFillColor("white");

        document.Add(circle);   
    }
}

void SvgBuilder::AddNames(const Descriptions::StopsDict& stops_dict) {
    for (const auto& stop : stops_dict) {
        
        std::vector<Svg::Text> text(2);

        Sphere::Point pt = stop.second->position;
        Svg::Point center = GetPoint({ pt.latitude, pt.longitude });

        for (int i = 0; i < 2; ++i) {
            text[i].SetPoint(center);
            text[i].SetOffset(svg_params.stop_label_offset);
            text[i].SetFontSize(svg_params.stop_label_font_size);
            text[i].SetFontFamily("Verdana");
            text[i].SetData(stop.first);
        }
        text[0].SetFillColor(svg_params.underlayer_color);
        text[0].SetStrokeColor(svg_params.underlayer_color);
        text[0].SetStrokeWidth(svg_params.underlayer_width);
        text[0].SetStrokeLineCap("round");
        text[0].SetStrokeLineJoin("round");

        text[1].SetFillColor("black");

        document.Add(text[0]);
        document.Add(text[1]);
    }
}


void SvgBuilder::AddNumbers(const Descriptions::BusesDict& buses_dict,
    const Descriptions::StopsDict& stops_dict) {

    size_t count_colors = svg_params.color_palette.size();
    size_t color_id = 0;

    for (const auto& [name, bus] : buses_dict) {
        std::vector<Svg::Text> text(2);
        Sphere::Point pt = stops_dict.at(bus->stops[0])->position;
        
        for (int i = 0; i < 2; ++i) {
            text[i].SetPoint(GetPoint({ pt.latitude, pt.longitude }));
            text[i].SetOffset(svg_params.bus_label_offset);
            text[i].SetFontSize(svg_params.bus_label_font_size);
            text[i].SetFontFamily("Verdana");
            text[i].SetFontWeight("bold");
            text[i].SetData(name);
        }
        text[0].SetFillColor(svg_params.underlayer_color);
        text[0].SetStrokeColor(svg_params.underlayer_color);
        text[0].SetStrokeWidth(svg_params.underlayer_width);
        text[0].SetStrokeLineCap("round");
        text[0].SetStrokeLineJoin("round");

        text[1].SetFillColor(svg_params.color_palette[color_id]);

        document.Add(text[0]);
        document.Add(text[1]);

        if (!bus->is_roundtrip && bus->stops[bus->stops.size() / 2] != bus->stops[0]) {
            std::vector<Svg::Text> text(2);
            Sphere::Point pt = stops_dict.at(bus->stops[bus->stops.size() / 2])->position;

            for (int i = 0; i < 2; ++i) {
                text[i].SetPoint(GetPoint({ pt.latitude, pt.longitude }));
                text[i].SetOffset(svg_params.bus_label_offset);
                text[i].SetFontSize(svg_params.bus_label_font_size);
                text[i].SetFontFamily("Verdana");
                text[i].SetFontWeight("bold");
                text[i].SetData(name);
            }
            text[0].SetFillColor(svg_params.underlayer_color);
            text[0].SetStrokeColor(svg_params.underlayer_color);
            text[0].SetStrokeWidth(svg_params.underlayer_width);
            text[0].SetStrokeLineCap("round");
            text[0].SetStrokeLineJoin("round");

            text[1].SetFillColor(svg_params.color_palette[color_id]);

            document.Add(text[0]);
            document.Add(text[1]);
        }

        color_id++;
        if (color_id == count_colors) color_id = 0;
    }
}

std::string SvgBuilder::GetResult() const {
    std::ostringstream stream;
    document.Render(stream);
    return std::move(stream.str());
}

string TransportCatalog::RenderMap() const {
    SvgBuilder svg_builder(svg_params, stops_dict);

    svg_builder.AddBuses(buses_dict, stops_dict);
    svg_builder.AddNumbers(buses_dict, stops_dict);
    svg_builder.AddStops(stops_dict);
    svg_builder.AddNames(stops_dict);

    std::string result = svg_builder.GetResult();

    ////
    setlocale(LC_ALL, "Russian");
    std::ofstream out("output.svg");
    out << result;
    out.close();
    ////

    return std::move(result);
}