include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
# For Windows: prevents gtest from overriding your compiler's runtime settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

# Function to create test executables
function(add_GamePlayer_test test_name source_file)
    add_executable(${test_name} ${source_file})
    set_target_properties(${test_name} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
    if(WIN32)
        target_compile_definitions(${test_name}
            PRIVATE
                NOMINMAX
                WIN32_LEAN_AND_MEAN
                VC_EXTRALEAN
                _CRT_SECURE_NO_WARNINGS
                _SECURE_SCL=0
                _SCL_SECURE_NO_WARNINGS
        )
    endif()

    target_link_libraries(${test_name} 
        PRIVATE 
            ${PROJECT_NAME}::${PROJECT_NAME}
            GTest::gtest
            GTest::gtest_main
            GTest::gmock
    )
    gtest_discover_tests(${test_name})
    message(STATUS "Added test executable: ${test_name}")
endfunction()

file(GLOB SOURCES CONFIGURE_DEPENDS "*.cpp")

message(STATUS "Building tests for ${PROJECT_NAME}")

foreach(FILE ${SOURCES})
    get_filename_component(TEST ${FILE} NAME_WE)
    add_GamePlayer_test("${PROJECT_NAME}_${TEST}" ${FILE})
endforeach()
