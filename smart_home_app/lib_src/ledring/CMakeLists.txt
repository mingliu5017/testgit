cmake_minimum_required(VERSION 3.2)

file(GLOB_RECURSE c_files_ledring		"*.c") 

add_library(ledring SHARED ${c_files_ledring})
