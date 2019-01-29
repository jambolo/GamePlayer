cmake_minimum_required (VERSION 3.4)
project(GamePlayer CXX)

option(${PROJECT_NAME}_BUILD_TESTS "Build unit tests for ${PROJECT_NAME} if BUILD_TESTING is ON" FALSE)
option(BUILD_SHARED_LIBS "Build libraries as DLLs" FALSE)

set(${PROJECT_NAME}_DOXYGEN_OUTPUT_DIRECTORY "" CACHE PATH "Doxygen output directory (empty to disable)")

if(${PROJECT_NAME}_DOXYGEN_OUTPUT_DIRECTORY)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_OUTPUT_DIRECTORY ${${PROJECT_NAME}_DOXYGEN_OUTPUT_DIRECTORY})
        configure_file(Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile)
        add_custom_target(doxygen ALL ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
            SOURCES ${PROJECT_BINARY_DIR}/Doxyfile
    )
    endif(DOXYGEN_FOUND)
endif()

#option(FEATURE_QUIESCENT_SEARCH                           "Quiescent search is enabled if true"                            FALSE)
#option(FEATURE_INCREMENTAL_STATIC_EVALUATION_VALIDATION   "Validation of incremental static evaluation is enabled if true" FALSE)
#option(FEATURE_TRANSPOSITION_TABLE_INCLUDES_PRUNED_VALUES "Transposition table includes results of pruned searches"        FALSE)
#option(FEATURE_ITERATIVE_DEEPENING                        "Iterative deepening is enabled if true"                         FALSE)
#option(FEATURE_INTERNAL_ITERATIVE_DEEPENING               "Internal iterative deepening is enabled if true"                FALSE)
#option(FEATURE_NULL_MOVE                                  "Null move search is enabled if true"                            FALSE)
#option(FEATURE_BITBOARD_MOVE_GENERATION                   "Move generation is done with bitboards if true"                 FALSE)
option(FEATURE_NEGAMAX                                    "Use negamax algorithm rath than min-max"                        FALSE)

message(STATUS "FEATURE_QUIESCENT_SEARCH                           : ${FEATURE_QUIESCENT_SEARCH}")
message(STATUS "FEATURE_INCREMENTAL_STATIC_EVALUATION_VALIDATION   : ${FEATURE_INCREMENTAL_STATIC_EVALUATION_VALIDATION}")
message(STATUS "FEATURE_TRANSPOSITION_TABLE_INCLUDES_PRUNED_VALUES : ${FEATURE_TRANSPOSITION_TABLE_INCLUDES_PRUNED_VALUES}")
message(STATUS "FEATURE_ITERATIVE_DEEPENING                        : ${FEATURE_ITERATIVE_DEEPENING}")
message(STATUS "FEATURE_INTERNAL_ITERATIVE_DEEPENING               : ${FEATURE_INTERNAL_ITERATIVE_DEEPENING}")
message(STATUS "FEATURE_NULL_MOVE                                  : ${FEATURE_NULL_MOVE}")
message(STATUS "FEATURE_BITBOARD_MOVE_GENERATION                   : ${FEATURE_BITBOARD_MOVE_GENERATION}")
message(STATUS "FEATURE_NEGAMAX                                    : ${FEATURE_NEGAMAX}")

option(ANALYSIS_PLAYER              "General ComputerPlayer analysis is enabled if true"     FALSE)
option(ANALYSIS_TRANSPOSITION_TABLE "General TranspositionTable analysis is enabled if true" FALSE)
option(ANALYSIS_GAME_TREE           "General GameTree analysis is enabled if true"           FALSE)

message(STATUS "ANALYSIS_PLAYER                                    : ${ANALYSIS_PLAYER}")
message(STATUS "ANALYSIS_TRANSPOSITION_TABLE                       : ${ANALYSIS_TRANSPOSITION_TABLE}")
message(STATUS "ANALYSIS_GAME_TREE                                 : ${ANALYSIS_GAME_TREE}")

#option(DEBUG_GAME_TREE_NODE_INFO "GameTree info is dumped if true" FALSE)

message(STATUS "DEBUG_GAME_TREE_NODE_INFO                          : ${DEBUG_GAME_TREE_NODE_INFO}")

set(SOURCES
    include/GamePlayer/GameState.h
    include/GamePlayer/GameTree.h
    include/GamePlayer/StaticEvaluator.h
    include/GamePlayer/TranspositionTable.h

    GameTree.cpp
    TranspositionTable.cpp
)

set(PRIVATE_INCLUDE_PATHS
    .
)

set(PUBLIC_INCLUDE_PATHS
    include
)

add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${PUBLIC_INCLUDE_PATHS} PRIVATE ${PRIVATE_INCLUDE_PATHS} )
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        -DNOMINMAX
        -DWIN32_LEAN_AND_MEAN
        -DVC_EXTRALEAN
        -D_CRT_SECURE_NO_WARNINGS
        -D_SECURE_SCL=0
        -D_SCL_SECURE_NO_WARNINGS
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

if(FEATURE_QUIESCENT_SEARCH)
    target_compile_definitions(${PROJECT_NAME} PUBLIC FEATURE_QUIESCENT_SEARCH=1)
endif()
if(FEATURE_INCREMENTAL_STATIC_EVALUATION_VALIDATION)
    target_compile_definitions(${PROJECT_NAME} PUBLIC FEATURE_INCREMENTAL_STATIC_EVALUATION_VALIDATION=1)
endif()
if(FEATURE_TRANSPOSITION_TABLE_INCLUDES_PRUNED_VALUES)
    target_compile_definitions(${PROJECT_NAME} PUBLIC FEATURE_TRANSPOSITION_TABLE_INCLUDES_PRUNED_VALUES=1)
endif()
if(FEATURE_ITERATIVE_DEEPENING)
    target_compile_definitions(${PROJECT_NAME} PUBLIC FEATURE_ITERATIVE_DEEPENING=1)
endif()
if(FEATURE_INTERNAL_ITERATIVE_DEEPENING)
    target_compile_definitions(${PROJECT_NAME} PUBLIC FEATURE_INTERNAL_ITERATIVE_DEEPENING=1)
endif()
if(FEATURE_NULL_MOVE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC FEATURE_NULL_MOVE=1)
endif()
if(FEATURE_BITBOARD_MOVE_GENERATION)
    target_compile_definitions(${PROJECT_NAME} PUBLIC FEATURE_BITBOARD_MOVE_GENERATION=1)
endif()
if(FEATURE_NEGAMAX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC FEATURE_NEGAMAX=1)
endif()

if(ANALYSIS_PLAYER)
    target_compile_definitions(${PROJECT_NAME} PUBLIC ANALYSIS_PLAYER=1)
endif()
if(ANALYSIS_TRANSPOSITION_TABLE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC ANALYSIS_TRANSPOSITION_TABLE=1)
endif()
if(ANALYSIS_GAME_TREE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC ANALYSIS_GAME_TREE=1)
endif()

if(DEBUG_GAME_TREE_NODE_INFO)
    target_compile_definitions(${PROJECT_NAME} PUBLIC DEBUG_GAME_TREE_NODE_INFO=1)
endif()

source_group(Sources FILES ${SOURCES})

if(${PROJECT_NAME}_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
