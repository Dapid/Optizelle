# Sets up and runs Optizelle unit tests 
macro(add_optizelle_json_test_cpp files executable)
            
    # Grab any extra arguments
    set (extra_macro_args ${ARGN})
    list(LENGTH extra_macro_args num_extra_args)
    if (${num_extra_args} GREATER 0)
        list(GET extra_macro_args 0 optional_arg)
    endif()

    # Make sure that tests are enabled
    if(ENABLE_CPP_UNIT)

        # Grab all of the different .json test files
        file(GLOB_RECURSE units ${CMAKE_CURRENT_SOURCE_DIR} ${files})

        # Make sure we run everything in order
        list(SORT units)

        # Remove any optional json file from this list
        if (${num_extra_args} GREATER 0)
            list(REMOVE_ITEM units ${optional_arg})
        endif()

        # We start indexing from 0, but the length parameter starts at 1.
        # We do the math here to fix it.
        list(LENGTH units len1)
        math(EXPR len2 "${len1} - 1")

        # Loop over each of the tests
        foreach(index RANGE ${len2})

            # Grab the particular unit test
            list(GET units ${index} unit)

            # Run the optimization
            if (${num_extra_args} EQUAL 0)
                add_test("Execution_of_cpp_${unit}" ${executable} ${unit})
            else()
                add_test("Execution_of_cpp_${unit}" ${executable} ${unit}
                    ${optional_arg})
            endif()

            # Diff the result of the optimization against the known solution
            add_test("Solution_to_cpp_${unit}"
                "${CMAKE_BINARY_DIR}/src/unit/utility/diff_restart"
                ${unit} solution.json)
        endforeach()
    endif()

endmacro()
