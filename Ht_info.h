#ifndef __HT_INFO_H__
#define __HT_INFO_H__
typedef enum file_type{HEAP_FILE,HASH_FILE} File_type;
typedef struct ht_info{
  File_type t;
  char attrType;
  int attrLength;
  char * attrName;
  int fileDesc;
  // this is for hash file
  int numBuckets;
}HT_info;
void print_info(HT_info * info);
#endif
