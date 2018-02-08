# Display header files in VC
file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp)
source_group("Include" FILES ${CURRENT_HEADERS}) 

# Save the list of all source files to DIR_LIB_SRCS
aux_source_directory(. DIR_LIB_SRCS)

# Generating library
add_library (common ${DIR_LIB_SRCS} ${CURRENT_HEADERS})
set_target_properties(common PROPERTIES FOLDER libs)