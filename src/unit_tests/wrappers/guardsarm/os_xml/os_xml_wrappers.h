/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef OS_XML_WRAPPERS_H
#define OS_XML_WRAPPERS_H

#include "../../common.h"
#include "os_xml.h"

const char* __wrap_w_get_attr_val_by_name(xml_node* node, const char* name);
xml_node** __wrap_OS_GetElementsbyNode(const OS_XML* _lxml, const xml_node* node) __attribute__((nonnull(1)));
void __wrap_OS_ClearNode(xml_node** node);
void __wrap_OS_ClearXML(OS_XML* _lxml) __attribute__((nonnull));
int __wrap_OS_ReadXML(const char* file, OS_XML* lxml) __attribute__((nonnull));
int __wrap_OS_ReadXML_Ex(const char* file, OS_XML* lxml) __attribute__((nonnull));
int __wrap_OS_ReadXMLString(const char* file, OS_XML* lxml) __attribute__((nonnull));

#endif
