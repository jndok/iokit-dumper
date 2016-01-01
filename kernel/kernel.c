//
//  kernel.c
//  tree-parser-priv-final
//
//  Created by jndok on 29/12/15.
//  Copyright © 2015 jndok. All rights reserved.
//

#include "kernel.h"

uint64_t kslide=0;

uint64_t get_kslide(void)
{
    if (getuid() != 0) {
        return -1;
    }
    
    uint64_t _kslide=0;
    uint64_t _kslide_sz=sizeof(kslide);
    
    syscall(SYS_kas_info, KAS_INFO_KERNEL_TEXT_SLIDE_SELECTOR, &_kslide, &_kslide_sz);
    
    return _kslide;
}

/*
 *  Credits to Jonathan Levin!
 *  This function is able to "exploit" the processor_set_tasks mach trap to retrieve
 *  the kernel's mach port. This only works on systems where SIP is either disabled or
 *  not present at all.
 *  If you want to use an alternative method to read from kernel memory, try with enabling
 *  /dev/kmem and read from that device.
 *
 *  Note that code in this project is tuned to read directly from kernel task, hence you 
 *  just need to disable SIP and you're good to go, instead of rewriting code to support
 *  /dev/kmem.
 */
mach_port_t task_for_pid_workaround(int Pid)
{
    
    host_t        myhost = mach_host_self();
    mach_port_t   psDefault;
    mach_port_t   psDefault_control;
    
    task_array_t  tasks;
    mach_msg_type_number_t numTasks;
    int i;
    
    kern_return_t kr;
    
    kr = processor_set_default(myhost, &psDefault);
    
    kr = host_processor_set_priv(myhost, psDefault, &psDefault_control);
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "host_processor_set_priv failed with error %x\n", kr);
        mach_error("host_processor_set_priv",kr);
        exit(1);
    }
    
    kr = processor_set_tasks(psDefault_control, &tasks, &numTasks);
    if (kr != KERN_SUCCESS) {
        fprintf(stderr,"processor_set_tasks failed with error %x\n",kr); exit(1);
    }
    
    for (i = 0; i < numTasks; i++)
    {
        int pid;
        pid_for_task(tasks[i], &pid);
        if (pid == Pid) return (tasks[i]);
    }
    
    return (MACH_PORT_NULL);
}

mach_port_t get_kernel_task(void)
{
    if (getuid() != 0) {
        __dbg("Program should be run as root.");
        return 0;
    }
    
    mach_port_t ktask = task_for_pid_workaround(0);
    if (!ktask) {
        __dbg("processor_set_tasks() failed. is SIP enabled?");
        return 0;
    }
    
    int32_t pid;
    pid_for_task(ktask, &pid);
    if (pid!=0) {
        __dbg("kernel task was returned by processor_set_tasks(), but it appears to have the wrong PID. If you are reading this, the universe is probably collapsing.");
        return 0;
    }
    
    return ktask;
}

void *read_kernel_memory(task_t task, vm_offset_t addr, uint32_t size)
{
    void* mem = malloc(size);
    mach_vm_size_t sz = 0;
    mach_vm_read_overwrite(task, addr, size, (mach_vm_address_t)mem, (mach_vm_size_t*)&sz);
    
    if (!mem) {
        __dbg("(!) read failed.");
        return NULL;
    }
    
    return mem;
}

__attribute__((always_inline)) void read_kernel_memory_in_buffer(task_t task, vm_offset_t addr, uint32_t size, void *buffer)
{
    mach_vm_size_t sz = 0;
    mach_vm_read_overwrite(task, addr, size, (mach_vm_address_t)buffer, (mach_vm_size_t*)&sz);
    
    if (!buffer) {
        __dbg("(!) read failed.");
        return;
    }
}