/*
 *  iokit-dumper –– v1
 *
 *  This tool is useful for dumping the IOKit class hierarchy of the kernel
 *  or any KEXT. It generates a DOT file that can be processed with the dot engine
 *  to create a visual graph.
 *
 *  I am aware that this can be done much more cleanly with scripts
 *  for IDA, but since I don't have IDA and don't know how to script for it,
 *  I decided to do it myself :p
 *
 *  ---
 *
 *  Example usage: ./iokit-dumper com.apple.driver.AppleHDA /Users/$USER/Desktop/test.dot
 */

//
//  main.c
//  tree-parser-priv-final
//
//  Created by jndok on 29/12/15.
//  Copyright © 2015 jndok. All rights reserved.
//

#define SET_KERNEL_SLIDE(kslide) {   \
    if((kslide=get_kslide()) == -1) {   \
        __dbg("(!) Program should be run as root!");    \
        return -1;  \
    }   \
}

#include <stdio.h>
#include <fcntl.h>

#include "import.h"

extern uint64_t kslide;
extern uint64_t KextUnslidBaseAddress(const char *KextBundleName);

int main(int argc, const char * argv[]) {

    if (argc < 3) {
        fprintf(stderr, "Usage: ./exec kernel/[kext_bundle_id] [output_file] [overwrite]\n");
        fprintf(stderr, "\tkernel/[kext_bundle_id]: Pass the \'kernel\' string without quotes to dump the kernel hierarchy. Pass a KEXT bundle ID to dump that KEXT hierarchy.\n");
        fprintf(stderr, "\t[output_file]: Pass the path of the output file, where the hierarchy in DOT format will be written.\n");
        fprintf(stderr, "\t[overwrite]: Boolean. Pass 0 or 1. If the hierarchy in DOT format should be appended to [output_file] or if the content should be overwritten.\n");
        return 1;
    }

    uint64_t READ_BASE=0;
    const char *OUT_FILE=argv[2];
    boolean_t overwrite=TRUE;

    if (strcmp(argv[1], "kernel") == 0) {
        READ_BASE=0xffffff8000200000;
    } else {
        READ_BASE=KextUnslidBaseAddress(argv[1]);
        if (READ_BASE == 0) {
            __dbg("(!) Invalid read address. Must be a KEXT bundle ID or the \'kernel\' string.");
            return 1;
        }
    }

    if (open(argv[2], O_RDONLY) < 0) {
        __dbg("(!) File was not found on disk. Creating it now.");
        if(open(argv[2], O_CREAT|O_RDWR, 0666) < 0) {
            __dbg("(!) Unable to create file. Aborting.");
            return 1;
        }
    }

    if (argv[3] != NULL) {
        if (atoi(argv[3]) == FALSE) {
            overwrite=FALSE;
        } else {
            overwrite=TRUE;
        }
    }

    SET_KERNEL_SLIDE(kslide);
    mach_port_t kport = get_kernel_task();
    if (!kport) {
        return -2;
    }

    return dump_hierarchy(kport, READ_BASE, OUT_FILE, overwrite);;
}
