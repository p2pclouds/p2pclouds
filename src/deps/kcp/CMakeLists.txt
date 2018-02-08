cmake_minimum_required(VERSION 3.6)
project(kcp)

macro( add_submodule dir )
    set( options )
    set( oneValueArgs )
    set( multiValueArgs EXHEADERS )
    cmake_parse_arguments( add_submodule "" "" "${multiValueArgs}" ${ARGN} )

    #message( STATUS "{dir} ${dir}" )
    #message( STATUS "{add_submodule_UNPARSED_ARGUMENTS} ${add_submodule_UNPARSED_ARGUMENTS}" )
    foreach( name ${add_submodule_UNPARSED_ARGUMENTS} )
        set( LIBSRC ${LIBSRC} ${dir}/${name} )
    endforeach( name )

    #message( STATUS "{add_submodule_EXHEADERS} ${add_submodule_EXHEADERS}" )
    foreach( name ${add_submodule_EXHEADERS} )
        set( EXHEADERS ${EXHEADERS} ${dir}/${name} )
    endforeach( name )

endmacro( add_submodule )

add_submodule (. ikcp.c)

add_library(kcp STATIC ${LIBSRC})
set_target_properties(kcp PROPERTIES FOLDER deps)