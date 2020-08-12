#pragma once

#include <map>
#include <unordered_map>
#include <string>
#include <set>
#include <cmath>

#include "key_names.h"
#include "json.h"
#include "graph.h"
#include "router.h"

const double PI = 3.1415926535;

struct Stop {
    double x = 0, y = 0;
    std::set<std::string> buses;
    std::unordered_map<std::string, double> dists;
    size_t number;
};

struct Route {
    std::string name;
    std::vector<std::string> stops;
    int unique_count = 0;
};

class BusManager {
public:
    BusManager(const std::map<std::string, Json::Node>& input_map)
        : BUS_WAIT_TIME(input_map.at(key_routing_settings).AsMap().at(key_bus_wait_time).AsInt())
        , BUS_VELOCITY(input_map.at(key_routing_settings).AsMap().at(key_bus_velocity).AsInt())
    {
        const std::vector<Json::Node>& queryes = input_map.at(key_base_requests).AsArray();
        const std::vector<Json::Node>& requests = input_map.at(key_stat_requests).AsArray();
        CreateBd(queryes);
        graph = BuildGraph();
        router = new Graph::Router<double>(graph);
        
    }
    Json::Document GetResult(const Json::Document& document) const;

private:

    const int BUS_WAIT_TIME = 1;
    const int BUS_VELOCITY = 1;

    std::unordered_map<std::string, Stop> stops;
    std::unordered_map<std::string, Route> routes;
    std::vector<const std::string*> stops_name;
    Graph::DirectedWeightedGraph<double> graph;
    Graph::Router<double>* router;

    void CreateBd(const std::vector<Json::Node>& queryes);
    void ReadRoute(const std::map<std::string, Json::Node>& request);
    void ReadStop(const std::map<std::string, Json::Node>& request);
    Graph::DirectedWeightedGraph<double> BuildGraph();
    double CalcDist(const std::string&, const std::string&) const;
    Json::Node GetBusAnswer(const std::map<std::string, Json::Node>& mp) const;
    Json::Node GetStopAnswer(const std::map<std::string, Json::Node>& mp) const;
    Json::Node GetRouteAnswer(const std::map<std::string, Json::Node>& mp) const;
};

void BusManager::CreateBd(const std::vector<Json::Node>& queryes) {

    
    for (const Json::Node& query : queryes) {
        const std::map<std::string, Json::Node>& request = query.AsMap();
        const std::string type_name = "type";
        const std::string& type = request.at(type_name).AsString();

        if (type == "Bus") {
            ReadRoute(request);
        }
        else if (type == "Stop") {
            ReadStop(request);
        }

    }
}

void BusManager::ReadRoute(const std::map<std::string, Json::Node>& request) {

    std::string name = request.at(key_name).AsString();
    const std::vector<Json::Node>& stops_ptr = request.at(key_stops).AsArray();
    bool is_roundtrip = request.at(key_is_roundtrip).AsBool();

    std::vector<std::string> stops_vec;
    std::set<std::string> s;
    for (const Json::Node& node : stops_ptr) {
        stops_vec.push_back(node.AsString());
        stops[stops_vec.back()].buses.insert(name);
        s.insert(stops_vec.back());
    }
    if (!is_roundtrip) {
        for (int i = stops_ptr.size() - 2; i >= 0; --i) {
            stops_vec.push_back(stops_vec[i]);
        }
    }
    routes[name] = { name, stops_vec, int(s.size()) };
}

std::unordered_map<std::string, double> ReadDists(const std::map<std::string, Json::Node>& dist_map) {
    std::unordered_map<std::string, double> dists;
    for (const auto& p : dist_map) {
        dists[p.first] = p.second.AsInt();
    }
    return dists;
}

void BusManager::ReadStop(const std::map<std::string, Json::Node>& request) {

    std::string name = request.at(key_name).AsString();
    double latitude = request.at(key_latitude).AsDouble();
    double longitude = request.at(key_longitude).AsDouble();
    std::unordered_map<std::string, double> dists = ReadDists(request.at(key_road_distances).AsMap());
    stops[name].x = latitude;
    stops[name].y = longitude;
    stops[name].dists = move(dists);
}

Graph::DirectedWeightedGraph<double> BusManager::BuildGraph() {
    Graph::DirectedWeightedGraph<double> graph(stops.size());
    size_t ind = 0;
    for (auto& stop : stops) {
        stop.second.number = ind++;
        stops_name.push_back(&stop.first);
    }
    for (auto& [bus, route] : routes) {
        for (int i = 0; i < route.stops.size() - 1; ++i) {
            double dist = 0;
            for (int j = i + 1; j < route.stops.size(); ++j) {
                const std::string& from = route.stops[i];
                const std::string& to = route.stops[j];
                const std::string& midle = route.stops[j - 1];
                dist += CalcDist(midle, to);
                size_t id_from = stops[from].number;
                size_t id_to = stops[to].number;
                double weight = BUS_WAIT_TIME + dist * 60 / BUS_VELOCITY / 1000;
                graph.AddEdge({id_from, id_to, weight, size_t(j - i), bus });
            }
        }
    }
    return graph;
}

