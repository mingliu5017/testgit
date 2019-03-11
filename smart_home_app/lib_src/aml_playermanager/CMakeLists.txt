cmake_minimum_required(VERSION 3.2)

file(GLOB_RECURSE c_files_aml_playermanager	"*.c") 
file(GLOB_RECURSE cxx_files_aml_playermanager "*.cpp") 

add_library(aml_playermanager SHARED ${c_files_aml_playermanager} ${cxx_files_aml_playermanager})