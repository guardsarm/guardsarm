/*
 * GuardSarm Module for Agent Upgrading
 * Copyright (C) 2026 GuardSarm, Inc.
 * October 19, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef GM_AGENT_UPGRADE_TASKS_H
#define GM_AGENT_UPGRADE_TASKS_H

#include "wm_task_manager.h"

/**
 * Initialization of wm_task_manager_upgrade
 * @param return an initialized wm_task_manager_upgrade structure
 * */
gm_task_manager_upgrade* gm_task_manager_init_upgrade_parameters();

/**
 * Initialization of wm_task_manager_upgrade_get_status
 * @param return an initialized wm_task_manager_upgrade_get_status structure
 * */
gm_task_manager_upgrade_get_status* gm_task_manager_init_upgrade_get_status_parameters();

/**
 * Initialization of wm_task_manager_upgrade_update_status
 * @param return an initialized wm_task_manager_upgrade_update_status structure
 * */
gm_task_manager_upgrade_update_status* gm_task_manager_init_upgrade_update_status_parameters();

/**
 * Initialization of wm_task_manager_upgrade_result
 * @param return an initialized wm_task_manager_upgrade_result structure
 * */
gm_task_manager_upgrade_result* gm_task_manager_init_upgrade_result_parameters();

/**
 * Initialization of wm_task_manager_upgrade_cancel_tasks
 * @param return an initialized wm_task_manager_upgrade_cancel_tasks structure
 * */
gm_task_manager_upgrade_cancel_tasks* gm_task_manager_init_upgrade_cancel_tasks_parameters();

/**
 * Initialization of wm_task_manager_task
 * @param return an initialized wm_task_manager_task structure
 * */
gm_task_manager_task* gm_task_manager_init_task();

/**
 * Deallocate wm_task_manager_upgrade structure
 * @param parameters wm_task_manager_upgrade structure to be deallocated
 * */
void gm_task_manager_free_upgrade_parameters(gm_task_manager_upgrade* parameters);

/**
 * Deallocate wm_task_manager_upgrade_get_status structure
 * @param parameters wm_task_manager_upgrade_get_status structure to be deallocated
 * */
void gm_task_manager_free_upgrade_get_status_parameters(gm_task_manager_upgrade_get_status* parameters);

/**
 * Deallocate wm_task_manager_upgrade_update_status structure
 * @param parameters wm_task_manager_upgrade_update_status structure to be deallocated
 * */
void gm_task_manager_free_upgrade_update_status_parameters(gm_task_manager_upgrade_update_status* parameters);

/**
 * Deallocate wm_task_manager_upgrade_result structure
 * @param parameters wm_task_manager_upgrade_result structure to be deallocated
 * */
void gm_task_manager_free_upgrade_result_parameters(gm_task_manager_upgrade_result* parameters);

/**
 * Deallocate wm_task_manager_upgrade_cancel_tasks structure
 * @param parameters wm_task_manager_upgrade_cancel_tasks structure to be deallocated
 * */
void gm_task_manager_free_upgrade_cancel_tasks_parameters(gm_task_manager_upgrade_cancel_tasks* parameters);

/**
 * Deallocate wm_task_manager_task structure
 * @param task wm_task_manager_task structure to be deallocated
 * */
void gm_task_manager_free_task(gm_task_manager_task* task);

#endif
