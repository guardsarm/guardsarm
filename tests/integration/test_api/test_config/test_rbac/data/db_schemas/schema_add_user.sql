/*
 * SQL Schema rbac tests
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

-- PRAGMA foreign_keys=OFF;
-- BEGIN TRANSACTION;

/* Testing */
INSERT INTO users VALUES('1000', 'test_user', 'pbkdf2:sha256:150000$QWFBPDUI$52f5eda84138dab4b5dd3bd3ddd1b34180abde1a68c1e70aa8825640c7660790', 0, '2020-04-27 09:02:52.866608');
