cmake_minimum_required(VERSION 3.2)

file(GLOB_RECURSE c_files_key_event		"*.c") 

add_library(key_event SHARED ${c_files_key_event})
