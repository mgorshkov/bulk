cmake_minimum_required(VERSION 3.2)
project(bulk VERSION 0.0.$ENV{TRAVIS_BUILD_NUMBER})

configure_file(version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/version.h)

set(DEBUG_PRINT "0")

if (DEBUG_PRINT)
	message("DEBUG_PRINT is activated!")
	add_definitions(-DDEBUG_PRINT)
endif()

add_executable(bulk bulk.cpp)

set_target_properties(bulk PROPERTIES
	COMPILE_OPTIONS -Wpedantic -Wall -Wextra
	CMAKE_CXX_STANDARD 17
	CMAKE_CXX_STANDARD_REQUIRED ON
)

set(CMAKE_CXX_FLAGS "-std=c++1z")

target_link_libraries(bulk)

install(TARGETS bulk RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT mikhail.gorshkov@gmail.com)

include(CPack)
