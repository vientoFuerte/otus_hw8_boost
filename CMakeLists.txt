cmake_minimum_required(VERSION 3.12)

project(bayan)

option(WITH_BOOST_TEST "Whether to build Boost test" ON)
option(WITH_DOXYGEN "Build Doxygen documentation" OFF)

set(Boost_NO_BOOST_CMAKE ON)

# Ищем Boost для основной программы
find_package(Boost REQUIRED COMPONENTS filesystem program_options)

add_executable(bayan main.cpp bayan.cpp)

set_target_properties(bayan PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(bayan PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(bayan PRIVATE 
	${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
)

# Boost.Test
if(WITH_BOOST_TEST AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
    # Для тестов нужен unit_test_framework
    find_package(Boost REQUIRED COMPONENTS unit_test_framework program_options)
    
    add_executable(bayan_tests test.cpp bayan.cpp)
    
    set_target_properties(bayan_tests PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    
    target_include_directories(bayan_tests PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(bayan_tests PRIVATE ${Boost_LIBRARIES})
    
    target_compile_definitions(bayan_tests PRIVATE BOOST_TEST_NO_LIB)
    
    enable_testing()
    add_test(NAME AllTests COMMAND bayan_tests)
    
    message(STATUS "Boost.Test enabled")
endif()

if (MSVC)
    target_compile_options(bayan PRIVATE /W4)
else ()
    target_compile_options(bayan PRIVATE
        -Wall -Wextra -pedantic
    )
endif()

install(TARGETS bayan RUNTIME DESTINATION bin)

include(CPack)
