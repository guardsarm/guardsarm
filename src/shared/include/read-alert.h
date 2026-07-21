/* Copyright (C) 2026 GuardSarm, Inc.
 * All right reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef CRALERT_H
#define CRALERT_H

#define CRALERT_MAIL_SET    0x001
#define CRALERT_EXEC_SET    0x002
#define CRALERT_READ_ALL    0x004
#define CRALERT_READ_FAILED 0x008
#define CRALERT_FP_SET      0x010

/* File queue */
typedef struct _alert_data {
    unsigned int rule;
    unsigned int level;
    char *alertid;
    char *date;
    char *location;
    char *comment;
    char *group;
    char *srcip;
    int srcport;
    char *dstip;
    int dstport;
    char *user;
    char *filename;
    char *old_md5;
    char *new_md5;
    char *old_sha1;
    char *new_sha1;
    char *old_sha256;
    char *new_sha256;
    char **log;
    char *srcgeoip;
    char *dstgeoip;
    /* "9/19/2016 - Sivakumar Nellurandi - parsing additions" */
    char *file_size;
    char *owner_chg;
    char *group_chg;
    char *perm_chg;
    /* "9/19/2016 - Sivakumar Nellurandi - parsing additions" */
} alert_data;

alert_data *GetAlertData(int flag, FILE *fp) __attribute__((nonnull));
void        FreeAlertData(alert_data *al_data) __attribute__((nonnull));

#endif /* CRALERT_H */
