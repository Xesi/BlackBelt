//#define _GLIBCXX_DEBUG 1
//#define _GLIBCXX_DEBUG_PENDENTIC 1

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

#include "descriptions.h"
#include "json.h"
#include "requests.h"
#include "sphere.h"
#include "transport_catalog.h"
#include "utils.h"

#include <iostream>

using namespace std;

int main() {
    {
        const auto input_doc = Json::Load(cin);
        const auto& input_map = input_doc.GetRoot().AsMap();

        const TransportCatalog db(
            Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray()),
            input_map.at("routing_settings").AsMap()
        );

        Json::PrintValue(
            Requests::ProcessAll(db, input_map.at("stat_requests").AsArray()),
            cout
        );
        cout << endl;
    }
  //_CrtDumpMemoryLeaks();
  return 0;
}
