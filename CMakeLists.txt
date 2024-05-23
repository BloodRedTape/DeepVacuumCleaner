cmake_minimum_required (VERSION 3.8)

cmake_policy(SET CMP0091 NEW)

project ("DeepVacuumCleaner")

add_subdirectory(libs/sfml)
find_package(imgui REQUIRED)
find_package(freetype REQUIRED)

add_library(ImGui-SFML STATIC "libs/imgui-sfml/imgui-SFML.cpp")
target_include_directories(ImGui-SFML PUBLIC "libs/imgui-sfml/")
target_link_libraries(ImGui-SFML PUBLIC freetype PUBLIC SFML::System PUBLIC SFML::Graphics PUBLIC imgui::imgui)


add_executable (DeepVacuumCleaner 
	"sources/main.cpp" 
	"sources/nn.cpp"
	"sources/application.cpp" 
	"sources/render.cpp" 
	"sources/environment.cpp" 
	"sources/vacuum_cleaner.cpp" 
	"sources/map_editor.cpp"
	"sources/stupid_agent.cpp" 
	"sources/neural_network_agent.cpp" 
	"sources/vacuum_cleaner_operator.cpp" 
	"sources/evolution_training.cpp"
)

target_link_libraries(DeepVacuumCleaner ImGui-SFML)

