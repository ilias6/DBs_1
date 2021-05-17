#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HP_functs.h"
#include "../Record/Record.h"
#include "../BF.h"
#include <stdint.h>

#define MAX_FILES 100


void HP_Init() {
    BF_Init();
    return ;
}

int HP_CreateFile( char * fileName, char attrType, char * attrName, int attrLength) {
    int err_no;
    File_type t = HEAP_FILE;
    if (BF_CreateFile(fileName) < 0) {
        BF_PrintError("Error creating file");
        return -1;
    }

    int fd = BF_OpenFile(fileName);
    if (fd < 0) {
        BF_PrintError("Error opening file");
        return -1;
    }

    do {
      // if fail -> try again
    } while(BF_AllocateBlock(fd) < 0);

    uint8_t * block;
    if (BF_ReadBlock(fd, 0, (void **)&block) < 0) {
        BF_PrintError("Error reading block 0 from file");
        return -1;
    }

    memcpy(block, &t, sizeof(File_type));
    memcpy(block +sizeof(File_type), (uint8_t *)&attrType, sizeof(char));
    memcpy(block +sizeof(File_type) +sizeof(char), (uint8_t *)&attrLength, sizeof(int));
    memcpy(block +sizeof(File_type) +sizeof(char) +sizeof(int), (uint8_t *)attrName, attrLength);

    if (BF_WriteBlock(fd, 0) < 0) {
        BF_PrintError("Error writing block 0 to file");
        return -1;
    }
    // allocate first data block and write 0 as first int indicating current records in block
    do {
        // if fail -> try again
    } while(BF_AllocateBlock(fd) < 0);

    int records_at_start = 0;
    if (BF_ReadBlock(fd, 1, (void **)&block) < 0) {
        BF_PrintError("Error reading block 0 from file");
        return -1;
    }

    memcpy(block, &records_at_start, sizeof(int));

    if (BF_WriteBlock(fd, 1) < 0) {
        BF_PrintError("Error writing block 0 to file");
        return -1;
    }

    if (BF_CloseFile(fd) < 0) {
        BF_PrintError("Error closing file");
        return -1;
    }
    return 0;
}

HT_info * HP_OpenFile(char * fileName) {
    static int num_of_open_files = 0;
    // open up to MAX_FILES
    if (num_of_open_files > MAX_FILES)
        return NULL;

    HT_info * info = malloc(sizeof(HT_info));
    if (info==NULL) {
        printf("Malloc error line 87!\n");
        return NULL;
    }

    info->fileDesc = BF_OpenFile(fileName);
    if (info->fileDesc < 0) {
        BF_PrintError("Error opening file");
        return NULL;
    }

    uint8_t * block;
    do {
    } while(BF_ReadBlock(info->fileDesc, 0, (void **)&block) < 0);

    memcpy(&info->t, block, sizeof(File_type));
    if (info->t != HEAP_FILE) {
        free(info);
        return NULL;
    }

    memcpy(&info->attrType, block +sizeof(File_type), sizeof(uint8_t));
    memcpy(&info->attrLength, block +sizeof(File_type) +1, sizeof(int));

    do {
    } while( (info->attrName = malloc(info->attrLength) )==NULL);

    memcpy(info->attrName, block +sizeof(File_type) +1 +sizeof(int), info->attrLength);

    num_of_open_files++;

    return info;
}


int HP_CloseFile(HT_info * header_info) {

    if (BF_CloseFile(header_info->fileDesc) < 0) {
        BF_PrintError("Error closing file");
        return -1;
    }

    free(header_info->attrName);
    free(header_info);

    return 0;
}


int HP_InsertEntry(HT_info header_info, Record record){
    int num_of_blocks = BF_GetBlockCounter(header_info.fileDesc);
    if (num_of_blocks < 0) {
        BF_PrintError("Error getting number of blocks in db!");
        return -1;
    }

    uint8_t * block;
    // check if record exists
    int records_in_block;
    Record r;
    int last_block = 1;
    for(int i = 1; i < num_of_blocks; i++) {
        if (BF_ReadBlock(header_info.fileDesc, i, (void **)&block) < 0) {
            BF_PrintError("Error reading block from file line 145 [HP_insertEntry]");
            return -1;
        }
        memcpy(&records_in_block,block, sizeof(int));
        if (records_in_block == 0) {
          if (BF_ReadBlock(header_info.fileDesc, last_block, (void **)&block) < 0) {
              BF_PrintError("Error reading block from file line 151 [HP_insertEntry]");
              return -1;
          }
          break;
        }
        for (int j = 0; j < records_in_block; j++) {
            if (mem_to_record(block +sizeof(int) +j*sizeofRecord(&record), &r)==0) {
                printf("Mem to record fail![HP_Insert]");
                return -1;
            }
            if(reccmp(&r, &record, header_info.attrName)==0) {
                //it means record exists
                // printf("Record exists (id: %d)!\n",record.id);
                return -1;
            }
        }
        last_block = i;
    }

    // if doesn't exist and block is full of records
    if (records_in_block==(BLOCK_SIZE -sizeof(int))/sizeofRecord(&record)) {
        records_in_block = 0;
        if (num_of_blocks == last_block+1) {
          // allocate new block
          do {
            // if fail -> try again
          } while(BF_AllocateBlock(header_info.fileDesc) < 0);
          num_of_blocks++;

        if (BF_ReadBlock(header_info.fileDesc, num_of_blocks-1, (void **)&block) < 0) {
            BF_PrintError("Error reading block from file line 181 [HP_insertEntry]");
            return -1;
        }
      }
    }
    //insert to apropriate block
    if (record_to_mem(block +sizeof(int) +records_in_block*sizeofRecord(&record), &record)==0) {
        // printf("---Failed to insert record with id %d---\n",record.id);
        return -1;
    }
    records_in_block++;
    // update number of records in block
    memcpy(block, &records_in_block, sizeof(int));

    if (BF_WriteBlock(header_info.fileDesc, num_of_blocks-1) < 0) {
        BF_PrintError("Error writing block 0 to file");
        return -1;
    }

    return num_of_blocks -1;
}


