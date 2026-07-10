/* Copyright (C) 2026 GuardSarm, Inc.
 * May, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef BINARIES_OP_H
#define BINARIES_OP_H

/**
 * @brief Check if the binary exists in the default path and complete the path parameter with the full_path.
 *
 * @param command Command searched for.
 * @param validated_comm Variable to be filled with full_path in case of success, or with the original command if it was not found in any path.
 * @retval -1 If it was not found on any path.
 * @retval 0 If it was found.
 */
int get_binary_path(const char *binary, char **validated_comm);

#endif /* BINARIES_OP_H */
