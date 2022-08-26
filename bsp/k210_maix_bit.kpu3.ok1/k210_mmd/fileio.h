#ifndef _FILE_IO_H
#define _FILE_IO_H

#include "3dconfig.hpp"
#if defined(PC)
typedef void* fil;
#include <unistd.h>
#elif defined(USE_K210)
 #include "ff.h"
#include <dfs_fs.h>
#include <dfs_file.h>
//struct dfs_fd fil;
#endif

#if defined(__cplusplus)
extern "C"{
#endif
#if 0
}
#endif
//file open
//return 0 if success
int filopen(const char *pathname,struct dfs_fd *f) ;

//file read
int filread(struct dfs_fd *fp,void *buff,size_t byte);

//file close
int filclose(struct dfs_fd *fp);

void send_line(int ypos, uint8_t *line);

void lcd_ram_draw_string_width(char *str, uint32_t *ptr, uint16_t font_color, uint16_t bg_color,uint16_t width);

//fail
static inline
void fail(){
  //todo
  while(1)
#if defined(PC)
    usleep(114514);
#else
  ;
#endif
}

#if defined(__cplusplus)
};
#endif

#endif//_FILE_IO_H
