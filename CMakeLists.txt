cmake_minimum_required(VERSION 3.27)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "-O3")

project(CustomFighter)

set(Core

	src/core/input_interpreter.cpp
	src/core/video.cpp
	src/core/audio.cpp
	src/core/save.cpp
	src/core/skeleton.cpp
	src/core/animation.cpp
	src/core/math.cpp
	src/core/player.cpp
	src/core/move.cpp
	src/core/net_tools.cpp
	src/core/menu.cpp
	src/core/button.cpp
	src/core/clothing.cpp
	src/core/animated_texture.cpp
	src/core/stage.cpp	
	)

set(ImGui

	ext/imgui/imgui.cpp
	ext/imgui/imgui_demo.cpp
	ext/imgui/imgui_draw.cpp
	ext/imgui/imgui_tables.cpp	
	ext/imgui/imgui_widgets.cpp
	ext/imgui/imgui_stdlib.cpp
	ext/imgui/imgui-SFML.cpp
	)

set(IncludeDirs
	
	src
	ext/SFML-2.6.1/include 
	ext/imgui ext/nativefiledialog-extended-1.2.0/src/include
	ext/nlohmann
	ext/curl-8.9.1_1-win64-mingw/include
	ext/ggpo-master/src/include	
	ext/sockpp-master/include/
	)

set(LibraryDirs

	ext/SFML-2.6.1/build/lib
	ext/nativefiledialog-extended-1.2.0/build/src
	ext/curl-8.9.1_1-win64-mingw/lib
	ext/ggpo-master/build/lib/x64/Release
	ext/sockpp-master/build	
	)

set(LinkLibrary

	sfml-graphics 
	sfml-window
	sfml-audio
	sfml-system

	GGPO
	nfd
	curl
	sockpp

	opengl32
	ws2_32
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
target_link_libraries(Editor PUBLIC ${LinkLibrary})



add_executable(Game
	src/game/main.cpp
	src/game/lobby.cpp	
	src/game/local_game.cpp	
	src/game/net_game.cpp
	src/game/character_select.cpp
	src/game/game_tools.cpp
	src/game/game_state.cpp
	src/game/options.cpp
	src/game/button_config.cpp
	src/game/video_config.cpp

	${Core}
	${ImGui}
	)


target_compile_features(Game PUBLIC cxx_std_17)
target_include_directories(Game PUBLIC ${IncludeDirs})
target_link_directories(Game PUBLIC ${LibraryDirs})
target_link_libraries(Game PUBLIC ${LinkLibrary})