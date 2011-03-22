cmake_minimum_required (VERSION 2.8)
project (psort)

include_directories (/usr/local/include)
link_directories (/usr/local/lib)

add_executable (psort main.cpp rcmp.cpp strref.cpp)
target_link_libraries(psort GUtils loki)
