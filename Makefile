all:
	gcc -g hashtable.c -o ip_hash_test.run
	gcc -g id_allocator.c -o id_alloc.run
	gcc -g ip-hash.c -o hash_profile.run
clean:
	rm -rf *.run
rebuild: clean all
