set(PROJECT
	TransportManager_E
	)

set (HEADERS 
	graph.h
	bus_manager.h
	json.h
	key_names.h
	router.h
	)
 
set (SOURCES 
	json.cpp
	main.cpp) 

	 
add_executable (${PROJECT} ${HEADERS} ${SOURCES})