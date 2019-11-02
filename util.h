/* *
 * @module: util.h
 * @author: Pawan Kumar
 * @email: jmppawanhit@gmail.com
 * @license: GPL
 * @domain: Linux Network Programming
 * @description: Demonstrating the simple firewall module 
 *               using netfilter hooks.
 * @copyright: Copyright (C) 2018
 */
#ifndef _UTIL_H
#define _UTIL_H

#include <linux/types.h>

#define MAX_ENTRY 128

typedef struct id_store_s
{
    __UINT64_TYPE__ ip_bit_array[2];
    __UINT64_TYPE__ tcp_bit_array[2];
    __UINT64_TYPE__ udp_bit_array[2];
} id_store_t;

void util_dump_hex(const __UINT8_TYPE__* const ptr, __UINT64_TYPE__ len);
__INT32_TYPE__ util_id_allocate(__UINT64_TYPE__ *key_array);
void util_id_dallocate(__INT32_TYPE__ id, __UINT64_TYPE__ *key_array);

#endif /* _UTIL_H*/
