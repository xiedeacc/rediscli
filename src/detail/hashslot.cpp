#include <stdint.h>

uint16_t crc16(const char *buf, int len);

/* We have 16384 hash slots. The hash slot of a given key is obtained
 * as the least significant 14 bits of the crc16 of the key.
 *
 * However if the key contains the {...} pattern, only the part between
 * { and } is hashed. This may be useful in the future to force certain
 * keys to be in the same node (assuming no resharding is in progress). */
unsigned int keyHashSlot(char *key, int keylen) {
  int s, e; /* start-end indexes of { and } */

  for (s = 0; s < keylen; s++)
    if (key[s] == '{') break;

  /* No '{' ? Hash the whole key. This is the base case. */
  if (s == keylen) return crc16(key, keylen) & 0x3FFF;

  /* '{' found? Check if we have the corresponding '}'. */
  for (e = s + 1; e < keylen; e++)
    if (key[e] == '}') break;

  /* No '}' or nothing betweeen {} ? Hash the whole key. */
  if (e == keylen || e == s + 1) return crc16(key, keylen) & 0x3FFF;

  /* If we are here there is both a { and a } on its right. Hash
   * what is in the middle between { and }. */
  return crc16(key + s + 1, e - s - 1) & 0x3FFF;
}
