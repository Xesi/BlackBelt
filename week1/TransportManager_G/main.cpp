#include "descriptions.h"
#include "json.h"
#include "requests.h"
#include "sphere.h"
#include "transport_catalog.h"
#include "utils.h"

#include "svg.h"

#include <iostream>

//#include <locale>
//#include <stdlib.h>
//#include <windows.h>

using namespace std;

int main() {
    //SetConsoleCP(1251);
    //SetConsoleOutputCP(1251);
    //setlocale(LC_ALL, "Russian");
    const auto input_doc = Json::Load(cin);
    const auto& input_map = input_doc.GetRoot().AsMap();

    auto queries = Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray());

    const TransportCatalog db(
        queries,
        input_map.at("routing_settings").AsMap(),
        input_map.at("render_settings").AsMap()
    );

    Json::PrintValue(
        Requests::ProcessAll(db, input_map.at("stat_requests").AsArray()),
        cout
    );
    cout << endl;
    return 0;
}
