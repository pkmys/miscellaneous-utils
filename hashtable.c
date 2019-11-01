#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <arpa/inet.h>

#define TABLE_SIZE 128

typedef struct ip_lookup_s {
    uint8_t ip_proto;
    uint8_t id;
    uint32_t key; 
    void *value;
    struct ip_lookup_s *next;
} ip_lookup_t;

typedef struct tcp_node_s {
    uint8_t ip_proto;
    uint8_t id;
    uint16_t port;
    void *next;
} tcp_node_t;	

typedef struct udp_node_s {
    uint8_t ip_proto;
    uint8_t id;
    uint16_t port;
    void *next;
} udp_node_t;

typedef struct ip_hash_s {
    ip_lookup_t **entries;
} ip_ht_t;

// integer hash function
// Thomas Wang's 32 Bit Mix Function
const unsigned byte_hash(const uint8_t key8)
{
    unsigned key = key8;
    key += ~(key << 15);
    key ^= (key >> 10);
    key += (key << 3);
    key ^= (key >> 6);
    key += ~(key << 11);
    key ^= (key >> 16);
    return key%TABLE_SIZE;
}

// Thomas Wang's 32 Bit Mix Function
const unsigned sort_hash(const uint16_t key16)
{
    unsigned key = key16;
    key += ~(key << 15);
    key ^= (key >> 10);
    key += (key << 3);
    key ^= (key >> 6);
    key += ~(key << 11);
    key ^= (key >> 16);
    return key%TABLE_SIZE;
}

// Thomas Wang's 32 Bit Mix Function
const unsigned long_hash(const uint32_t key32)
{
    unsigned key = key32;
    key += ~(key << 15);
    key ^= (key >> 10);
    key += (key << 3);
    key ^= (key >> 6);
    key += ~(key << 11);
    key ^= (key >> 16);
    return key%TABLE_SIZE;
}
// Thomas Wang's 64 bit Mix Function
const unsigned long_long_hash(const uint64_t key64)
{
    unsigned long long key = key64;
    key += ~(key << 32);
    key ^= (key >> 22);
    key += ~(key << 13);
    key ^= (key >> 8);
    key += (key << 3);
    key ^= (key >> 15);
    key += ~(key << 27);
    key ^= (key >> 31);
    return key%TABLE_SIZE;
}

ip_lookup_t *ip_ht_insert(const uint32_t key, void* value) {
    // allocate the entry
    ip_lookup_t *entry = (ip_lookup_t *)malloc(sizeof(ip_lookup_t));
    entry->key = key;
    if(value) {
    	// copy the key and value in place
    	entry->value = value;
	entry->ip_proto = ((ip_lookup_t *)value)->ip_proto;
    }
    // next starts out null but may be set later on
    entry->next = NULL;
    return entry;
}

ip_ht_t *ip_ht_create(void) {
    // allocate table
    ip_ht_t *hash_table = (ip_ht_t *)malloc(sizeof(ip_ht_t));
    // allocate table entries
    hash_table->entries = (ip_lookup_t **)malloc(sizeof(ip_lookup_t *)*TABLE_SIZE);
    // set each to null (needed for proper operation)
    for (int i = 0; i < TABLE_SIZE; ++i)  hash_table->entries[i] = NULL;
    printf("IP HASH: table init OK\n");
    return hash_table;
}

void ip_ht_set(ip_ht_t *hash_table, const uint32_t key, void* value) {
    // calculate hash key
    const unsigned hash = long_hash(key);
    // try to look up an entry set
    printf("IP HASH: hashing @ 0x%08x\n", hash);
    ip_lookup_t *entry = hash_table->entries[hash];
    // no entry means slot empty, insert immediately
    if (!entry) {
        hash_table->entries[hash] = ip_ht_insert(key, value);
        return;
    }

    ip_lookup_t *prev;
    // walk through each entry until either the end is
    // reached or a matching key is found
    while (entry) {
        // check key
        if (entry->key == key) {
            // match found, itrate over value
	    if (!entry->value) {
                if (value) {
	            entry->ip_proto = *((uint8_t *)value);
		    if (entry->ip_proto == 6 || entry->ip_proto == 17)
		        entry->value = value;
	        }
	        return;
	    }
            
            void *it_value = entry->value;
	    uint8_t ip_proto = entry->ip_proto;

	    while(it_value) {
		switch (ip_proto) {
		    case 6:
			if (!((tcp_node_t *)it_value)->next) {
			    ((tcp_node_t *)it_value)->ip_proto = *(uint8_t *)value;
			    ((tcp_node_t *)it_value)->next = value;
			    return;
			}
			it_value = ((tcp_node_t *)it_value)->next;
			break;
		    case 17:
                        if (!((udp_node_t *)it_value)->next) {
                            ((udp_node_t *)it_value)->ip_proto = *(uint8_t *)value;
                            ((udp_node_t *)it_value)->next = value;
                            return;
                        }
                        it_value = ((udp_node_t *)it_value)->next;
                        break;
		    default:
			printf("IP HASH LIST: Unknown Protocol, list corrupted!\n");
			return;
		    }
		ip_proto = *((uint8_t *)it_value);
	    }
	return;
        }
        // walk to next
        prev = entry;
        entry = prev->next;
    }

    // end of chain reached without a match, add new
    prev->next = ip_ht_insert(key, value);
}

