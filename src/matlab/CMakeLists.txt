# Check if we want to build the MATLAB interface 
mark_as_advanced(CLEAR ENABLE_MATLAB)
set(ENABLE_MATLAB OFF CACHE BOOL "Enable the MATLAB build?")
if(ENABLE_MATLAB)
    # Common includes
    include_directories(${OPTIZELLE_INCLUDE_DIRS})
    include_directories(${JSONCPP_INCLUDE_DIRS})

    # Make sure we can find the MATLAB libraries and then add the
    # directory with the sources.
    find_package(MATLAB REQUIRED)
    add_subdirectory(optizelle)

    # Install the setup file
    install(FILES setupOptizelle.m DESTINATION share/optizelle/matlab/)
endif()
