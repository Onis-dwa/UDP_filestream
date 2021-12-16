cmake_minimum_required (VERSION 3.13)
project (UDP_filestream)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_executable(udp_server)
add_executable(udp_client)
set_property(TARGET udp_server PROPERTY CXX_STANDARD 20)
set_property(TARGET udp_client PROPERTY CXX_STANDARD 20)

# compiler options
if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
	add_compile_options("/O2")
	#add_compile_options("/std:c++20")
	message("setting /favor:${CMAKE_HOST_SYSTEM_PROCESSOR}")
	add_compile_options("/favor:${CMAKE_HOST_SYSTEM_PROCESSOR}")
endif()

target_include_directories(udp_server PRIVATE "common") # for include resolve
target_include_directories(udp_client PRIVATE "common") # for include resolve
add_subdirectory(common)
add_subdirectory(udp_server)
add_subdirectory(udp_client)