file(GLOB dot_cpp *.cpp)
file(GLOB dot_h *.h)

add_library(GamePlay STATIC ${dot_cpp} ${dot_h})




