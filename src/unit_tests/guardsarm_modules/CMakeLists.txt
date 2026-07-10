# Copyright (C) 2026 GuardSarm, Inc.
#
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.) as published by the
# FSF - Free Software Foundation.

cmake_minimum_required(VERSION 3.10)

if(${TARGET} STREQUAL "manager")
  add_subdirectory(database)
  add_subdirectory(scheduling)
  add_subdirectory(vulnerability_detection)
  add_subdirectory(task_manager)
  add_subdirectory(agent_upgrade)
  add_subdirectory(wmodules)
endif()

if(${TARGET} STREQUAL "agent")
  add_subdirectory(aws)
  add_subdirectory(azure)
  add_subdirectory(command)
  add_subdirectory(docker)
  add_subdirectory(gcp)
  add_subdirectory(github)
  add_subdirectory(office365)
  add_subdirectory(ms_graph)
  add_subdirectory(agent_upgrade)
endif()

if(${TARGET} STREQUAL "winagent")
  # The agent-side flow of agent_upgrade is built into the winagent binary
  # (src/guardsarm_modules/CMakeLists.txt:111-112 picks up agent_upgrade/*.c and
  # agent_upgrade/agent/*.c for non-server builds). The cmocka tests were
  # already authored with #ifdef TEST_WINAGENT guards in
  # test_wm_agent_upgrade_agent.c and winagent.cmake defines TEST_WINAGENT,
  # so wiring the subdirectory here is enough to compile/run them.
  add_subdirectory(agent_upgrade)
endif()

if(${TARGET} STREQUAL "manager")
  add_subdirectory(control)
endif()

add_subdirectory(exec)
