cmake_minimum_required (VERSION 3.8)

project ("DeepVacuumCleaner")

add_subdirectory(libs/sfml)

add_executable (DeepVacuumCleaner 
	"sources/main.cpp" 
	"sources/nn.cpp"
	"sources/application.cpp" 
	"sources/render.cpp" 
	"sources/environment.cpp" 
	"sources/vacuum_cleaner.cpp" 
	"sources/map_editor.cpp"
 "sources/stupid_agent.cpp" "sources/neural_network_agent.cpp" "sources/vacuum_cleaner_operator.cpp" "sources/evolution_training.cpp")

target_link_libraries(DeepVacuumCleaner SFML::System SFML::Window SFML::Graphics sfml-system)

