/* Copyright (C) 2026 GuardSarm, Inc.
 * August 4, 2018
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef YAML2JSON_H
#define YAML2JSON_H

#include "yaml.h"
#include <cJSON.h>

int yaml_parse_stdin(yaml_document_t * document);
int yaml_parse_file(const char * path, yaml_document_t * document);
cJSON * yaml2json(yaml_document_t * document, int quoted_float_as_string );

#endif
