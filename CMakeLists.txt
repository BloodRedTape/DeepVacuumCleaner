cmake_minimum_required (VERSION 3.8)

cmake_policy(SET CMP0091 NEW)

project ("DeepVacuumCleaner")

add_subdirectory(libs/sfml)
add_subdirectory(libs/sfpl)
add_subdirectory(libs/bsl)
find_package(imgui REQUIRED)
find_package(freetype REQUIRED)

add_library(ImGui-SFML STATIC "libs/imgui-sfml/imgui-SFML.cpp")
target_include_directories(ImGui-SFML PUBLIC "libs/imgui-sfml/")
target_link_libraries(ImGui-SFML PUBLIC freetype PUBLIC SFML::System PUBLIC SFML::Graphics PUBLIC imgui::imgui)


add_executable (DeepVacuumCleaner 
	"sources/main.cpp" 
	"sources/utils/nn.cpp"
	"sources/application.cpp" 
	"sources/utils/render.cpp" 
	"sources/env/environment.cpp" 
	"sources/model/vacuum_cleaner.cpp" 
	"sources/map_editor.cpp"
	"sources/agents/stupid_agent.cpp" 
	"sources/agents/nn_agent.cpp" 
	"sources/model/neural_network_agent.cpp" 
	"sources/model/vacuum_cleaner_operator.cpp" 
	"sources/model/evolution_training.cpp"
	"sources/env/grid.cpp"
	"sources/env/coverage.cpp" 
	"sources/env/wall.cpp" 
	"sources/env/graph.cpp"
	"sources/env/path.cpp"
 "sources/agents/agent.cpp" "sources/agents/manual.cpp")

target_include_directories(DeepVacuumCleaner PUBLIC "./sources")

target_link_libraries(DeepVacuumCleaner ImGui-SFML bsl sfpl)

