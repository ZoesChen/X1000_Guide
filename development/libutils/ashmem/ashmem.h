/* cutils/ashmem.h
 **
 ** Copyright 2008 The Android Open Source Project
 **
 ** This file is dual licensed.  It may be redistributed and/or modified
 ** under the terms of the Apache 2.0 License OR version 2 of the GNU
 ** General Public License.
 */

#ifndef _CUTILS_ASHMEM_H
#define _CUTILS_ASHMEM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ashmem_create_region / create ashmem region the same as android
 *
 * @param name:the ashmem name ,warning it cann't seem as ipc key
 * @param size:the size of ashmem
 *
 *
 * @return ashmen file descriptor
*/
int ashmem_create_region(const char *name, size_t size);

/**
 * @brief ashmem_set_prot_region / set ashmem region prot
 *
 * @param fd :ashmen file descriptor
 * @param prot :seem as vm_flags
 *
 * @return 0 success
 */
int ashmem_set_prot_region(int fd, int prot);

/**
 * @brief ashmem_pin_region / you can pin some pages and it can't restore by system memory recovery
 *
 * @param fd : ashmen file descriptorashmen file descriptor
 * @param offset : the offset of ashmem region,and it page-aligned auto
 * @param len : the len of pin region
 *
 * @return 0 success
 */
int ashmem_pin_region(int fd, size_t offset, size_t len);

/**
 * @brief ashmem_unpin_region / opposite operation with pin.the region can be mananged by system memory recovery
 *
 * @param fd
 * @param offset
 * @param len
 *
 * @return 
 */
int ashmem_unpin_region(int fd, size_t offset, size_t len);
int ashmem_get_size_region(int fd);

#ifdef __cplusplus
}
#endif

#ifndef __ASHMEMIOC	/* in case someone included <linux/ashmem.h> too */

#define ASHMEM_NAME_LEN		256

#define ASHMEM_NAME_DEF		"dev/ashmem"

/* Return values from ASHMEM_PIN: Was the mapping purged while unpinned? */
#define ASHMEM_NOT_PURGED	0
#define ASHMEM_WAS_PURGED	1

/* Return values from ASHMEM_UNPIN: Is the mapping now pinned or unpinned? */
#define ASHMEM_IS_UNPINNED	0
#define ASHMEM_IS_PINNED	1

#endif	/* ! __ASHMEMIOC */

#endif	/* _CUTILS_ASHMEM_H */
