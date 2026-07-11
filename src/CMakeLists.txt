cmake_minimum_required(VERSION 3.12.4)

# Force ranlib and windres for scan-build cross-compilation compatibility Must
# be set BEFORE project() command to take effect
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  # Set the correct cross-compilation tools
  set(CMAKE_RANLIB
      "i686-w64-mingw32-ranlib"
      CACHE FILEPATH "Ranlib for Windows cross-compilation" FORCE)
  set(CMAKE_RC_COMPILER
      "i686-w64-mingw32-windres"
      CACHE FILEPATH "Resource compiler for Windows cross-compilation" FORCE)
  set(CMAKE_C_ARCHIVE_FINISH "<CMAKE_RANLIB> <TARGET>")
  set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> <TARGET>")
endif()

project(GuardSarm)

enable_testing()

if(NOT CMAKE_BUILD_TYPE)
  if(CMAKE_SYMBOLS_IN_RELEASE MATCHES "ON")
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
  else()
    set(CMAKE_BUILD_TYPE Release)
  endif()
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Modern Linux toolchains (el10 / gcc 14) default to PIE at link; some test/util tools
# don't build their objects position-independent, which fails the PIE link
# ("relocation R_X86_64_32S ... can not be used when making a PIE object"). Compile all
# Linux objects position-independent so the default-PIE link succeeds. Windows (MinGW)
# is left alone - its DLL targets set POSITION_INDEPENDENT_CODE 0 to avoid a warning.
if(UNIX AND NOT APPLE)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Coverity static analysis compatibility
if(COVERITY)
  add_definitions(-D__GNUC__=8)
endif(COVERITY)

# Compiler flags for Debug and Release builds
if(NOT FSANITIZE)
  set(CMAKE_CXX_FLAGS_DEBUG "-g")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
else()
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -s")
endif()

# Sanitizer flags (override DEBUG flags when enabled)
if(FSANITIZE)
  set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address,leak,undefined")
endif(FSANITIZE)

# Function to link coverage libraries for unit tests Uses plain signature for
# compatibility with existing plain signature usage
function(add_coverage_libraries target_name)
  if(UNIT_TEST)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      target_compile_options(${target_name} PUBLIC --coverage)
      target_link_libraries(${target_name} PUBLIC -fprofile-arcs)
    else()
      target_compile_options(${target_name} PUBLIC --coverage)
      target_link_libraries(${target_name} PUBLIC gcov)
    endif()
  endif()
endfunction()

# SRC_FOLDER should point to the src/ directory Sub-modules will try to redefine
# it, so we set a guard variable
set(SRC_FOLDER ${CMAKE_SOURCE_DIR})
set(GUARDSARM_MAIN_PROJECT
    ON
    CACHE BOOL "Main GuardSarm CMake project" FORCE)

# Shared CMake helpers
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(GuardSarmRpath)

# Define CLIENT for agent builds
if("${TARGET}" STREQUAL "winagent" OR "${TARGET}" STREQUAL "agent")
  add_definitions(-DCLIENT)
endif()

