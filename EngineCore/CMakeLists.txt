include_directories("${PROJECT_SOURCE_DIR}/ThirdParty")


file(GLOB_RECURSE KEWindow "KEWindow.cpp" "KEWindow.h")
source_group("KEWindow" FILES ${KEWindow})
add_library(EngineCore STATIC ${KEWindow})
target_link_libraries(EngineCore SDL2 vulkan-1)




