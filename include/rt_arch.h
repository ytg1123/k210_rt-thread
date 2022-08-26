
/****************************************************************************
 * Inline Functions
 ****************************************************************************/

#ifndef __ASSEMBLY__

# define getreg8(a)           (*(volatile rt_uint8_t *)(a))
# define putreg8(v,a)         (*(volatile rt_uint8_t *)(a) = (v))
# define getreg16(a)          (*(volatile rt_uint16_t *)(a))
# define putreg16(v,a)        (*(volatile rt_uint16_t *)(a) = (v))
# define getreg32(a)          (*(volatile rt_uint32_t *)(a))
# define putreg32(v,a)        (*(volatile rt_uint32_t *)(a) = (v))

#endif /* __ASSEMBLY__ */
