set(PROJECT
	SaveAndLoad
	)

set (HEADERS 
	"saveload.h")
 
set (SOURCES 
	"test.cpp"
	) 


add_executable (${PROJECT} ${HEADERS} ${SOURCES})