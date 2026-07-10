/*
 * SQL Schema rbac tests
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

-- PRAGMA foreign_keys=OFF;
-- BEGIN TRANSACTION;

/* Testing */
DELETE FROM users WHERE username = 'test_user';
