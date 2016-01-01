//
//  runtime_parsing.h
//  tree-parser-priv-final
//
//  Created by jndok on 29/12/15.
//  Copyright © 2015 jndok. All rights reserved.
//

#ifndef runtime_parsing_h
#define runtime_parsing_h

#include <stdio.h>
#include <stdlib.h>
#include <mach/mach.h>
#include <mach-o/loader.h>

#include "../../helper.h"
#include "../kernel.h"

/***/

__attribute__((always_inline)) struct mach_header_64 *find_mach_header_kmem_addr(task_t task, uint64_t addr);
__attribute__((always_inline)) struct segment_command_64 *find_segment_command_kmem(struct mach_header_64 *header, void *cmds_buffer, const char *seg_name);
__attribute__((always_inline)) struct section_64 *find_section_command_kmem(struct segment_command_64 *seg_cmd, const char *sect_name);

/***/

#endif /* runtime_parsing_h */