const void *ip_ht_get(ip_ht_t *hash_table, const uint32_t key) {
    // calculate hash key
    const uint32_t hash = long_hash(key);
    // try to find a valid slot
    ip_lookup_t *entry = hash_table->entries[hash];
    // no slot means no entry
    if (!entry) return NULL;

    // walk through each entry in the slot, which could just be a single thing
    while (entry) {
        // return value if found
        if (entry->key == key) return entry->value;
        // proceed to next key if available
        entry = entry->next;
    }

    // reaching here means there were >= 1 entries but no key match
    return NULL;
}

void ip_ht_del(ip_ht_t *hash_table, const uint32_t key) {
    // calculate hash key
    const uint32_t hash = long_hash(key);
    // try to find a valid bucket
    ip_lookup_t *entry = hash_table->entries[hash];
    // no bucket means no entry
    if (!entry)  return;

    ip_lookup_t *prev;
    int idx = 0;

    // walk through each entry until either the end is reached or a matching key is found
    while (entry) {
        // check key
        if (entry->key == key) {
            // first item and no next entry
            if (entry->next == NULL && idx == 0)  hash_table->entries[hash] = NULL;

            // first item with a next entry
            if (entry->next != NULL && idx == 0)  hash_table->entries[hash] = entry->next;

            // last item
            if (entry->next == NULL && idx != 0)  prev->next = NULL;

            // middle item
            if (entry->next != NULL && idx != 0)  prev->next = entry->next;

            free(entry);
            return;
        }

        // walk to next
        prev = entry;
        entry = prev->next;
        ++idx;
    }
}

void ip_ht_dump(ip_ht_t *hash_table) {

    struct in_addr addr = {0};
    for (int i = 0; i < TABLE_SIZE; ++i) {

        ip_lookup_t *entry = hash_table->entries[i];

        if (!entry) continue;

        printf("slot[%04d]: ", i);

        for(;;) {
	    addr.s_addr = htonl(entry->key);
            printf("%-15s : %s  ", inet_ntoa(addr), entry->value ? "":"NULL");

            void *it_value = entry->value;
            uint8_t ip_proto = entry->ip_proto;

            while(it_value) {
                switch (ip_proto) {
                    case 6:
                        printf("--> [TCP #%04d] ", ((tcp_node_t *)it_value)->port);
			ip_proto = *((uint8_t *)it_value);
                        it_value = ((tcp_node_t *)it_value)->next;
                        break;
                    case 17:
                        printf("--> [UDP #%04d] ", ((udp_node_t *)it_value)->port);
			ip_proto = *((uint8_t *)it_value);
                        it_value = ((udp_node_t *)it_value)->next;
                        break;
                    default:
                        printf(" IP HASH LIST: Unknown Protocol, list corrupted!\n");
                        goto EXIT_LEAF;
                    }
            }
EXIT_LEAF:
            if (!entry->next) break;
	    printf("\n            ");

            entry = entry->next;
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {

    ip_ht_t *ht = NULL; 
    struct in_addr addr = {0};
    uint32_t ip = 0x46827024;
    uint32_t mask = 0xFFFFFFFF;

    ht =  ip_ht_create();

    if (ht) {
        for (int i = 0; i < 128; i++) {
            addr.s_addr = htonl(ip);
	    udp_node_t *udp = (udp_node_t *)malloc(sizeof(udp_node_t));
	    udp->ip_proto = 6;
	    udp->port = 0x77 + (i%15) * 55;
            ip_ht_set(ht, ip&mask, udp);
            tcp_node_t *tcp = (tcp_node_t *)malloc(sizeof(tcp_node_t));
            tcp->ip_proto = 17;
            tcp->port = 0x77 + (i%15) * 56;
            ip_ht_set(ht, ip&mask, tcp);
            ip += (i%2);
        }
        ip_ht_dump(ht);
    }

    return 0;
}
