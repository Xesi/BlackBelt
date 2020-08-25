set(PROJECT
	SaveAndLoad
	)

set (HEADERS 
	"saveload.h"
	  "test_runner.h")
 
set (SOURCES 
	"test.cpp"
	) 


add_executable (${PROJECT} ${HEADERS} ${SOURCES} "test_runner.h")