int find_last_block(HT_info header_info) {
    int last_block = BF_GetBlockCounter(header_info.fileDesc) -1;

    uint8_t * block;
    int records_in_block;
    int found = 0;

    for(;last_block >= 1; last_block--) {
        if (BF_ReadBlock(header_info.fileDesc, last_block, (void **)&block) < 0) {
            BF_PrintError("Error reading block from file line 213 [find_last_block]");
            return -1;
        }
        memcpy(&records_in_block,block, sizeof(int));
        if (records_in_block != 0) {
            found = 1;
            break;
        }
    }
    if(found)
        return last_block;

    return -1;
}

int HP_DeleteEntry(HT_info header_info, void * value){
    uint8_t * block;
    int exist = 0;
    int num_of_blocks = BF_GetBlockCounter(header_info.fileDesc);

    if (num_of_blocks < 0) {
        BF_PrintError("Error getting number of blocks in db!");
        return -1;
    }
    // check if record exists
    int records_in_block;
    int which_block = 0;
    int which_record;
    Record r;
    for(int i = 1; i < num_of_blocks && !exist; i++) {
        if (BF_ReadBlock(header_info.fileDesc, i, (void **)&block) < 0) {
            BF_PrintError("Error reading block from file line 244[HP_DeleteEntry]");
            return -1;
        }

        memcpy(&records_in_block,block, sizeof(int));
        for(int j = 0; j < records_in_block; j++) {
            if (mem_to_record(block +sizeof(int) +j*sizeofRecord(&r), &r)==0) {
                printf("Mem to record fail![HP_DeleteEntry]");
                return -1;
            }
            if(rec_key_cmp(&r, value,header_info.attrName)==0) {
                exist = 1;
                which_block = i;
                which_record = j;
                break;
            }
        }
    }
    if (exist) {
        //record found in which_block (block variable) in position which record
        uint8_t* last_block;
        int records_in_last_block;
        //this function returns the last block that has at least 1 record
        // when we delete lets say all the records from last block this block will remain in db (consume not needed space)
        //although it has no data (records in block is 0)
        //because the exercise dont give us the tools to free this block
        // (meaning BF_GetBlockCounter doesn't return the last block that has data)
        // we must manually find the last block because in this case BF_getBlockCounter has no value....
        int last_block_index = find_last_block(header_info);

        //if something happens in find_last_block
        if (last_block_index==-1)
            return -1;
        if (BF_ReadBlock(header_info.fileDesc, which_block, (void **)&block) < 0) {
            BF_PrintError("Error reading block from file line 278[HP_DeleteEntry]");
            return -1;
        }
        // if record is found in last block with data
        if (last_block_index==which_block) {
            //if there is one record in last block we simply decrease records in block int
            memcpy(&records_in_last_block, block, sizeof(int));
            if (records_in_last_block==1) {
                records_in_last_block = 0;
                memcpy(block, &records_in_last_block, sizeof(int));
                BF_WriteBlock(header_info.fileDesc, which_block);
                return 0;
            }
        }
        // we must take last record and fill the whole
        //read block again because if db is big it may not still be in buffer
        if (BF_ReadBlock(header_info.fileDesc, last_block_index, (void **)&last_block) < 0) {
            BF_PrintError("Error reading block from file line 295 [HP_DeleteEntry]");
            return -1;
        }

        memcpy(&records_in_last_block, last_block, sizeof(int));

        Record last_rec;
        mem_to_record(last_block +sizeof(int) +(records_in_last_block -1)*sizeofRecord(&last_rec), &last_rec);
        record_to_mem(block +sizeof(int) +which_record*sizeofRecord(&last_rec), &last_rec);
        records_in_last_block--;
        memcpy(last_block, &records_in_last_block, sizeof(int));

        if (BF_WriteBlock(header_info.fileDesc,which_block) < 0) {
            BF_PrintError("Failed to write line 308 [HP_DeleteEntry]!");
            return -1;
        }
        if(BF_WriteBlock(header_info.fileDesc, last_block_index) < 0) {
            BF_PrintError("Failed to write line 312 [HP_DeleteEntry]!");
            return -1;
        }
        // printf("WROTE TO %d in %d rec\n",which_block,which_record);
        return 0;
    }
    //if not found
    return -1;
}
int HP_GetAllEntries(HT_info header_info, void * value) {
    uint8_t * block;
    int num_of_blocks = BF_GetBlockCounter(header_info.fileDesc);

    int records_in_block;
    int blocks_read = 0;
    Record r;


    for(int i = 1; i < num_of_blocks; i++) {
        if (BF_ReadBlock(header_info.fileDesc, i, (void **)&block) < 0) {
            BF_PrintError("Error reading block from file line 332[HP_GetAllEntries]");
            return -1;
        }
        blocks_read++;
        memcpy(&records_in_block,block, sizeof(int));

        for (int j = 0; j < records_in_block; j++) {
            if (mem_to_record(block +sizeof(int) +j*sizeofRecord(&r), &r)==0) {
                printf("Mem to record fail![HP_GetAllEntries]");
                return -1;
            }
            if (value == NULL) {
                printf("--------------------\n");
                printRecord(r);
                continue;
            }
            if (rec_key_cmp(&r, value, header_info.attrName)==0) {
              printRecord(r);
              return blocks_read;
            }
        }
    }

    return blocks_read;
}
