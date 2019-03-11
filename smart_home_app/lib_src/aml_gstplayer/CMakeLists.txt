cmake_minimum_required(VERSION 3.2)

file(GLOB_RECURSE c_files_aml_gstplayer	"*.c") 
file(GLOB_RECURSE cxx_files_aml_gstplayer	"*.cpp") 

add_library(aml_gstplayer SHARED ${c_files_aml_gstplayer} ${cxx_files_aml_gstplayer})