//
//  dumper.c
//  tree-parser-priv-final
//
//  Created by jndok on 01/01/16.
//  Copyright © 2016 jndok. All rights reserved.
//

#include "dumper.h"

extern uint64_t kslide;
extern uint64_t KextUnslidBaseAddress(const char *KextBundleName);

char *read_line(FILE *fin) {
    char *buffer;
    char *tmp;
    int read_chars = 0;
    int bufsize = INITIAL_ALLOC;
    char *line = malloc(bufsize);

    if ( !line ) {
        return NULL;
    }

    buffer = line;

    while ( fgets(buffer, bufsize - read_chars, fin) ) {
        read_chars = (int)strlen(line);

        if ( line[read_chars - 1] == '\n' ) {
            line[read_chars - 1] = '\0';
            return line;
        }

        else {
            bufsize = 2 * bufsize;
            tmp = realloc(line, bufsize);
            if ( tmp ) {
                line = tmp;
                buffer = line + read_chars;
            }
            else {
                free(line);
                return NULL;
            }
        }
    }
    return NULL;
}

uint32_t dump_hierarchy(task_t kport, uint64_t read_addr, char *name, const char *path, boolean_t override)
{
    uint64_t TEXT_SEG_START_ADDR=0, TEXT_SEG_END_ADDR=0, TEXT_SEG_SIZE=0;
    uint64_t CONST_SECT_START_ADDR=0, CONST_SECT_SIZE=0, COMMON_SECT_START_ADDR=0, COMMON_SECT_END_ADDR=0, COMMON_SECT_SIZE=0;

    struct mach_header_64 *header = find_mach_header_kmem_addr(kport, SLIDE_POINTER(read_addr));

    void *load_commands = read_kernel_memory(kport, SLIDE_POINTER(read_addr+sizeof(struct mach_header_64)), header->sizeofcmds);

    struct segment_command_64 *TEXT_SEG = find_segment_command_kmem(header, load_commands, SEG_TEXT);
    struct segment_command_64 *DATA_SEG = find_segment_command_kmem(header, load_commands, SEG_DATA);

    if (!TEXT_SEG || !DATA_SEG) {
        __dbg("(!) Invalid KEXT.");
        return 2;
    }

    struct section_64 *DATA_CONST_SECT = find_section_command_kmem(DATA_SEG, SECT_CONST);
    struct section_64 *DATA_COMMON_SECT = find_section_command_kmem(DATA_SEG, SECT_COMMON);

    if (!DATA_CONST_SECT || !DATA_COMMON_SECT) {
        __dbg("(!) Invalid KEXT.");
        return 2;
    }

    TEXT_SEG_START_ADDR =       TEXT_SEG->vmaddr;
    TEXT_SEG_SIZE =             TEXT_SEG->vmsize;
    TEXT_SEG_END_ADDR =         TEXT_SEG_START_ADDR + TEXT_SEG_SIZE;

    CONST_SECT_START_ADDR =     DATA_CONST_SECT->addr;
    CONST_SECT_SIZE =           DATA_CONST_SECT->size;

    COMMON_SECT_START_ADDR =    DATA_COMMON_SECT->addr;
    COMMON_SECT_SIZE =          DATA_COMMON_SECT->size;
    COMMON_SECT_END_ADDR =      COMMON_SECT_START_ADDR + COMMON_SECT_SIZE;

    void *const_sect_buffer = malloc(CONST_SECT_SIZE);
    for (uint32_t i=0; i<CONST_SECT_SIZE; i+=128) {
        read_kernel_memory_in_buffer(kport, CONST_SECT_START_ADDR+i, 128, const_sect_buffer+i);
    }

    uint64_t vtable_ptrs[16];
    uint32_t vtable_ptrs_cnt=0;

    if (override) {
        INIT_OUTPUT_FILE_OVERRIDE(path, name);
    } else {
        INIT_OUTPUT_FILE(path, name);
    }

    for (uint32_t i=0; i<CONST_SECT_SIZE; i+=8) {
        if (vtable_ptrs_cnt==13) {  //we are in a vtable
            if (IS_VALID_POINTER(vtable_ptrs[7], TEXT_SEG_START_ADDR, TEXT_SEG_END_ADDR)) {

                struct hierarchy_entry *k, *last=NULL, *last_parent=NULL, *last_entry=NULL;

                int found_in_file=0;

                FILE *fin;
                char *line;

                uint64_t gMetaClass_addr = calculate_gMetaClass_addr_from_getMetaClass(kport, vtable_ptrs[7]);
                if (!IS_VALID_POINTER(gMetaClass_addr, COMMON_SECT_START_ADDR, COMMON_SECT_END_ADDR)) {
                    goto end;
                }

                SLIST_HEAD(hierarchy_head, hierarchy_entry) hierarchy_h = SLIST_HEAD_INITIALIZER(hierarchy_h);
                SLIST_HEAD(hierarchy_write_head, hierarchy_entry) sub_hierarchy_h = SLIST_HEAD_INITIALIZER(sub_hierarchy_h);

                SLIST_INIT(&hierarchy_h);
                SLIST_INIT(&sub_hierarchy_h);

                while (1) {
                    MetaClass_Object_t *meta = parse_MetaClass_object(kport, gMetaClass_addr);
                    OSSymbol_Object_t *symbol = parse_OSSymbol_object(kport, meta->OSSymbol_ptr);

                    struct hierarchy_entry *entry = malloc(sizeof(struct hierarchy_entry));
                    entry->name=getOSSymbolName(kport, symbol);
                    SLIST_INSERT_HEAD(&hierarchy_h, entry, entries);

                    if (meta->Parent_gMetaClass_ptr==0) {
                        break;
                    }

                    gMetaClass_addr=meta->Parent_gMetaClass_ptr;
                }

                SLIST_FOREACH(k, &hierarchy_h, entries)
                last=k;

                char search[128];

                SLIST_FOREACH(k, &hierarchy_h, entries) {

                    strcpy(search, " ");
                    strcat(search, k->name);
                    strcat(search, " ");        //this is needed for some rare cases

                    fin=fopen(path, "r");
                    while ((line = read_line(fin)) ) {
                        if (strstr(line, search)) {
                            found_in_file=1;
                            break;
                        } else {
                            found_in_file=0;
                        }

                        free(line);
                    }
                    fclose(fin);

                    if (found_in_file) {
                        last_parent=k;
                    } else {
                        if (last_parent) {
                            struct hierarchy_entry *write_entry = malloc(sizeof(struct hierarchy_entry));
                            write_entry->name=k->name;
                            if (!last_entry) {
                                SLIST_INSERT_HEAD(&sub_hierarchy_h, write_entry, entries);
                                last_entry=write_entry;
                            } else {
                                SLIST_INSERT_AFTER(last_entry, write_entry, entries);
                                last_entry=write_entry;
                            }
                        }
                    }

                    bzero(search, 128);
                }

                fin=fopen(path, "a");

                if (!last_parent) {
                    struct hierarchy_entry *entry;
                    SLIST_FOREACH(entry, &hierarchy_h, entries) {
                        fprintf(fin, "%s ", entry->name);
                        if (strcmp(entry->name, last->name) != 0) {
                            fprintf(fin, "-> ");
                        } else {
                            fprintf(fin, "\n");
                        }
                    }
                } else if (last_parent && !found_in_file) {
                    fprintf(fin, "%s -> ", last_parent->name);

                    struct hierarchy_entry *entry;
                    SLIST_FOREACH(entry, &sub_hierarchy_h, entries) {
                        fprintf(fin, "%s ", entry->name);
                        if (strcmp(entry->name, last->name) != 0) {
                            fprintf(fin, "-> ");
                        } else {
                            fprintf(fin, "\n");
                        }
                    }
                }

                fclose(fin);

            end:;

            }
            vtable_ptrs_cnt=0;
            for ( ;; i+=8) {    //advance to next vtable
                uint64_t addr_junk=*(uint64_t*)(const_sect_buffer+i);
                if (addr_junk==0) {
                    break;
                }
            }
        }


        uint64_t curr_ptr = *(uint64_t*)(const_sect_buffer+i);
        if (curr_ptr!=0x0) {
            vtable_ptrs[vtable_ptrs_cnt]=curr_ptr;
            vtable_ptrs_cnt++;
        } else if (curr_ptr==0x0) {
            vtable_ptrs_cnt=0;
        } else {
            __dbg("(!) Invalid vtable pointer found while parsing (%#llx)! Aborting.", curr_ptr);
            return 2;
        }
    }

    CLOSE_OUTPUT_FILE(path);

    __dbg("(+) Done! Written hierarchy for %#llx (%s) @ path: %s", read_addr, name, path);
    return 0;
}