double CalcGeodist(const Stop& a, const Stop& b, double R = 6'371'000) {
    double lhs_lat = a.x * PI / 180;
    double lhs_lon = a.y * PI / 180;
    double rhs_lat = b.x * PI / 180;
    double rhs_lon = b.y * PI / 180;
    return std::acos(std::sin(lhs_lat) * std::sin(rhs_lat) 
        + std::cos(lhs_lat) * std::cos(rhs_lat) 
        * std::cos(std::abs(lhs_lon - rhs_lon))) * R;
}

double BusManager::CalcDist(const std::string& from, const std::string& to) const {
    if (stops.at(from).dists.count(to)) {
        return stops.at(from).dists.at(to);
    }
    else if(stops.at(to).dists.count(from)) {
        return  stops.at(to).dists.at(from);
    }
    return false;
}

Json::Node BusManager::GetBusAnswer(const std::map<std::string, Json::Node>& mp) const {

    std::map<std::string, Json::Node> ans;
    ans[key_request_id] = Json::Node(mp.at(key_id).AsInt());

    const std::string bus = mp.at(key_name).AsString();
    if (routes.count(bus) == 0) {
        ans["error_message"] = Json::Node(key_not_found);
        return ans;
    }
    const Route& route = routes.at(bus);
    double geografical_dist = 0, dist = 0;
    for (int i = 0; i < route.stops.size() - 1; ++i) {
        const std::string& from = route.stops[i];
        const std::string& to = route.stops[i + 1];
        geografical_dist += CalcGeodist(stops.at(from), stops.at(to));
        dist += CalcDist(from, to);
    }
    double curvature = dist / geografical_dist;
    ans[key_route_length] = Json::Node(dist);
    ans[key_curvature] = Json::Node(curvature);
    ans[key_stop_count] = Json::Node((int)route.stops.size());
    ans[key_unique_stop_count] = Json::Node(route.unique_count);

    return ans;
}

Json::Node BusManager::GetStopAnswer(const std::map<std::string, Json::Node>& mp) const {

    std::map<std::string, Json::Node> ans;
    ans[key_request_id] = Json::Node(mp.at(key_id).AsInt());

    const std::string stop = mp.at(key_name).AsString();

    if (stops.count(stop) == 0) {
        ans["error_message"] = Json::Node(key_not_found);
        return ans;
    }
    std::vector<Json::Node> vec;
    for (auto& str : stops.at(stop).buses) {
        vec.push_back(Json::Node(str));
    }
    ans["buses"] = Json::Node(vec);
    return ans;
}

Json::Node BusManager::GetRouteAnswer(const std::map<std::string, Json::Node>& mp) const {
    
    std::map<std::string, Json::Node> ans;
    ans[key_request_id] = Json::Node(mp.at(key_id).AsInt());

    const std::string& from = mp.at(key_from).AsString();
    const std::string& to = mp.at(key_to).AsString();

    size_t id_from = stops.at(from).number;
    size_t id_to = stops.at(to).number;

    auto route = router->BuildRoute(id_from, id_to);
    if (!route) {
        ans["error_message"] = Json::Node(key_not_found);
        return Json::Node(ans);
    }
    ans[key_total_time] = route->weight;

    size_t id = route->id;
    std::vector<Json::Node> steps;

    std::string last_name = "";
    for (size_t i = 0; i < route->edge_count; ++i) {
        const Graph::Edge<double>& step = graph.GetEdge(router->GetRouteEdge(id, i));
        
        std::map<std::string, Json::Node> node_wait, node_bus;
        node_wait["type"] = Json::Node(std::string("Wait"));
        node_bus["type"] = Json::Node(std::string("Bus"));
        node_wait["time"] = Json::Node(BUS_WAIT_TIME);
        node_bus["time"] = Json::Node(step.weight - BUS_WAIT_TIME);
        node_wait["stop_name"] = Json::Node(*stops_name[step.from]);
        node_bus["span_count"] = Json::Node((int)step.count_stops);
        node_bus["bus"] = Json::Node(step.bus);
        
        steps.push_back(Json::Node(node_wait));
        steps.push_back(Json::Node(node_bus));

    }
    ans["items"] = Json::Node(steps);

    router->ReleaseRoute(id);
    return Json::Node(ans);
}

Json::Document BusManager::GetResult(const Json::Document& document) const {
    std::vector<Json::Node> result;
    const std::vector<Json::Node>& requests = document.GetRoot().AsMap().at(key_stat_requests).AsArray();
    
    for (const Json::Node& node : requests) {
        const std::map<std::string, Json::Node>& mp = node.AsMap();
        const std::string type = mp.at(key_type).AsString();
        if (type == "Bus") {
            result.push_back(GetBusAnswer(mp));
        }
        else if (type == "Stop") {
            result.push_back(GetStopAnswer(mp));
        }
        else if (type == "Route") {
            result.push_back(GetRouteAnswer(mp));
        }
    }
    return Json::Document(Json::Node(result));
}