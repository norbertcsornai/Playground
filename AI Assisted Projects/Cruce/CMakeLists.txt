cmake_minimum_required(VERSION 3.20)

project(Cruce
  VERSION 0.1.0
  DESCRIPTION "Server-authoritative Cruce card game"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(cruce_core
  src/core/deck.cpp
  src/core/match.cpp
  src/core/rules_engine.cpp
  src/core/types.cpp
)

target_include_directories(cruce_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_library(cruce_server
  src/server/game_server.cpp
  src/server/privacy_filter.cpp
  src/server/user_store.cpp
)

target_link_libraries(cruce_server PUBLIC cruce_core)
target_include_directories(cruce_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(cruce_clients
  src/clients/client_app.cpp
)

target_link_libraries(cruce_clients PUBLIC cruce_server)
target_include_directories(cruce_clients PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(cruce_demo src/app/main.cpp)
target_link_libraries(cruce_demo PRIVATE cruce_clients)

if(WIN32)
  add_executable(cruce_server_app src/app/server_main.cpp)
  target_link_libraries(cruce_server_app PRIVATE cruce_clients Ws2_32)
endif()

enable_testing()

add_executable(cruce_core_tests tests/cpp/core_rules_tests.cpp)
target_link_libraries(cruce_core_tests PRIVATE cruce_core)
add_test(NAME cruce_core_tests COMMAND cruce_core_tests)

add_executable(cruce_server_tests tests/cpp/server_tests.cpp)
target_link_libraries(cruce_server_tests PRIVATE cruce_clients)
add_test(NAME cruce_server_tests COMMAND cruce_server_tests)

find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
  add_test(
    NAME cruce_python_smoke
    COMMAND Python3::Interpreter
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/python/integration_smoke.py
            $<TARGET_FILE:cruce_demo>
  )

  add_test(
    NAME cruce_windows_python_compile
    COMMAND Python3::Interpreter
            -m py_compile
            ${CMAKE_CURRENT_SOURCE_DIR}/src/app/windows_client.py
  )

  add_test(
    NAME cruce_ai_python_compile
    COMMAND Python3::Interpreter
            -m py_compile
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/export_training_data.py
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/ai_self_play.py
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/compare_ai_models.py
            ${CMAKE_CURRENT_SOURCE_DIR}/src/ai/__init__.py
            ${CMAKE_CURRENT_SOURCE_DIR}/src/ai/features.py
            ${CMAKE_CURRENT_SOURCE_DIR}/src/ai/model.py
            ${CMAKE_CURRENT_SOURCE_DIR}/src/ai/policy.py
            ${CMAKE_CURRENT_SOURCE_DIR}/src/ai/train.py
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/python/ai_bot_smoke.py
  )

  if(WIN32)
    add_test(
      NAME cruce_local_server_smoke
      COMMAND Python3::Interpreter
              ${CMAKE_CURRENT_SOURCE_DIR}/tests/python/local_server_smoke.py
              $<TARGET_FILE:cruce_server_app>
    )

    add_test(
      NAME cruce_ai_bot_smoke
      COMMAND Python3::Interpreter
              ${CMAKE_CURRENT_SOURCE_DIR}/tests/python/ai_bot_smoke.py
              $<TARGET_FILE:cruce_server_app>
    )
  endif()
endif()
