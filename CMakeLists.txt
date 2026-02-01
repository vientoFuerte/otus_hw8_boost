cmake_minimum_required(VERSION 3.12)

project(bayan)

option(WITH_BOOST_TEST "Whether to build Boost test" ON)
# Опция для включения Doxygen
option(WITH_DOXYGEN "Build Doxygen documentation" OFF)

add_executable(bayan main.cpp bayan.cpp)

set_target_properties(bayan PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)


# Boost.Test (если включена опция и файл test.cpp существует)
if(WITH_BOOST_TEST AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
    # Ищем Boost с библиотекой unit_test_framework
    find_package(Boost 1.70 REQUIRED COMPONENTS unit_test_framework)  # Изменено: добавили REQUIRED и компонент
    
    message(STATUS "Found Boost version: ${Boost_VERSION}")
    
    # Тестовый исполняемый файл
    add_executable(bayan_tests test.cpp bayan.cpp)
    
    # Устанавливаем стандарт C++ для тестов
    set_target_properties(bayan_tests PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    
    # Линкуем с Boost.Test библиотекой
    target_link_libraries(bayan_tests PRIVATE ${Boost_LIBRARIES})  # Изменено: добавляем линковку
    
    # Добавляем include директории Boost
    target_include_directories(bayan_tests PRIVATE ${Boost_INCLUDE_DIRS})
    
    # Тесты
    enable_testing()
    add_test(NAME AllTests COMMAND bayan_tests)
    
    message(STATUS "Boost.Test enabled")
    
elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
    message(WARNING "test.cpp found but Boost.Test is disabled. Use -DWITH_BOOST_TEST=ON to enable")
else()
    message(WARNING "test.cpp not found, tests will not be built")
endif()

if (MSVC)
    target_compile_options(bayan PRIVATE
        /W4
    )
else ()
    target_compile_options(bayan PRIVATE
        -Wall -Wextra -pedantic -Werror -Wunused -Wunused-variable -Wunused-parameter
    )
endif()

target_compile_definitions(bayan PRIVATE 
    $<$<CONFIG:Debug>:ENABLE_DEBUG_LOGGING>
    $<$<CONFIG:RelWithDebInfo>:ENABLE_DEBUG_LOGGING>
)


install(TARGETS bayan RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_DEBIAN_FILE_NAME "bayan.deb")
set(CPACK_PACKAGE_CONTACT example@example.com)


include(CPack)
