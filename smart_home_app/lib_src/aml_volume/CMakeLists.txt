cmake_minimum_required(VERSION 3.2)

file(GLOB_RECURSE c_files_aml_volume	"*.c") 
file(GLOB_RECURSE cxx_files_aml_volume	"*.cpp") 

add_library(aml_volume SHARED ${c_files_aml_volume} ${cxx_files_aml_volume})