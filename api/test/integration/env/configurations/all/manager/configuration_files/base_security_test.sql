/*
 * SQL Schema all denied tests
 * Copyright (C) 2026 GuardSarm, Inc.
 * February 12, 2021.
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;

/* Testing user without any role assigned (no permissions) */
INSERT INTO users VALUES(99,'testing','pbkdf2:sha256:150000$OMVAATei$cb30da77537eea26b964265dab6f403e9499f18522c7cc9e6ba2cb2d33694e1f',0,'1970-01-01 00:00:00');

COMMIT;
