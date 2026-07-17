cmake_minimum_required(VERSION 3.12.4)

project(guardsarm-active-response C)

if(NOT DEFINED GUARDSARM_MAIN_PROJECT)
  get_filename_component(SRC_FOLDER ${CMAKE_SOURCE_DIR}/../ ABSOLUTE)
endif()

# Skip building active responses during unit tests
if(UNIT_TEST)
  return()
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${SRC_FOLDER}/config/include)
include_directories(${SRC_FOLDER}/shared/include/error_messages)
include_directories(${SRC_FOLDER}/shared/include)
include_directories(${SRC_FOLDER}/shared/os_regex)
include_directories(${SRC_FOLDER}/shared/os_crypto/sha1)
include_directories(${SRC_FOLDER}/shared/os_xml)
include_directories(${SRC_FOLDER}/external/cJSON)
include_directories(${SRC_FOLDER}/external/curl/include/curl/)
include_directories(${SRC_FOLDER}/external/libpcre2/include/)
include_directories(${SRC_FOLDER}/external/libyaml/include)
include_directories(${SRC_FOLDER}/external/bzip2/)
include_directories(${SRC_FOLDER}/external/sqlite/)

# Platform-specific definitions
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_definitions(-DWIN32=1 -D_POSIX_C_SOURCE -DPSAPI_VERSION=1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  add_definitions(-DDarwin -DHIGHFIRST)
endif()

# Define function to create active response executable for Unix-like systems
function(add_active_response_executable name source)
  add_executable(${name} ${source})

  # Set ARGV0 definition for each binary
  target_compile_definitions(${name} PRIVATE ARGV0="${name}")

  # Set output directory to src/build/bin
  set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                           ${SRC_FOLDER}/build/bin OUTPUT_NAME ${name})

  # Link with libguardsarm.a
  target_link_libraries(${name} PRIVATE guardsarm)

  # Platform-specific linker flags
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Linux: Set runtime library path and link system libraries
    set_target_properties(${name} PROPERTIES
      INSTALL_RPATH "$ORIGIN/../../lib"
      BUILD_WITH_INSTALL_RPATH TRUE)
    target_link_options(${name} PRIVATE -pthread)

    # Add system libraries
    target_link_libraries(${name} PRIVATE pthread rt dl m)

  elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # macOS: Add rpath for @executable_path/../../lib, plus pthread
    target_link_options(${name} PRIVATE -pthread)
    set_target_properties(
      ${name} PROPERTIES INSTALL_RPATH "@executable_path/../../lib"
                         BUILD_WITH_INSTALL_RPATH TRUE)

    # Add macOS frameworks
    target_link_libraries(
      ${name} PRIVATE "-framework Security" "-framework CoreFoundation"
                      "-framework SystemConfiguration")
  endif()
endfunction()

