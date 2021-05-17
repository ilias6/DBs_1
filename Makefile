
all: HEAP_FILE/heap_main HASH_FILE/hash_main

HEAP_FILE/heap_main: HEAP_FILE/HP_functs.o HEAP_FILE/HP_main.o BF_64.a Record/Recordfuncts.o
	gcc -o HEAP_FILE/heap_main HEAP_FILE/HP_functs.o HEAP_FILE/HP_main.o BF_64.a Record/Recordfuncts.o -no-pie

HP_main.o:  HEAP_FILE/HP_main.c HEAP_FILE/HP_functs.h Record/Record.h Ht_info.h
	gcc -c HEAP_FILE/HP_main.c
HP_functs.o:  HEAP_FILE/HP_functs.c Record/Recordfuncts.h HEAP_FILE/HP_functs.h BF.h
	gcc -c HEAP_FILE/HP_functs.c
Recordfuncts.o: Record/Recordfuncts.c Record/Recordfuncts.h
	gcc -c Record/Recordfuncts.c

HASH_FILE/hash_main: HASH_FILE/hashfuncts.o HASH_FILE/HT_functs.o HASH_FILE/HT_main.o BF_64.a Record/Recordfuncts.o
	gcc -o HASH_FILE/hash_main HASH_FILE/hashfuncts.o HASH_FILE/HT_functs.o HASH_FILE/HT_main.o BF_64.a Record/Recordfuncts.o -no-pie

HT_main.o:  HASH_FILE/HT_main.c Record/Record.h  HASH_FILE/HT_functs.h Ht_info.h
	gcc -c HASH_FILE/HT_main.c
hashfuncts.o:  HASH_FILE/hashfuncts.c HASH_FILE/hashfuncts.h
	gcc -c HASH_FILE/hashfuncts.c
HT_functs.o:  HASH_FILE/HT_functs.c  Record/Recordfuncts.h HASH_FILE/HT_functs.h HASH_FILE/hashfuncts.h BF.h
	gcc -c HASH_FILE/HT_functs.c

clean:
	rm HASH_FILE/*.o HEAP_FILE/*.o Record/*.o HEAP_FILE/heap_main HASH_FILE/hash_main