# Keep Make-based Windows semantics for external headers that otherwise declare
# symbols as dllimport. We link these through delay import stubs.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_definitions(-DPCRE2_EXP_DECL= -DBUILDING_LIBCURL=)

  # Add system include paths for scan-build cross-compilation compatibility
  # These paths are needed when using clang static analyzer wrappers with mingw
  set(MINGW_SYSTEM_INCLUDES
      "-isystem/usr/lib/gcc/i686-w64-mingw32/13-posix/include/c++"
      "-isystem/usr/lib/gcc/i686-w64-mingw32/13-posix/include/c++/i686-w64-mingw32"
      "-isystem/usr/lib/gcc/i686-w64-mingw32/13-posix/include/c++/backward"
      "-isystem/usr/lib/gcc/i686-w64-mingw32/13-posix/include"
      "-isystem/usr/lib/gcc/i686-w64-mingw32/13-posix/include-fixed"
      "-isystem/usr/lib/gcc/i686-w64-mingw32/13-posix/../../../../i686-w64-mingw32/include"
  )

  # Add flags directly to compiler to work around scan-build issues with
  # response files
  add_compile_options(${MINGW_SYSTEM_INCLUDES})

  # The mingw g++ cross-compiler driver on this toolchain does NOT append the C++
  # runtime (libstdc++) to Windows shared-library (-shared) links, so C++ modules
  # that use std::thread / std::condition_variable (agent_info.dll, sca.dll, ...)
  # fail at link with "undefined reference to std::condition_variable::...". The
  # original build only linked by accident because its CI image shipped a gcc whose
  # driver did add it. Resolve it deterministically: append the compiler's own
  # static libstdc++.a to CMAKE_CXX_STANDARD_LIBRARIES, which CMake always emits at
  # the very end of every link line (correct static-link order, after the module
  # archives that need it). Using the static .a also drops the external
  # libstdc++-6.dll runtime dependency from the distributed agent. libgcc / winpthread
  # are already supplied by the driver (-pthread) and other module import libs, so
  # adding them here would duplicate unwind symbols (_Unwind_Resume) — do not.
  execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libstdc++.a
    OUTPUT_VARIABLE GUARDSARM_LIBSTDCXX_A
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(EXISTS "${GUARDSARM_LIBSTDCXX_A}")
    string(APPEND CMAKE_CXX_STANDARD_LIBRARIES " ${GUARDSARM_LIBSTDCXX_A}")
    message(STATUS "GuardSarm: appended static libstdc++ to link: ${GUARDSARM_LIBSTDCXX_A}")
  endif()

  # Disable response files for scan-build compatibility
  set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES OFF)
  set(CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES OFF)
endif()
# Set Wine as crosscompiling emulator for Windows unit tests
if(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND UNIT_TEST)
  find_program(WINE "wine")
  if(NOT WINE)
    message(WARNING "Wine is required to run Windows tests on Linux")
  endif()
  set(CMAKE_CROSSCOMPILING_EMULATOR wine)
endif()

# Read the VERSION JSON file
file(READ "${SRC_FOLDER}/../VERSION.json" VERSION_JSON)

# Extract "version"
string(REGEX MATCH "\"version\"[ \t]*:[ \t]*\"([^\"]+)\"" _ ${VERSION_JSON})
set(VERSION_FILE "v${CMAKE_MATCH_1}") # <-- Add "v" prefix since old VERSION
                                      # file had it

# Extract "stage" as revision
string(REGEX MATCH "\"stage\"[ \t]*:[ \t]*\"([^\"]+)\"" _ ${VERSION_JSON})
set(REVISION_FILE "${CMAKE_MATCH_1}")

# Define preprocessor macros
add_definitions(-DVERSION="${VERSION_FILE}")
add_definitions(-DREVISION="${REVISION_FILE}")

# If REVISION OR FILE is empty fail
if("${REVISION_FILE}" STREQUAL "")
  message(FATAL_ERROR "REVISION file is empty")
endif("${REVISION_FILE}" STREQUAL "")

if("${VERSION_FILE}" STREQUAL "")
  message(FATAL_ERROR "VERSION file is empty")
endif("${VERSION_FILE}" STREQUAL "")

message("==============================================")
message("GuardSarm version: ${VERSION_FILE}")
message("GuardSarm revision: ${REVISION_FILE}")
message("==============================================")

