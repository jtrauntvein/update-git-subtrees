PROJECT(test)

FIND_LIBRARY(coratools_lib NAMES coratools PATHS ../..)
ADD_EXECUTABLE(test main.cpp)
IF(coratools_lib)
   TARGET_LINK_LIBRARIES(test ${coratools_lib} pthread)
ENDIF(coratools_lib)

