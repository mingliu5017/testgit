cmake_minimum_required(VERSION 3.2)

file(GLOB_RECURSE c_files_pcm_player_recorder		"*.c") 
file(GLOB_RECURSE cxx_files_pcm_player_recorder	"*.cpp") 

add_library(pcm_player_recorder SHARED ${c_files_pcm_player_recorder} ${cxx_files_pcm_player_recorder})