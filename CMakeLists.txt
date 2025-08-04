cmake_minimum_required(VERSION 3.21)
project(GamePlayer 
    VERSION 0.1.0 
    LANGUAGES CXX 
    DESCRIPTION "A game player for two-player perfect information games, implemented using a min-max game tree"
)

# Use modern CMake policies
cmake_policy(SET CMP0077 NEW)  # option() honors normal variables
cmake_policy(SET CMP0074 NEW)  # find_package uses <PackageName>_ROOT variables

#########################################################################
# Project Options                                                      #
#########################################################################

option(BUILD_SHARED_LIBS "Build libraries as DLLs" OFF)
option(${PROJECT_NAME}_ANALYSIS_TRANSPOSITION_TABLE "General TranspositionTable analysis is enabled if true" OFF)
option(${PROJECT_NAME}_ANALYSIS_GAME_TREE "General GameTree analysis is enabled if true" OFF)
option(${PROJECT_NAME}_ANALYSIS_GAME_STATE "General GameState analysis is enabled if true" OFF)
option(${PROJECT_NAME}_DEBUG_GAME_TREE_NODE_INFO "GameTree info is dumped if true" OFF)
option(${PROJECT_NAME}_FEATURE_INCREMENTAL_STATIC_EVALUATION "Incremental static evaluation is enabled if true" OFF)
option(${PROJECT_NAME}_FEATURE_INCREMENTAL_STATIC_EVALUATION_VALIDATION "Validation of incremental static evaluation is enabled if true" OFF)
option(${PROJECT_NAME}_FEATURE_NEGAMAX "Use negamax algorithm rather than min-max" OFF)
option(${PROJECT_NAME}_FEATURE_PRIORITIZED_MOVE_ORDERING "Prioritized move ordering is enabled if true" OFF)
option(${PROJECT_NAME}_FEATURE_QUIESCENT_SEARCH "Quiescent search is enabled if true" OFF)
#option(${PROJECT_NAME}_ANALYSIS_PLAYER "General GamePlayer analysis is enabled if true" OFF)
#option(${PROJECT_NAME}_FEATURE_BITBOARD_MOVE_GENERATION "Move generation is done with bitboards if true" OFF)
#option(${PROJECT_NAME}_FEATURE_INTERNAL_ITERATIVE_DEEPENING "Internal iterative deepening is enabled if true" OFF)
#option(${PROJECT_NAME}_FEATURE_ITERATIVE_DEEPENING "Iterative deepening is enabled if true" OFF)
#option(${PROJECT_NAME}_FEATURE_NULL_MOVE "Null move search is enabled if true" OFF)
#option(${PROJECT_NAME}_FEATURE_TRANSPOSITION_TABLE_INCLUDES_PRUNED_VALUES "Transposition table includes results of pruned searches" OFF)

# Print configuration summary
message(STATUS "${PROJECT_NAME} Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Shared Libraries: ${BUILD_SHARED_LIBS}")
message(STATUS "ANALYSIS_GAME_STATE                               : ${${PROJECT_NAME}_ANALYSIS_GAME_STATE}")
message(STATUS "ANALYSIS_GAME_TREE                                : ${${PROJECT_NAME}_ANALYSIS_GAME_TREE}")
message(STATUS "ANALYSIS_TRANSPOSITION_TABLE                      : ${${PROJECT_NAME}_ANALYSIS_TRANSPOSITION_TABLE}")
message(STATUS "DEBUG_GAME_TREE_NODE_INFO                         : ${${PROJECT_NAME}_DEBUG_GAME_TREE_NODE_INFO}")
message(STATUS "FEATURE_INCREMENTAL_STATIC_EVALUATION             : ${${PROJECT_NAME}_FEATURE_INCREMENTAL_STATIC_EVALUATION}")
message(STATUS "FEATURE_INCREMENTAL_STATIC_EVALUATION_VALIDATION  : ${${PROJECT_NAME}_FEATURE_INCREMENTAL_STATIC_EVALUATION_VALIDATION}")
message(STATUS "FEATURE_NEGAMAX                                   : ${${PROJECT_NAME}_FEATURE_NEGAMAX}")
message(STATUS "FEATURE_PRIORITIZED_MOVE_ORDERING                 : ${${PROJECT_NAME}_FEATURE_PRIORITIZED_MOVE_ORDERING}")
message(STATUS "FEATURE_QUIESCENT_SEARCH                          : ${${PROJECT_NAME}_FEATURE_QUIESCENT_SEARCH}")
#message(STATUS "ANALYSIS_PLAYER                                   : ${${PROJECT_NAME}_ANALYSIS_PLAYER}")
#message(STATUS "FEATURE_BITBOARD_MOVE_GENERATION                  : ${${PROJECT_NAME}_FEATURE_BITBOARD_MOVE_GENERATION}")
#message(STATUS "FEATURE_INTERNAL_ITERATIVE_DEEPENING              : ${${PROJECT_NAME}_FEATURE_INTERNAL_ITERATIVE_DEEPENING}")
#message(STATUS "FEATURE_ITERATIVE_DEEPENING                       : ${${PROJECT_NAME}_FEATURE_ITERATIVE_DEEPENING}")
#message(STATUS "FEATURE_NULL_MOVE                                 : ${${PROJECT_NAME}_FEATURE_NULL_MOVE}")
#message(STATUS "FEATURE_TRANSPOSITION_TABLE_INCLUDES_PRUNED_VALUES: ${${PROJECT_NAME}_FEATURE_TRANSPOSITION_TABLE_INCLUDES_PRUNED_VALUES}")

#########################################################################
# Dependencies                                                         #
#########################################################################

