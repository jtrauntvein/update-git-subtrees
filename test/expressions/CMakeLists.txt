PROJECT(test)

IF(WIN32)
  STRING(REGEX REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
  STRING(REGEX REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE}) 
ENDIF(WIN32)

FIND_LIBRARY(coratools_lib NAME coratools PATHS ../../ ../../debug_static)
ADD_EXECUTABLE(test main.cpp)
IF(coratools_lib)
  IF(WIN32) 
    TARGET_LINK_LIBRARIES(test ${coratools_lib})
  ELSE(WIN32)
    TARGET_LINK_LIBRARIES(test ${coratools_lib} pthread)
  ENDIF(WIN32)
ENDIF(coratools_lib)
  
