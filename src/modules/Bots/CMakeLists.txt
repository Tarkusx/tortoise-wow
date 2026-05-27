# Diagnostic wrapper for the cmangos/playerbots submodule.
# This intentionally only configures the upstream CMake project.
# Do not link the playerbots target into game or mangosd from here.

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/upstream/CMakeLists.txt")
  message(FATAL_ERROR "BUILD_PLAYERBOTS is enabled, but src/modules/Bots/upstream is missing. Run git submodule update --init --recursive.")
endif()

message(STATUS "Configuring cmangos/playerbots from src/modules/Bots/upstream")
add_subdirectory(upstream)

if(TARGET playerbots)
  target_sources(playerbots PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PlayerbotsCompatibility.cpp
  )

  target_compile_options(playerbots PRIVATE
    -include ${CMAKE_CURRENT_SOURCE_DIR}/include/PlayerbotsCompatibility.h
  )

  target_compile_definitions(playerbots PRIVATE
    MANGOSBOT_ZERO
  )

  target_include_directories(playerbots PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/dep/include/g3dlite
    ${CMAKE_SOURCE_DIR}/src/game
    ${CMAKE_SOURCE_DIR}/src/game/AI
    ${CMAKE_SOURCE_DIR}/src/game/AuctionHouse
    ${CMAKE_SOURCE_DIR}/src/game/Chat
    ${CMAKE_SOURCE_DIR}/src/game/Commands
    ${CMAKE_SOURCE_DIR}/src/game/Database
    ${CMAKE_SOURCE_DIR}/src/framework
    ${CMAKE_SOURCE_DIR}/src/game/Battlegrounds
    ${CMAKE_SOURCE_DIR}/src/game/Group
    ${CMAKE_SOURCE_DIR}/src/game/LFG
    ${CMAKE_SOURCE_DIR}/src/game/Handlers
    ${CMAKE_SOURCE_DIR}/src/game/Maps
    ${CMAKE_SOURCE_DIR}/src/game/Maps/Pool
    ${CMAKE_SOURCE_DIR}/src/game/MapNodes
    ${CMAKE_SOURCE_DIR}/src/game/Movement
    ${CMAKE_SOURCE_DIR}/src/game/Movement/spline
    ${CMAKE_SOURCE_DIR}/src/game/Objects
    ${CMAKE_SOURCE_DIR}/src/game/Protocol
    ${CMAKE_SOURCE_DIR}/src/game/Spells
    ${CMAKE_SOURCE_DIR}/src/game/Threat
    ${CMAKE_SOURCE_DIR}/src/game/Transports
    ${CMAKE_SOURCE_DIR}/src/shared
    ${MYSQL_INCLUDE_DIR}
  )
endif()
