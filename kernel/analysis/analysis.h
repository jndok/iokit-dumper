//
//  analysis.h
//  tree-parser-priv-final
//
//  Created by jndok on 29/12/15.
//  Copyright © 2015 jndok. All rights reserved.
//

#ifndef analysis_h
#define analysis_h

#include "../../helper.h"
#include "../kernel.h"

/***/

__attribute__((always_inline)) MetaClass_Object_t *parse_MetaClass_object(task_t task, uint64_t addr);
__attribute__((always_inline)) OSSymbol_Object_t *parse_OSSymbol_object(task_t task, uint64_t addr);

__attribute__((always_inline)) char *getOSSymbolName(task_t task, OSSymbol_Object_t *object);
__attribute__((always_inline)) uint32_t getOSSymbolNameLength(OSSymbol_Object_t *object);

__attribute__((always_inline)) uint64_t calculate_gMetaClass_addr_from_getMetaClass(task_t task, uint64_t getMetaClass_method);

/***/

#endif /* analysis_h */