include(FetchContent)
find_package(nlohmann_json REQUIRED)

#########################################################################
# Library Target                                                       #
#########################################################################

set(PUBLIC_HEADERS
    include/GamePlayer/GameState.h
    include/GamePlayer/GameTree.h
    include/GamePlayer/StaticEvaluator.h
    include/GamePlayer/TranspositionTable.h
)

set(PRIVATE_SOURCES
    GameState.cpp
    GameTree.cpp
    TranspositionTable.cpp
)

add_library(${PROJECT_NAME} ${PRIVATE_SOURCES} ${PUBLIC_HEADERS})

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    DEBUG_POSTFIX d
    EXPORT_NAME ${PROJECT_NAME}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}
)

target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
        nlohmann_json::nlohmann_json
)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME}
        PRIVATE
            NOMINMAX
            WIN32_LEAN_AND_MEAN
            VC_EXTRALEAN
            _CRT_SECURE_NO_WARNINGS
            _SECURE_SCL=0
            _SCL_SECURE_NO_WARNINGS
    )
endif()

# Features
target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        $<$<BOOL:${${PROJECT_NAME}_ANALYSIS_GAME_STATE}>:ANALYSIS_GAME_STATE=1>
        $<$<BOOL:${${PROJECT_NAME}_ANALYSIS_GAME_TREE}>:ANALYSIS_GAME_TREE=1>
        $<$<BOOL:${${PROJECT_NAME}_ANALYSIS_TRANSPOSITION_TABLE}>:ANALYSIS_TRANSPOSITION_TABLE=1>
        $<$<BOOL:${${PROJECT_NAME}_DEBUG_GAME_TREE_NODE_INFO}>:DEBUG_GAME_TREE_NODE_INFO=1>
        $<$<BOOL:${${PROJECT_NAME}_FEATURE_INCREMENTAL_STATIC_EVALUATION}>:FEATURE_INCREMENTAL_STATIC_EVALUATION=1>
        $<$<BOOL:${${PROJECT_NAME}_FEATURE_INCREMENTAL_STATIC_EVALUATION_VALIDATION}>:FEATURE_INCREMENTAL_STATIC_EVALUATION_VALIDATION=1>
        $<$<BOOL:${${PROJECT_NAME}_FEATURE_NEGAMAX}>:FEATURE_NEGAMAX=1>
        $<$<BOOL:${${PROJECT_NAME}_FEATURE_PRIORITIZED_MOVE_ORDERING}>:FEATURE_PRIORITIZED_MOVE_ORDERING=1>
        $<$<BOOL:${${PROJECT_NAME}_FEATURE_QUIESCENT_SEARCH}>:FEATURE_QUIESCENT_SEARCH=1>
#        $<$<BOOL:${${PROJECT_NAME}_ANALYSIS_PLAYER}>:ANALYSIS_PLAYER=1>
#        $<$<BOOL:${${PROJECT_NAME}_FEATURE_BITBOARD_MOVE_GENERATION}>:FEATURE_BITBOARD_MOVE_GENERATION=1>
#        $<$<BOOL:${${PROJECT_NAME}_FEATURE_INTERNAL_ITERATIVE_DEEPENING}>:FEATURE_INTERNAL_ITERATIVE_DEEPENING=1>
#        $<$<BOOL:${${PROJECT_NAME}_FEATURE_ITERATIVE_DEEPENING}>:FEATURE_ITERATIVE_DEEPENING=1>
#        $<$<BOOL:${${PROJECT_NAME}_FEATURE_NULL_MOVE}>:FEATURE_NULL_MOVE=1>
#        $<$<BOOL:${${PROJECT_NAME}_FEATURE_TRANSPOSITION_TABLE_INCLUDES_PRUNED_VALUES}>:FEATURE_TRANSPOSITION_TABLE_INCLUDES_PRUNED_VALUES=1>
)

# Organize source files for IDEs
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PRIVATE_SOURCES} ${PUBLIC_HEADERS})

#########################################################################
# Documentation                                                         #
#########################################################################

option(${PROJECT_NAME}_BUILD_DOCS "Build documentation using Doxygen" OFF)

if(${PROJECT_NAME}_BUILD_DOCS)
    find_package(Doxygen QUIET)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
        set(DOXYGEN_PROJECT_BRIEF ${PROJECT_DESCRIPTION})
        set(DOXYGEN_PROJECT_VERSION ${PROJECT_VERSION})
        set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs)

        doxygen_add_docs(${PROJECT_NAME}_docs
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/README.md
            COMMENT "Generating documentation for ${PROJECT_NAME}"
        )
    else()
        message(WARNING "Doxygen not found. Documentation will not be built.")
    endif()
endif()

#########################################################################
# Testing                                                               #
#########################################################################

# Only enable testing if this is the main project and testing is explicitly requested
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    enable_testing()
    message(STATUS "Testing is enabled for ${PROJECT_NAME}")
    add_subdirectory(test)
endif()

#########################################################################
# Installation                                                          #
#########################################################################

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

# Install targets
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT Runtime
        NAMELINK_COMPONENT Development
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT Development
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT Runtime
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install headers
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Development
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# Install targets file
install(EXPORT ${PROJECT_NAME}-targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CONFIG_INSTALL_DIR}
    COMPONENT Development
)

# Create package config file
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${CONFIG_INSTALL_DIR}
)

# Create package version file
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install config files
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${CONFIG_INSTALL_DIR}
    COMPONENT Development
)

# Export targets from build tree
export(EXPORT ${PROJECT_NAME}-targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
)

# Register package in user's package registry
export(PACKAGE ${PROJECT_NAME})
