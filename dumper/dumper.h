//
//  dumper.h
//  tree-parser-priv-final
//
//  Created by jndok on 01/01/16.
//  Copyright © 2016 jndok. All rights reserved.
//

#ifndef dumper_h
#define dumper_h

#define INITIAL_ALLOC 512

#define INIT_OUTPUT_FILE(file) {    \
    FILE *f=fopen(file, "w");   \
    fprintf(f, "Digraph G {\n");    \
    fclose(f);  \
}

#define CLOSE_OUTPUT_FILE(file) {   \
    FILE *f=fopen(file, "a");   \
    fprintf(f, "}\n");  \
    fclose(f);  \
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>

#include "../kernel/runtime_parsing/runtime_parsing.h"
#include "../kernel/analysis/analysis.h"

struct hierarchy_entry {
    char *name;
    SLIST_ENTRY(hierarchy_entry) entries;
};

/*
 *  dump_hierarchy(uint64_t, const char*, boolean_t)
 *
 *  Writes the class hierarchy for kernel/KEXT ar address `addr`,
 *  in DOT format, at file `path`. If `overwrite` is set, it overwrites
 *  the file content. Otherwise, it appends the hierarchy at the end of 
 *  the file.
 *
 */
uint32_t dump_hierarchy(task_t kport, uint64_t read_addr, const char *path, boolean_t overwrite);

#endif /* dumper_h */