# Build Unix/Linux/BSD unified block-ip active response
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  # Unified block-ip for Linux, FreeBSD, OpenBSD, NetBSD
  add_active_response_executable(
    block-ip
    ${CMAKE_CURRENT_SOURCE_DIR}/src/block-ip-unix.c)

  # Add helpers for firewall operations
  target_sources(block-ip PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/helpers/firewall_helpers.c)
  target_include_directories(block-ip PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/helpers)

  # Platform-specific compile definitions
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(block-ip PRIVATE PLATFORM_LINUX=1)
  elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    target_compile_definitions(block-ip PRIVATE PLATFORM_FREEBSD=1)
  elseif(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
    target_compile_definitions(block-ip PRIVATE PLATFORM_OPENBSD=1)
  elseif(CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
    target_compile_definitions(block-ip PRIVATE PLATFORM_NETBSD=1)
  endif()

  # Build other Unix/Linux/BSD active response programs
  add_active_response_executable(
    disable-account ${CMAKE_CURRENT_SOURCE_DIR}/src/disable-account.c)

  # GuardSarm Response 2.0 active responses
  add_active_response_executable(kill-process ${CMAKE_CURRENT_SOURCE_DIR}/src/kill-process.c)
  add_active_response_executable(quarantine-file ${CMAKE_CURRENT_SOURCE_DIR}/src/quarantine-file.c)
  add_active_response_executable(block-domain ${CMAKE_CURRENT_SOURCE_DIR}/src/block-domain.c)
  add_active_response_executable(block-hash ${CMAKE_CURRENT_SOURCE_DIR}/src/block-hash.c)
  add_active_response_executable(isolate ${CMAKE_CURRENT_SOURCE_DIR}/src/isolate.c)
  add_active_response_executable(remove-persistence ${CMAKE_CURRENT_SOURCE_DIR}/src/remove-persistence.c)
  add_active_response_executable(collect-forensic ${CMAKE_CURRENT_SOURCE_DIR}/src/collect-forensic.c)
endif()

# Build macOS-specific block-ip active response
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  # macOS uses separate block-ip binary with pf only
  add_active_response_executable(
    block-ip
    ${CMAKE_CURRENT_SOURCE_DIR}/src/block-ip-macos.c)

  # Add helpers for firewall operations
  target_sources(block-ip PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/helpers/firewall_helpers.c)
  target_include_directories(block-ip PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/helpers)

  target_compile_definitions(block-ip PRIVATE PLATFORM_MACOS=1)

  # Build other macOS active response programs
  add_active_response_executable(
    disable-account ${CMAKE_CURRENT_SOURCE_DIR}/src/disable-account.c)

  # GuardSarm Response 2.0 active responses
  add_active_response_executable(kill-process ${CMAKE_CURRENT_SOURCE_DIR}/src/kill-process.c)
  add_active_response_executable(quarantine-file ${CMAKE_CURRENT_SOURCE_DIR}/src/quarantine-file.c)
  add_active_response_executable(block-domain ${CMAKE_CURRENT_SOURCE_DIR}/src/block-domain.c)
  add_active_response_executable(block-hash ${CMAKE_CURRENT_SOURCE_DIR}/src/block-hash.c)
  add_active_response_executable(isolate ${CMAKE_CURRENT_SOURCE_DIR}/src/isolate.c)
  add_active_response_executable(remove-persistence ${CMAKE_CURRENT_SOURCE_DIR}/src/remove-persistence.c)
  add_active_response_executable(collect-forensic ${CMAKE_CURRENT_SOURCE_DIR}/src/collect-forensic.c)
endif()

# Build Windows active responses
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  # Windows active responses need win32_stubs and resource files (.rc)

  # Unified block-ip.exe (netsh + route fallback)
  add_executable(
    block-ip
    ${CMAKE_CURRENT_SOURCE_DIR}/src/block-ip-windows.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/helpers/firewall_helpers.c
    $<TARGET_OBJECTS:win32_stubs>
    ${SRC_FOLDER}/win32/block-ip.rc
    ${RESOURCE_OBJ_APP})
  target_compile_definitions(block-ip PRIVATE ARGV0="block-ip" PLATFORM_WINDOWS=1)
  target_include_directories(block-ip PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/helpers)
  set_target_properties(
    block-ip PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${SRC_FOLDER}/build/bin
                       OUTPUT_NAME block-ip)
  target_link_libraries(block-ip PRIVATE guardsarm ${GUARDSARM_WINDOWS_LIBS})

  # GuardSarm Response 2.0 active responses (Windows).
  function(add_windows_ar name source)
    add_executable(${name} ${source} ${ARGN} $<TARGET_OBJECTS:win32_stubs> ${RESOURCE_OBJ_APP})
    target_compile_definitions(${name} PRIVATE ARGV0="${name}" PLATFORM_WINDOWS=1)
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${SRC_FOLDER}/build/bin
                         OUTPUT_NAME ${name})
    target_link_libraries(${name} PRIVATE guardsarm ${GUARDSARM_WINDOWS_LIBS})
  endfunction()
  add_windows_ar(kill-process ${CMAKE_CURRENT_SOURCE_DIR}/src/kill-process.c)
  add_windows_ar(quarantine-file ${CMAKE_CURRENT_SOURCE_DIR}/src/quarantine-file.c)
  add_windows_ar(block-domain ${CMAKE_CURRENT_SOURCE_DIR}/src/block-domain.c)
  add_windows_ar(block-hash ${CMAKE_CURRENT_SOURCE_DIR}/src/block-hash.c)
  add_windows_ar(remove-persistence ${CMAKE_CURRENT_SOURCE_DIR}/src/remove-persistence.c)
  add_windows_ar(collect-forensic ${CMAKE_CURRENT_SOURCE_DIR}/src/collect-forensic.c)

  # Endpoint isolation (WFP) + the standalone network-repair / recovery watchdog.
  # Both compile the WFP engine and link the Windows Filtering Platform user-mode
  # client (fwpuclnt) + Winsock (ws2_32) for connectivity probing/allow-list parse.
  add_windows_ar(isolate
    ${CMAKE_CURRENT_SOURCE_DIR}/src/isolate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/helpers/wfp_isolation.c)
  target_link_libraries(isolate PRIVATE fwpuclnt ws2_32 iphlpapi)

  add_windows_ar(repair-network
    ${CMAKE_CURRENT_SOURCE_DIR}/src/repair-network.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/helpers/wfp_isolation.c)
  target_link_libraries(repair-network PRIVATE fwpuclnt ws2_32 iphlpapi advapi32)

  # Agent tamper-protection guardian (user-mode resilience tier). Independent SYSTEM
  # watchdog: service resilience + binary/config self-heal + fail-open uninstall.
  # advapi32 provides the SCM + CryptoAPI (SHA-256) entry points it uses.
  add_windows_ar(guardsarm-tamper-guard
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tamper-guard.c)
  target_link_libraries(guardsarm-tamper-guard PRIVATE advapi32)
endif()
