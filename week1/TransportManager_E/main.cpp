#define _SILENCE_CXX17_C_HEADER_DEPRECATION_WARNING
//#include <bits/stdc++.h>
#include <optional>

#include "json.h"
#include "key_names.h"
#include "bus_manager.h"
#include "graph.h"
#include "router.h"

using namespace std;

int main() {
    Json::Document document = Json::Load(cin);
    BusManager bus_manager(document.GetRoot().AsMap());
    Json::Print(cout, bus_manager.GetResult(document));
}