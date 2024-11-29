#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ENTRY 128

typedef enum key_type {PROTO_ID, KEY_ID} key_type_e;

typedef struct id_store_s
{
	uint64_t	__proto_bit_array[2];
	uint64_t	__keystore_bit_array[2];
} id_store_t;

static id_store_t *global_keystore;

int __id_allocator()
{
	int __id = 0;
	for(;__id < MAX_ENTRY; __id++)
	{
		if(__id < 64)
		{
			if(!(global_keystore->__proto_bit_array[0] & (1LU<<__id)))
			{	global_keystore->__proto_bit_array[0] |= (1UL<<__id);
				return __id;
			}
		}
		else
		{
			if(!(global_keystore->__proto_bit_array[1] & (1LU<<(__id - 64))))
			{
				global_keystore->__proto_bit_array[1] |= (1LU<<(__id - 64));
				return __id;
			}
		}
	}
	return -1;
}

void __id_deallocator(int __id)
{
	if(__id < 64) global_keystore->__proto_bit_array[0] &= ~(1LU<<__id);
	else global_keystore->__proto_bit_array[1] &= ~(1LU<<(__id - 64));
}

int main()
{
	int id = 0, opt = 0;
	global_keystore = (id_store_t *)malloc(sizeof(id_store_t));

	while(1)
	{
		printf("enter option: 1 = alloc, 2 = dealloc, 3 = exit\n");
		scanf("%d", &opt);
		switch(opt)
		{
			case 1:
				printf("alloc'd: %d\n", __id_allocator());
				break;
			case 2:
				printf("id to be dealloc'd\n");
				scanf("%d", &id);
				if(id<=MAX_ENTRY)
				{
					__id_deallocator(id);
					printf("dealloc'd\n");
				}
				else
				{
					printf("out of range, max:128\n");
				}
				break;
			case 3:
				free(global_keystore);
				exit(0);
			default:
				printf("invalid input\n");
		}
	}
	return 0;
}
