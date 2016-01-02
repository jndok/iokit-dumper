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
#include <unistd.h>
#include <getopt.h>

#include "import.h"

extern uint64_t kslide;
extern uint64_t KextUnslidBaseAddress(const char *KextBundleName);

void usage(void)
{
    fprintf(stderr, "Usage: ./iokit-dumper [-o] [-a kernel/[kext_bundle_id]] [-p output_path]\n");
}

int main(int argc, char *argv[]) {

    int option=0;

    char *name=NULL;
    uint64_t read_addr=0;
    char *path=NULL;
    boolean_t override=FALSE;

    while ((option = getopt(argc, argv, "oa:p:")) != -1) {
        switch (option) {
            case 'a':
                if (strcmp(optarg, "kernel") == 0) {
                    read_addr=0xffffff8000200000;
                } else {
                    read_addr=KextUnslidBaseAddress(optarg);
                    if (read_addr == 0) {
                        __dbg("(!) Invalid read address. Must be a KEXT bundle ID or the \'kernel\' string.");
                        return 1;
                    }
                }
                name=optarg;
                break;
            case 'p':
                if (open(optarg, O_RDONLY) < 0) {
                    __dbg("(!) File was not found on disk. Creating it now.");
                    if(open(optarg, O_CREAT|O_RDWR, 0666) < 0) {
                        __dbg("(!) Unable to create file. Aborting.");
                        return 1;
                    }
                }
                path=optarg;
                break;
            case 'o':
                override=TRUE;
                break;
            default:
                break;
        }
    }

    if (!read_addr || !path) {
        usage();
        return 0;
    }

    __dbg("(+) Will dump hierarchy for: %#llx (%s), @ path %s. Override: %d", read_addr, name, path, override);

    SET_KERNEL_SLIDE(kslide);
    mach_port_t kport = get_kernel_task();
    if (!kport) {
        return -2;
    }

    return dump_hierarchy(kport, read_addr, name, path, override);
}