# Read the GuardSarm branding JSON file (display-only, user-visible values).
# Mirrors the VERSION.json parse above. Values are injected as PRODUCT_* macros
# consumed ONLY by user-visible surfaces (see src/shared/include/branding.h).
# If branding.json is absent, the branding.h fallbacks reproduce GuardSarm branding,
# so removing branding.json is a complete rollback of the user-visible rebrand.
# NOTE: branding.json intentionally carries NO paths, users/groups, daemon names,
# ports, index names, or protocol constants -- those are frozen elsewhere.
set(BRANDING_JSON_PATH "${SRC_FOLDER}/../branding.json")
if(EXISTS "${BRANDING_JSON_PATH}")
  file(READ "${BRANDING_JSON_PATH}" BRANDING_JSON)

  string(REGEX MATCH "\"product_name\"[ \t]*:[ \t]*\"([^\"]+)\"" _ ${BRANDING_JSON})
  set(PRODUCT_NAME_FILE "${CMAKE_MATCH_1}")
  string(REGEX MATCH "\"product_version\"[ \t]*:[ \t]*\"([^\"]+)\"" _ ${BRANDING_JSON})
  set(PRODUCT_VERSION_FILE "${CMAKE_MATCH_1}")
  string(REGEX MATCH "\"author\"[ \t]*:[ \t]*\"([^\"]+)\"" _ ${BRANDING_JSON})
  set(PRODUCT_AUTHOR_FILE "${CMAKE_MATCH_1}")
  string(REGEX MATCH "\"contact\"[ \t]*:[ \t]*\"([^\"]+)\"" _ ${BRANDING_JSON})
  set(PRODUCT_CONTACT_FILE "${CMAKE_MATCH_1}")
  string(REGEX MATCH "\"site\"[ \t]*:[ \t]*\"([^\"]+)\"" _ ${BRANDING_JSON})
  set(PRODUCT_SITE_FILE "${CMAKE_MATCH_1}")

  if(NOT "${PRODUCT_NAME_FILE}" STREQUAL "")
    add_definitions(-DPRODUCT_NAME="${PRODUCT_NAME_FILE}")
  endif()
  if(NOT "${PRODUCT_VERSION_FILE}" STREQUAL "")
    add_definitions(-DPRODUCT_VERSION="${PRODUCT_VERSION_FILE}")
  endif()
  if(NOT "${PRODUCT_AUTHOR_FILE}" STREQUAL "")
    add_definitions(-DPRODUCT_AUTHOR="${PRODUCT_AUTHOR_FILE}")
  endif()
  if(NOT "${PRODUCT_CONTACT_FILE}" STREQUAL "")
    add_definitions(-DPRODUCT_CONTACT="${PRODUCT_CONTACT_FILE}")
  endif()
  if(NOT "${PRODUCT_SITE_FILE}" STREQUAL "")
    add_definitions(-DPRODUCT_SITE="${PRODUCT_SITE_FILE}")
  endif()

  message("==============================================")
  message("GuardSarm branding: ${PRODUCT_NAME_FILE} ${PRODUCT_VERSION_FILE}")
  message("==============================================")
endif()

set(BENCHMARK_ENABLE_TESTING "OFF")

link_directories(${SRC_FOLDER})
link_directories(${SRC_FOLDER}/build/lib)

# GuardSarm headers
include_directories(${SRC_FOLDER}/shared/include)

# Shared modules includes
include_directories(${SRC_FOLDER}/shared_modules/agent_metadata/include)
include_directories(${SRC_FOLDER}/shared_modules/common)
include_directories(${SRC_FOLDER}/shared_modules/content_manager/include)
include_directories(${SRC_FOLDER}/shared_modules/dbsync/include)
include_directories(${SRC_FOLDER}/shared_modules/file_helper/file_io/include)
include_directories(${SRC_FOLDER}/shared_modules/file_helper/filesystem/include)
include_directories(${SRC_FOLDER}/shared_modules/http-request/include)
include_directories(${SRC_FOLDER}/shared_modules/indexer_connector/include)
include_directories(${SRC_FOLDER}/shared_modules/keystore/include)
include_directories(${SRC_FOLDER}/shared_modules/router/include)
include_directories(${SRC_FOLDER}/shared_modules/schema_validator/include)
include_directories(${SRC_FOLDER}/shared_modules/sync_protocol/include)
include_directories(${SRC_FOLDER}/shared_modules/utils)

