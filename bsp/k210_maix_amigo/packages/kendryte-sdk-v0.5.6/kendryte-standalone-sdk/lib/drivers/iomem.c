
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "iomem.h"
#include "printf.h"
#include "atomic.h"
 
#include "rtthread.h"
 
 
void iomem_free(void *paddr)
{
    rt_free(paddr); 
}
 
void *iomem_malloc(uint32_t size)
{
    return rt_malloc(size);
}
 
uint32_t iomem_unused()
{
    rt_uint32_t total, used, max;
 
    rt_memory_info(&total, &used, &max);
 
    return total - used;
}
