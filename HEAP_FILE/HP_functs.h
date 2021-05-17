#ifndef __HP_FUNCTS_H__
#define __HP_FUNCTS_H__
#include "../Ht_info.h"
#include "../Record/Record.h"

void HP_Init();
int HP_CreateFile(char * fileName, char attrType, char * attrName, int attrLength);
HT_info * HP_OpenFile(char * fileName);
int HP_CloseFile(HT_info * header_info);
int HP_InsertEntry(HT_info header_info, Record record);
int HP_DeleteEntry(HT_info header_info, void * value);
int HP_GetAllEntries(HT_info header_info, void * value);
#endif
