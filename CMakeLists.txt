cmake_minimum_required(VERSION 3.27)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(CustomFighter)

set(Core

	src/core/input_interpreter.cpp
	src/core/render_instance.cpp
	src/core/save.cpp
	src/core/skeleton.cpp
	src/core/animation.cpp
	src/core/math.cpp
	)

set(ImGui

	ext/imgui/imgui.cpp
	ext/imgui/imgui_demo.cpp
	ext/imgui/imgui_draw.cpp
	ext/imgui/imgui_tables.cpp	
	ext/imgui/imgui_widgets.cpp
	ext/imgui/imgui-SFML.cpp
	)

set(IncludeDirs
	
	src
	ext/SFML-2.6.1/include 
	ext/imgui ext/nativefiledialog-extended-1.2.0/src/include
	ext/nlohmann
	)

set(LibraryDirs

	ext/SFML-2.6.1/lib 
	ext/nativefiledialog-extended-1.2.0/build/src
	)

add_executable(Editor
	src/editor/main.cpp	
	src/editor/editor.cpp
	
	${Core}
	${ImGui}
	)

target_compile_features(Editor PUBLIC cxx_std_17)
target_include_directories(Editor PUBLIC ${IncludeDirs})
target_link_directories(Editor PUBLIC ${LibraryDirs})
target_link_libraries(Editor PUBLIC sfml-graphics sfml-window sfml-audio sfml-system opengl32 nfd)



add_executable(Game
	src/game/main.cpp
	src/game/player.cpp
	
	${Core}
	${ImGui}
	)


target_compile_features(Game PUBLIC cxx_std_17)
target_include_directories(Game PUBLIC ${IncludeDirs})
target_link_directories(Game PUBLIC ${LibraryDirs})
target_link_libraries(Game PUBLIC sfml-graphics sfml-window sfml-audio sfml-system opengl32 nfd)