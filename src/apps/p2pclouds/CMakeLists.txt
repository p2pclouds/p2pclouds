PROJECT(p2pclouds)

file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp)
source_group("Include" FILES ${CURRENT_HEADERS}) 

aux_source_directory(. DIR_LIB_SRCS)

add_executable(p2pclouds ${DIR_LIB_SRCS} ${CURRENT_HEADERS})

target_link_libraries(p2pclouds app)
target_link_libraries(p2pclouds network)
target_link_libraries(p2pclouds log)
target_link_libraries(p2pclouds common)
target_link_libraries(p2pclouds blockchain)
target_link_libraries(p2pclouds kademlia)

target_link_libraries(p2pclouds asio)
target_link_libraries(p2pclouds kcp)
target_link_libraries(p2pclouds gflags)
target_link_libraries(p2pclouds libopenssl)

IF(UNIX)
target_link_libraries(p2pclouds pthread)
target_link_libraries(p2pclouds dl)
ELSE(UNIX)
target_link_libraries(p2pclouds Crypt32)
ENDIF(UNIX)


