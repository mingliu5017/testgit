cmake_minimum_required(VERSION 3.2)

file(GLOB_RECURSE c_files_network_cfg		"*.c") 

add_library(network_cfg SHARED ${c_files_network_cfg})
