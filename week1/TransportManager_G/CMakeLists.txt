set(PROJECT
	TransportManager_G)

set (HEADERS 
	descriptions.h
	graph.h
	json.h
	requests.h
	router.h
	sphere.h
	transport_catalog.h
	transport_router.h
	utils.h 
	svg.h
	render_settings.h

	)
 
set (SOURCES 
	descriptions.cpp
	json.cpp
	requests.cpp
	sphere.cpp
	transport_catalog.cpp
	transport_router.cpp
	utils.cpp
	main.cpp
	svg.cpp
	
	) 


add_executable (${PROJECT} ${HEADERS} ${SOURCES})