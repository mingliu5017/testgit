cmake_minimum_required(VERSION 3.2)

file(GLOB_RECURSE c_files_aml_utils		"*.c") 
file(GLOB_RECURSE cxx_files_aml_utils	"*.cpp") 

add_library(aml_utils SHARED ${c_files_aml_utils} ${cxx_files_aml_utils})