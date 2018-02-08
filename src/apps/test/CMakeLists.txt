PROJECT(testapp)

file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp)
source_group("Include" FILES ${CURRENT_HEADERS}) 

aux_source_directory(. DIR_LIB_SRCS)

add_executable(testapp ${DIR_LIB_SRCS} ${CURRENT_HEADERS})

target_link_libraries(testapp app)
target_link_libraries(testapp network)
target_link_libraries(testapp log)
target_link_libraries(testapp common)

target_link_libraries(testapp asio)
target_link_libraries(testapp kcp)
target_link_libraries(testapp gflags)

IF(UNIX)
target_link_libraries(testapp pthread)
ELSE(UNIX)
ENDIF(UNIX)





