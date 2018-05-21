cmake_minimum_required(VERSION 3.2)
project (DAEMONBOT)
set (CMAKE_CXX_STANDARD 14)

file(GLOB_RECURSE sources "src/*.cpp")
  
include_directories(
	extern/poco-1.9.0/Foundation/include
	extern/poco-1.9.0/JSON/include
	extern/poco-1.9.0/Net/include
	extern/poco-1.9.0/Util/include
	extern/cereal-1.2.2/include
    )
 
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
 
if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
    set(CMAKE_CXX_FLAGS_MINSIZEREL, "-Os -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE, "-O4 -DNDEBUG")
endif()
	
if (MSVC) 
  link_directories(
	extern/poco-1.9.0/lib/win32
    )

	add_executable(DAEMONBOT ${sources})

	target_link_libraries(DAEMONBOT
		debug "PocoJSONd.lib"			optimized	"PocoJSON.lib"
		debug "PocoUtild.lib"			optimized	"PocoUtil.lib"
		debug "PocoNetd.lib"			optimized	"PocoNet.lib"	
		debug "PocoFoundationd.lib"		optimized	"PocoFoundation.lib"
	)
else()
  find_package(Threads REQUIRED)
  link_directories(
	extern/sleepy-discord/lib/linux
	extern/poco-1.9.0/lib/linux
    )

	add_executable(DAEMONBOT ${sources})

	target_link_libraries(DAEMONBOT
		debug "PocoJSONd"		optimized	"PocoJSON"
		debug "PocoUtild"		optimized	"PocoUtil"
		debug "PocoNetd"		optimized	"PocoNet"
		debug "PocoFoundationd"	optimized	"PocoFoundation"
		debug "pthread"			optimized	"pthread")

endif()
