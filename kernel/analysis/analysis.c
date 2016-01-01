//
//  analysis.c
//  tree-parser-priv-final
//
//  Created by jndok on 29/12/15.
//  Copyright © 2015 jndok. All rights reserved.
//

#include "analysis.h"

__attribute__((always_inline)) MetaClass_Object_t *parse_MetaClass_object(task_t task, uint64_t addr)
{
    MetaClass_Object_t *MetaClass_object = malloc(sizeof(MetaClass_Object_t));
    void *buffer = read_kernel_memory(task, addr, 64);
    MetaClass_object = (MetaClass_Object_t*)buffer;
    
    return MetaClass_object;
}

__attribute__((always_inline)) OSSymbol_Object_t *parse_OSSymbol_object(task_t task, uint64_t addr)
{
    OSSymbol_Object_t *OSSymbol_object = malloc(sizeof(OSSymbol_Object_t));
    void *buffer = read_kernel_memory(task, addr, 64);
    OSSymbol_object = (OSSymbol_Object_t*)buffer;
    
    return OSSymbol_object;
}

__attribute__((always_inline)) char *getOSSymbolName(task_t task, OSSymbol_Object_t *object)
{
    void *buffer = read_kernel_memory(task, object->name_ptr, (uint32_t)object->name_length);
    return (char*)buffer;
}

__attribute__((always_inline)) uint32_t getOSSymbolNameLength(OSSymbol_Object_t *object)
{
    return (uint32_t)object->name_length;
}

__attribute__((always_inline)) uint64_t calculate_gMetaClass_addr_from_getMetaClass(task_t task, uint64_t getMetaClass_method)
{
    void *method_buffer = read_kernel_memory(task, getMetaClass_method, 32);
    if (*(char*)(method_buffer+12) != (char)0xc3) {
        return 0;
    }
    
    uint32_t relative_off=*(uint32_t*)(method_buffer+7);
    uint64_t absolute_off=getMetaClass_method+11;
    
    return (absolute_off+relative_off);
}