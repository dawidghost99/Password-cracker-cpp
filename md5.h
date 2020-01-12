// written by p.hall@abertay.ac.uk to avoid using open SSL libraries which your pc may no have.

#pragma once

#include <stdint.h>

#define MD5_DIGEST_LENGTH (16)

extern "C" void md5(uint8_t *initial_msg, size_t initial_len, unsigned char *digest);