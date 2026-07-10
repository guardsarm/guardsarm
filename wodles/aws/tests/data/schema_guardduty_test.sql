/*
 * SQL Schema AWS tests
 * Copyright (C) 2026 GuardSarm, Inc.
 * April 15, 2019.
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

CREATE TABLE 'guardduty' (
    bucket_path 'text' NOT NULL,
    aws_account_id 'text' NOT NULL,
    log_key 'text' NOT NULL,
    processed_date 'text' NOT NULL,
    created_date 'integer' NOT NULL,
    PRIMARY KEY (bucket_path, aws_account_id, log_key));

INSERT INTO 'guardduty' (
    bucket_path,
    aws_account_id,
    log_key,
    processed_date,
    created_date) VALUES (
    'test-bucket/',
    '123456789123',
    'guardduty/2019/04/15/07/firehose_guardduty-1-2019-04-15-08-16-03.zip',
    DATETIME('now'),
    '');

INSERT INTO 'guardduty' (
    bucket_path,
    aws_account_id,
    log_key,
    processed_date,
    created_date) VALUES (
    'test-bucket/',
    '123456789123',
    'guardduty/2019/04/15/07/firehose_guardduty-1-2019-04-15-09-16-03.zip',
    DATETIME('now'),
    '');

INSERT INTO 'guardduty' (
    bucket_path,
    aws_account_id,
    log_key,
    processed_date,
    created_date) VALUES (
    'test-bucket/',
    '123456789123',
    'guardduty/2019/04/15/07/firehose_guardduty-1-2019-04-15-10-16-03.zip',
    DATETIME('now'),
    '');

INSERT INTO 'guardduty' (
    bucket_path,
    aws_account_id,
    log_key,
    processed_date,
    created_date) VALUES (
    'test-bucket/',
    '123456789123',
    'guardduty/2019/04/15/07/firehose_guardduty-1-2019-04-15-11-16-03.zip',
    DATETIME('now'),
    '');

INSERT INTO 'guardduty' (
    bucket_path,
    aws_account_id,
    log_key,
    processed_date,
    created_date) VALUES (
    'test-bucket/',
    '123456789123',
    'guardduty/2019/04/15/07/firehose_guardduty-1-2019-04-15-12-16-03.zip',
    DATETIME('now'),
    '');

INSERT INTO 'guardduty' (
    bucket_path,
    aws_account_id,
    log_key,
    processed_date,
    created_date) VALUES (
    'test-bucket/',
    '123456789123',
    'guardduty/2019/04/15/07/firehose_guardduty-1-2019-04-15-13-17-03.zip',
    DATETIME('now'),
    '');

INSERT INTO 'guardduty' (
    bucket_path,
    aws_account_id,
    log_key,
    processed_date,
    created_date) VALUES (
    'test-bucket/',
    '123456789123',
    'guardduty/2019/04/15/07/firehose_guardduty-1-2019-04-15-13-18-03.zip',
    DATETIME('now'),
    '');

INSERT INTO 'guardduty' (
    bucket_path,
    aws_account_id,
    log_key,
    processed_date,
    created_date) VALUES (
    'test-bucket/',
    '123456789123',
    'guardduty/2019/04/15/07/firehose_guardduty-1-2019-04-15-13-19-03.zip',
    DATETIME('now'),
    '');
