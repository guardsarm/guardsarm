cmake_minimum_required(VERSION 3.12.4)

project(guardsarm-modulesd C CXX)

if(NOT DEFINED GUARDSARM_MAIN_PROJECT)
  get_filename_component(SRC_FOLDER ${CMAKE_SOURCE_DIR}/../ ABSOLUTE)
  list(APPEND CMAKE_MODULE_PATH "${SRC_FOLDER}/cmake")
  include(GuardSarmRpath)
endif()

# Core include directories
include_directories(
  ${SRC_FOLDER}/shared/include/
  ${SRC_FOLDER}/shared/os_regex/
  ${SRC_FOLDER}/external/curl/include/curl/
  ${SRC_FOLDER}/external/libpcre2/include/
  ${SRC_FOLDER}/external/libyaml/include
  ${SRC_FOLDER}/external/libplist/bin/include/
  ${SRC_FOLDER}/external/cJSON/
  ${SRC_FOLDER}/external/bzip2/
  ${SRC_FOLDER}/external/sqlite/
  ${SRC_FOLDER}/external/nlohmann/
  ${SRC_FOLDER}/external/openssl/include/
  ${SRC_FOLDER}/external/zlib/
  ${SRC_FOLDER}/external/rocksdb/include
  ${SRC_FOLDER}/external/simdjson/include
  ${SRC_FOLDER}/client-agent/include
  ${SRC_FOLDER}/config/include
  ${SRC_FOLDER}/shared/include/error_messages
  ${SRC_FOLDER}/shared/os_crypto/md5
  ${SRC_FOLDER}/shared/os_crypto/md5_sha1_sha256
  ${SRC_FOLDER}/shared/os_crypto/sha1
  ${SRC_FOLDER}/shared/os_crypto/sha256
  ${SRC_FOLDER}/shared/os_crypto/signature
  ${SRC_FOLDER}/shared/os_net
  ${SRC_FOLDER}/shared/os_xml
  ${SRC_FOLDER}/data_provider/include
  ${SRC_FOLDER}/shared_modules/
  ${SRC_FOLDER}/shared_modules/agent_metadata/include
  ${SRC_FOLDER}/shared_modules/common/
  ${SRC_FOLDER}/shared_modules/content_manager/include
  ${SRC_FOLDER}/shared_modules/dbsync/include
  ${SRC_FOLDER}/shared_modules/router/include
  ${SRC_FOLDER}/shared_modules/sync_protocol/include
  ${SRC_FOLDER}/syscheckd/include
  ${SRC_FOLDER}/guardsarm_db/include)

# Submodule include directories
include_directories(
  ${SRC_FOLDER}/guardsarm_modules/include
  ${SRC_FOLDER}/guardsarm_modules/src
  ${SRC_FOLDER}/guardsarm_modules/agent_info/include
  ${SRC_FOLDER}/guardsarm_modules/agent_info/agent_info_impl/include
  ${SRC_FOLDER}/guardsarm_modules/sca/include
  ${SRC_FOLDER}/guardsarm_modules/sca/sca_impl/include
  ${SRC_FOLDER}/guardsarm_modules/syscollector/include
  ${SRC_FOLDER}/guardsarm_modules/vulnerability_scanner/include
  ${SRC_FOLDER}/guardsarm_modules/inventory_sync/include
  ${SRC_FOLDER}/guardsarm_modules/src/agent_upgrade
  ${SRC_FOLDER}/guardsarm_modules/src/agent_upgrade/agent
  ${SRC_FOLDER}/guardsarm_modules/src/agent_upgrade/manager
  ${SRC_FOLDER}/guardsarm_modules/src/task_manager)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Keep agent daemon name for agent/winagent targets and manager daemon name for
# server targets.
set(GUARDSARM_MODULESD_DAEMON_NAME "guardsarm-modulesd")
set(GUARDSARM_RUNTIME_GROUP "guardsarm")
if(NOT "${TARGET}" STREQUAL "winagent" AND NOT "${TARGET}" STREQUAL "agent")
  set(GUARDSARM_MODULESD_DAEMON_NAME "guardsarm-manager-modulesd")
  set(GUARDSARM_RUNTIME_GROUP "guardsarm-manager")
endif()
add_definitions(-DARGV0="${GUARDSARM_MODULESD_DAEMON_NAME}")
add_definitions(-DGROUPGLOBAL="${GUARDSARM_RUNTIME_GROUP}")

if(INOTIFY_ENABLED)
  add_definitions(-DINOTIFY_ENABLED)
endif(INOTIFY_ENABLED)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_definitions(-DWIN32=1 -DWIN_EXPORT)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  add_definitions(-DDarwin -DHIGHFIRST)
endif()

set(LIB_DIR ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR})

if(NOT "${TARGET}" STREQUAL "winagent" AND NOT "${TARGET}" STREQUAL "agent")
  # Modules - Server only
  add_subdirectory(inventory_sync)
  add_subdirectory(vulnerability_scanner)
