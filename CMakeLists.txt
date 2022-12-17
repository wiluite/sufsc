cmake_minimum_required(VERSION 3.6.2)
project(sufsc)

# no memory allocations info if ON
option (SAMPLE_WITH_SANITY_CHECK "[FASTSTRINGCONCATENATOR] Build with sanitizers" ON)

if ((SAMPLE_WITH_SANITY_CHECK) AND (UNIX) AND (NOT (CMAKE_SYSTEM_NAME STREQUAL "CYGWIN")))
    set (SAMPLE_COMPILER_DEFINES "#define SANITY_CHECK")
    set (SAMPLE_ADDITIONAL_COMPILE_FLAGS "-fsanitize=address,leak")
    set (SAMPLE_ADDITIONAL_LINK_FLAGS "-fsanitize=address,leak")
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

# Memory sanitizer can be used with no chance to override global new/delete with signatures as in main()
if (${CMAKE_VERSION} STREQUAL "3.6.2")
    set (SAMPLE_CXX_STANDARD "-std=c++17")
else()
    set(CMAKE_CXX_STANDARD 17)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
    set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "${SAMPLE_CXX_STANDARD} -stdlib=libc++ ${SAMPLE_ADDITIONAL_COMPILE_FLAGS} ")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
    set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "${SAMPLE_CXX_STANDARD} ${SAMPLE_ADDITIONAL_COMPILE_FLAGS} ")
else()
    message(FATAL_ERROR "Adjust this CMakeLists.txt for your compiler and settings, please!")
endif()

add_executable(sample main.cpp fast_string_concatenator.hpp short_alloc.h config.h)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
    target_link_libraries(sample  -stdlib=libc++ ${SAMPLE_ADDITIONAL_LINK_FLAGS})
elseif( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
    target_link_libraries(sample  ${SAMPLE_ADDITIONAL_LINK_FLAGS})
endif()