# Windows version resource objects
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  enable_language(RC)

  # Version resource for DLLs (VFT_DLL)
  add_library(version_rc_dll OBJECT ${SRC_FOLDER}/win32/version.rc)
  target_compile_definitions(version_rc_dll PRIVATE VER_TYPE=VFT_DLL)

  # Version resource for executables (VFT_APP)
  add_library(version_rc_app OBJECT ${SRC_FOLDER}/win32/version.rc)
  target_compile_definitions(version_rc_app PRIVATE VER_TYPE=VFT_APP)

  set(RESOURCE_OBJ_DLL $<TARGET_OBJECTS:version_rc_dll>)
  set(RESOURCE_OBJ_APP $<TARGET_OBJECTS:version_rc_app>)

  find_program(MINGW_AR i686-w64-mingw32-ar REQUIRED)
  find_library(
    WIN_PTHREAD_STATIC_LIB
    NAMES winpthread
    PATHS /usr/i686-w64-mingw32/lib /usr/i686-w64-mingw32/sys-root/mingw/lib
          REQUIRED)
  find_file(
    WIN_PTHREAD_DLL
    NAMES libwinpthread-1.dll
    PATHS /usr/i686-w64-mingw32/lib /usr/i686-w64-mingw32/sys-root/mingw/bin
          REQUIRED)

  set(PATCHED_PTHREAD_LIB ${SRC_FOLDER}/build/lib/libwinpthreadpatched.a)
  set(WIN_PTHREAD_DLL_COPY ${SRC_FOLDER}/build/lib/libwinpthread-1.dll)
  add_custom_command(
    OUTPUT ${PATCHED_PTHREAD_LIB}
    COMMAND ${CMAKE_COMMAND} -E copy ${WIN_PTHREAD_STATIC_LIB}
            ${PATCHED_PTHREAD_LIB}
    COMMAND ${MINGW_AR} d ${PATCHED_PTHREAD_LIB} version.o
    DEPENDS ${WIN_PTHREAD_STATIC_LIB}
    COMMENT "Creating patched pthread library")
  add_custom_command(
    OUTPUT ${WIN_PTHREAD_DLL_COPY}
    COMMAND ${CMAKE_COMMAND} -E copy ${WIN_PTHREAD_DLL} ${WIN_PTHREAD_DLL_COPY}
    DEPENDS ${WIN_PTHREAD_DLL}
    COMMENT "Copying pthread DLL")
  add_custom_target(winpthreadpatched ALL DEPENDS ${PATCHED_PTHREAD_LIB}
                                                  ${WIN_PTHREAD_DLL_COPY})

  # Common Windows libraries that all Windows executables need
  set(GUARDSARM_WINDOWS_LIBS
      ${SRC_FOLDER}/build/lib/libguardsarmext.lib ${PATCHED_PTHREAD_LIB} wintrust
      psapi crypt32 shlwapi
      CACHE INTERNAL "Common Windows libraries for all executables")
endif()

# Helper variables
set(IS_AGENT FALSE)
if("${TARGET}" STREQUAL "winagent" OR "${TARGET}" STREQUAL "agent")
  set(IS_AGENT TRUE)
endif()

# External dependencies
add_subdirectory(external)

# Common modules (all targets)
add_subdirectory(config)
add_subdirectory(shared)
add_subdirectory(shared_modules/file_helper)
set(CURL_DEP guardsarmext)
add_subdirectory(shared_modules/http-request)
add_subdirectory(shared_modules/schema_validator)
add_subdirectory(shared_modules/utils)
add_subdirectory(guardsarm_modules)

# Agent-only modules
if(IS_AGENT)
  add_subdirectory(active-response)
  add_subdirectory(addagent)
  add_subdirectory(client-agent)
  add_subdirectory(data_provider)
  add_subdirectory(logcollector)
  add_subdirectory(os_execd)
  add_subdirectory(rootcheck)
  add_subdirectory(shared_modules/agent_metadata)
  add_subdirectory(shared_modules/dbsync)
  add_subdirectory(shared_modules/sync_protocol)
  add_subdirectory(syscheckd)
  # Windows agent executables and resources
  if("${TARGET}" STREQUAL "winagent")
    add_subdirectory(win32)
  endif()
endif()

# Server-only modules
if(NOT IS_AGENT)
  add_subdirectory(engine)
  add_subdirectory(monitord)
  add_subdirectory(os_auth)
  add_subdirectory(remoted)
  add_subdirectory(shared_modules/content_manager)
  add_subdirectory(shared_modules/indexer_connector)
  add_subdirectory(shared_modules/keystore)
  add_subdirectory(shared_modules/router)
  add_subdirectory(util)
  add_subdirectory(guardsarm_db)
endif()

# Create flatbuffers alias if the real target wasn't created by subdirectories
if(NOT TARGET flatbuffers)
  add_library(flatbuffers ALIAS ext_flatbuffers)
endif()
