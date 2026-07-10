/*
 * Label data operations
 * Copyright (C) 2026 GuardSarm, Inc.
 * February 27, 2017.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef LABELS_OP_H
#define LABELS_OP_H

 /* Label flags bitfield */

 typedef struct label_flags_t {
     unsigned int hidden:1;
     unsigned int system:1;
 } label_flags_t;

 /* Label data structure */

 typedef struct wlabel_t {
     char *key;
     char *value;
     label_flags_t flags;
 } wlabel_t;

/* Append a new label into an array of (size) labels at the moment of inserting. Returns the new pointer. */
wlabel_t* labels_add(wlabel_t *labels, size_t * size, const char *key, const char *value, label_flags_t flags, int overwrite);

/* Free label array */
void labels_free(wlabel_t *labels);

#endif
