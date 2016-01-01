//
//  runtime_parsing.c
//  tree-parser-priv-final
//
//  Created by jndok on 29/12/15.
//  Copyright © 2015 jndok. All rights reserved.
//

#include "runtime_parsing.h"

__attribute__((always_inline)) struct mach_header_64 *find_mach_header_kmem_addr(task_t task, uint64_t addr)
{
    struct mach_header_64 *header = read_kernel_memory(task, addr, sizeof(struct mach_header_64));
    if (header->magic != MH_MAGIC_64) {
        return NULL;
    }
    
    return header;
}

__attribute__((always_inline)) struct segment_command_64 *find_segment_command_kmem(struct mach_header_64 *header, void *cmds_buffer, const char *seg_name)
{
    if (!header) {
        return NULL;
    }
    
    struct load_command *lcmd=(struct load_command*)cmds_buffer;
    for (uint32_t i=0; i<header->ncmds; ++i) {
        if (lcmd->cmd == LC_SEGMENT_64) {
            struct segment_command_64 *seg_cmd=(struct segment_command_64*)lcmd;
            
            if (strcmp(seg_cmd->segname, seg_name) == 0) {
                return seg_cmd;
            }
        }
        
        lcmd = ((void*)lcmd + lcmd->cmdsize);
    }
    return NULL;
}

__attribute__((always_inline)) struct section_64 *find_section_command_kmem(struct segment_command_64 *seg_cmd, const char *sect_name)
{
    if (!seg_cmd) {
        return NULL;
    }
    
    struct section_64 *sect_cmd = (struct section_64*)((void*)seg_cmd + sizeof(struct segment_command_64));
    for (uint32_t i=0; i<seg_cmd->nsects; ++i) {
        if (strcmp(sect_cmd->sectname, sect_name) == 0) {
            return sect_cmd;
        }
        sect_cmd = ((void*)sect_cmd + sizeof(struct section_64));
    }
    return NULL;
}