/* OS_crypto/AES Library
 * Copyright (C) 2026 GuardSarm, Inc.
 * March 12, 2018.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef AES_OP_H
#define AES_OP_H

#define OS_ENCRYPT      1
#define OS_DECRYPT      0

int encrypt_AES(const unsigned char *plaintext, int plaintext_len, unsigned char *key,
    unsigned char *iv, unsigned char *ciphertext);
int decrypt_AES(const unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
        unsigned char *iv, unsigned char *plaintext);
int OS_AES_Str(const char *input, char *output, const char *charkey,
              long size, short int action) __attribute((nonnull));

#endif /* AES_OP_H */
