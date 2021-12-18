cmake_minimum_required (VERSION 3.13)
project (UDP_filestream)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_executable(tserver)
add_executable(tclient)

# set up c++ version
set(CXX_VERSION 20)
#add_compile_options("/std:c++20")
set_property(TARGET tserver PROPERTY CXX_STANDARD ${CXX_VERSION})
set_property(TARGET tclient PROPERTY CXX_STANDARD ${CXX_VERSION})
add_definitions(-DCXX_VERSION=${CXX_VERSION}) # Visual Studio cmake err fix

# other compiler options
#add_compile_options("/O2")
#message("setting /favor:${CMAKE_HOST_SYSTEM_PROCESSOR}")
#add_compile_options("/favor:${CMAKE_HOST_SYSTEM_PROCESSOR}")

target_include_directories(tserver PRIVATE "common") # for include resolve
target_include_directories(tclient PRIVATE "common") # for include resolve
add_subdirectory(common)
add_subdirectory(udp_server)
add_subdirectory(udp_client)