else()
  # Modules - Agent only
  add_subdirectory(agent_info)
  add_subdirectory(sca)
  add_subdirectory(syscollector)
endif()

if(NOT "${TARGET}" STREQUAL "winagent" AND NOT "${TARGET}" STREQUAL "agent")
  file(
    GLOB
    GUARDSARM_MODULES_SRC
    "${SRC_FOLDER}/guardsarm_modules/src/*.c"
    "${SRC_FOLDER}/guardsarm_modules/src/agent_upgrade/*.c"
    "${SRC_FOLDER}/guardsarm_modules/src/agent_upgrade/manager/*.c"
    "${SRC_FOLDER}/guardsarm_modules/src/task_manager/*.c")
else()
  file(GLOB GUARDSARM_MODULES_SRC "${SRC_FOLDER}/guardsarm_modules/src/*.c"
       "${SRC_FOLDER}/guardsarm_modules/src/agent_upgrade/*.c"
       "${SRC_FOLDER}/guardsarm_modules/src/agent_upgrade/agent/*.c")
endif()

# Remove agent-only modules from server build
if(NOT "${TARGET}" STREQUAL "winagent" AND NOT "${TARGET}" STREQUAL "agent")
  list(REMOVE_ITEM GUARDSARM_MODULES_SRC "${SRC_FOLDER}/guardsarm_modules/src/wm_sca.c")
endif()

# Build static library (without main.c)
list(REMOVE_ITEM GUARDSARM_MODULES_SRC "${SRC_FOLDER}/guardsarm_modules/src/main.c")
add_library(guardsarm_modulesd_lib STATIC ${GUARDSARM_MODULES_SRC})
set_target_properties(guardsarm_modulesd_lib PROPERTIES LINKER_LANGUAGE C)

# Enable GUARDSARM_UNIT_TESTING to export static functions for unit tests
if(UNIT_TEST)
  target_compile_definitions(guardsarm_modulesd_lib PRIVATE GUARDSARM_UNIT_TESTING)
endif()

if(NOT "${TARGET}" STREQUAL "winagent" AND NOT "${TARGET}" STREQUAL "agent")
  # Server build
  target_link_libraries(
    guardsarm_modulesd_lib
    PUBLIC inventory_sync
           vulnerability_scanner
           guardsarmext
           pthread
           config
           guardsarm
           schema_validator)

  # Link jemalloc for server builds (unless explicitly disabled)
  if(NOT DEFINED DISABLE_JEMALLOC OR NOT DISABLE_JEMALLOC)
    find_library(
      JEMALLOC_LIB jemalloc
      PATHS ${SRC_FOLDER}/external/jemalloc/lib
      NO_DEFAULT_PATH)
    if(JEMALLOC_LIB)
      target_link_libraries(guardsarm_modulesd_lib PUBLIC ${JEMALLOC_LIB})
    endif()
  endif()
else()
  # Agent build
  target_link_libraries(
    guardsarm_modulesd_lib
    PUBLIC agent_info
           sca
           syscollector
           sysinfo
           guardsarmext
           pthread
           config
           guardsarm
           agent_sync_protocol
           dbsync
           schema_validator)
endif()

if(UNIX)
  target_link_libraries(guardsarm_modulesd_lib PUBLIC dl)
  if(NOT APPLE)
    target_link_libraries(guardsarm_modulesd_lib PUBLIC rt)
  endif(NOT APPLE)
endif(UNIX)

# Build Windows libraries always (needed by Makefile for final executables)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_library(guardsarm-modulesd STATIC ${GUARDSARM_MODULES_SRC})
  target_compile_definitions(guardsarm-modulesd PUBLIC -D_WIN32_WINNT=0x600)
  if(UNIT_TEST)
    target_compile_definitions(guardsarm-modulesd PRIVATE GUARDSARM_UNIT_TESTING)
  endif()
  target_link_libraries(
    guardsarm-modulesd
    PRIVATE agent_info
            sca
            syscollector
            sysinfo
            guardsarmext
            pthread
            config
            guardsarm
            agent_sync_protocol
            dbsync
            schema_validator)
  # Only build Linux executable when not running unit tests
elseif(NOT UNIT_TEST)
  add_executable(${GUARDSARM_MODULESD_DAEMON_NAME}
                 "${SRC_FOLDER}/guardsarm_modules/src/main.c")
  set_target_properties(
    ${GUARDSARM_MODULESD_DAEMON_NAME}
    PROPERTIES LINKER_LANGUAGE C RUNTIME_OUTPUT_DIRECTORY
                                 ${SRC_FOLDER}/build/bin)
  target_link_libraries(${GUARDSARM_MODULESD_DAEMON_NAME}
                        PRIVATE guardsarm_modulesd_lib)

  guardsarm_set_runtime_rpath(${GUARDSARM_MODULESD_DAEMON_NAME})
endif()
