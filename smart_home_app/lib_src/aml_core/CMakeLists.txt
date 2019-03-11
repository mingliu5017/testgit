cmake_minimum_required(VERSION 3.2)

file(GLOB_RECURSE c_files_aml_core	"*.c") 
file(GLOB_RECURSE cxx_files_aml_core	"*.cpp") 

add_library(aml_core SHARED ${c_files_aml_core} ${cxx_files_aml_core})