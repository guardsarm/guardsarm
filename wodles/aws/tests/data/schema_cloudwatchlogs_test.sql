/*
 * SQL Schema AWS tests
 * Copyright (C) 2026 GuardSarm, Inc.
 * April 15, 2019.
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

CREATE TABLE 'cloudwatch_logs' (
    aws_region 'text' NOT NULL,
    aws_log_group 'text' NOT NULL,
    aws_log_stream 'text' NOT NULL,
    next_token 'text',
    start_time 'integer',
    end_time 'integer',
    PRIMARY KEY (aws_region, aws_log_group, aws_log_stream));

INSERT INTO 'cloudwatch_logs' (
    aws_region,
    aws_log_group,
    aws_log_stream,
    next_token,
    start_time,
    end_time) VALUES (
    'us-east-1',
    'test_log_group',
    'test_stream',
    'f/12345678123456781234567812345678123456781234567812345678/s',
    1640996200000,
    1659355591835
    );
