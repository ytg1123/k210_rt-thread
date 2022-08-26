/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

#include <rtthread.h>
#if defined(RT_USING_SPI_MSD) && defined(RT_USING_DFS_ELMFAT)
#include <spi_msd.h>
#include <dfs_fs.h>
#include <dfs_file.h>
#include <dfs_private.h>

int mnt_init(void)
{
    msd_init("sd0", "spi10");
    if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
    {
        rt_kprintf("Mount \"/dev/sd0\" on \"/\"\n");
    }

    // dfs_file test code
    // all comment out 
    //struct dfs_fd *f;
    //unsigned int len,ret;
    //char buff[1024];
    //ret = dfs_file_open(f,"boot.py",O_RDONLY) ;
    //rt_kprintf("dfs_file_open return(%x)\n",ret);
    //if ( ret < 0)
    //{
    //return 1;
    //}
    //len = dfs_file_read(f,buff,100);
    //if(len>0)rt_kprintf("dfs_file_read OK(%d)\n",len);
    //else rt_kprintf("dfs_file_read NG(%d)\n",len);
    //dfs_file_close(f);
    //rt_kprintf("dfs_file_closed \n");

    return 0;

}

INIT_ENV_EXPORT(mnt_init);

#endif

