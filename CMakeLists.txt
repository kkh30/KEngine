cmake_minimum_required(VERSION 3.9)


project(KEngine)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


include_directories(${PROJECT_SOURCE_DIR}/EngineCore)
include_directories(${PROJECT_SOURCE_DIR}/ThirdParty)
include_directories(${PROJECT_SOURCE_DIR}/Renderer)
include_directories(${PROJECT_SOURCE_DIR}/Framework)
include_directories(${PROJECT_SOURCE_DIR}/GamePlay)


link_directories("C:/Windows/SysWOW64")
link_directories("C:/VulkanSDK/1.1.70.1/Lib")
link_directories(${PROJECT_SOURCE_DIR}/ThirdParty/libs)

add_subdirectory(${PROJECT_SOURCE_DIR}/ThirdParty)
add_subdirectory(${PROJECT_SOURCE_DIR}/EngineCore)
add_subdirectory(${PROJECT_SOURCE_DIR}/Framework)
add_subdirectory(${PROJECT_SOURCE_DIR}/GamePlay/)
add_subdirectory(${PROJECT_SOURCE_DIR}/Renderer)
add_subdirectory(${PROJECT_SOURCE_DIR}/WorldEditor)








