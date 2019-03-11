cmake_minimum_required(VERSION 3.2)

file(GLOB_RECURSE c_files_a2dp_ctl		"*.c") 

add_library(a2dp_ctl SHARED ${c_files_a2dp_ctl})
#add_executable(a2dp_ctl ${c_files_a2dp_ctl})
#target_link_libraries(a2dp_ctl  ${LINK_LIBS2})