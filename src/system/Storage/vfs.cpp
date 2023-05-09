//
//    LunokWatch, a open source smartwatch software
//    Copyright (C) 2022,2023  Jordi Rubió <jordi@binarycell.org>
//    This file is part of LunokWatch.
//
// LunokWatch is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software 
// Foundation, either version 3 of the License, or (at your option) any later 
// version.
//
// LunokWatch is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
// details.
//
// You should have received a copy of the GNU General Public License along with 
// LunokWatch. If not, see <https://www.gnu.org/licenses/>. 
//

//#include <Arduino.h>
#include "../../app/LogView.hpp"
#include <FS.h>
#include "vfs.hpp"
#include <esp_vfs.h>
#include <sys/types.h>
SemaphoreHandle_t lfsMutex = xSemaphoreCreateMutex();
/*
int lfsHighDescriptor=-1; // max = FD_SETSIZE - 1
typedef struct  {
    const char * path;
    int flags;
    int mode;
} lfsDescriptor;
lfsDescriptor lfsDescriptors[FD_SETSIZE - 1];
*/
ssize_t lunokfs_write(int fd, const void * data, size_t size) {
    lLog("LUNOKIOT WRITE\n");
    return 0;
}

int lunokfs_open(const char * path, int flags, int mode) {
    //flags O_APPEND
    //#define	O_RDONLY	0		/* +1 == FREAD */
    //#define	O_WRONLY	1		/* +1 == FWRITE */
    //#define	O_RDWR		2		/* +1 == FREAD|FWRITE */
    // mode (ignore permissions)
    /*
    lfsHighDescriptor++;
    int myDescriptor=lfsHighDescriptor;
    lfsDescriptors[myDescriptor].path = path;
    lfsDescriptors[myDescriptor].flags = flags;
    lfsDescriptors[myDescriptor].mode = mode;
    return myDescriptor;
    */
    xSemaphoreTake( lfsMutex, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    const char *REAL = "/littlefs";
    char * realPath=(char*)ps_malloc(strlen(path)+strlen(REAL)+1);
    sprintf(realPath,"%s%s",REAL,path);
    lLog("@TODO REMEMBER LEAK ON FOPEN\n");
    int desc = open(realPath,flags,mode);
    lLog("LUNOKIOT FOPEN: '%s' flags: %d mode: %d desc: %d\n", realPath, flags, mode,desc);
    xSemaphoreGive(lfsMutex);
    return desc;
}

int lunokfs_fstat(int fd, struct stat * st) {
    xSemaphoreTake( lfsMutex, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    int res = fstat(fd,st);
    lLog("LUNOKIOT FSTAT: %d\n",fd);
    xSemaphoreGive(lfsMutex);
    return res;
}

int lunokfs_close(int fd) {
    xSemaphoreTake( lfsMutex, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    int res = close(fd);
    lLog("LUNOKIOT FCLOSE: %d\n",fd);
    xSemaphoreGive(lfsMutex);
    return res;
}

ssize_t lunokfs_read(int fd, void * dst, size_t size) {
    xSemaphoreTake( lfsMutex, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    lLog("LUNOKIOT READ: %d to %p size: %u\n", fd, dst, size);
    int res = read(fd,dst,size);
    xSemaphoreGive(lfsMutex);
    return res;
}

off_t lunokfs_lseek(int fd, off_t size, int mode) {
    xSemaphoreTake( lfsMutex, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    lLog("LUNOKIOT LSEEK: %d size: %p mode: %u\n", fd, size, mode);
    off_t res = lseek(fd,size,mode);
    xSemaphoreGive(lfsMutex);
    return res;
}
int lunokfs_stat(const char * path, struct stat * st) {
    xSemaphoreTake( lfsMutex, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    lLog("LUNOKIOT STAT: '%s'\n", path);
    int res = stat(path,st);
    xSemaphoreGive(lfsMutex);
    return res;
}

int lunokfs_link(const char* n1, const char* n2) {
    xSemaphoreTake( lfsMutex, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    lLog("LUNOKIOT Link: '%s'->'%s'\n", n1,n2);
    int res = link(n1,n2);
    xSemaphoreGive(lfsMutex);
    return res;
}

int lunokfs_unlink(const char *path) {
    xSemaphoreTake( lfsMutex, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    lLog("LUNOKIOT UNLink: '%s'\n",path);
    int res = unlink(path);
    xSemaphoreGive(lfsMutex);
    return res;
}
DIR* lunokfs_opendir(const char* name) {
    xSemaphoreTake( lfsMutex, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    DIR * res = opendir(name);
    lLog("LUNOKIOT Opendir: '%s' %p\n",name,res);
    xSemaphoreGive(lfsMutex);
    return res;
}

int lunokfs_closedir(DIR* pdir) {
    xSemaphoreTake( lfsMutex, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    lLog("LUNOKIOT Closedir: %p\n",pdir);
    int res = closedir(pdir);
    xSemaphoreGive(lfsMutex);
    return res;
}

struct dirent* lunokfs_readdir(DIR* pdir) {
    xSemaphoreTake( lfsMutex, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    lLog("LUNOKIOT Readdir: %p\n",pdir);
    struct dirent *res = readdir(pdir);
    xSemaphoreGive(lfsMutex);
    return res;
}

long lunokfs_telldir(DIR* pdir) {
    xSemaphoreTake( lfsMutex, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    lLog("LUNOKIOT Telldir: %p\n",pdir);
    long res = telldir(pdir);
    xSemaphoreGive(lfsMutex);
    return res;
}
void lunokfs_seekdir(DIR* pdir, long offset) {
    xSemaphoreTake( lfsMutex, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    lLog("LUNOKIOT Seekdir: %p off: %ld\n",pdir,offset);
    seekdir(pdir,offset);
    xSemaphoreGive(lfsMutex);
}
int lunokfs_mkdir(const char* name, mode_t mode) {
    xSemaphoreTake( lfsMutex, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    lLog("LUNOKIOT mkdir: '%s' mode: %u\n",name,mode);
    int res = mkdir(name, mode);
    xSemaphoreGive(lfsMutex);
    return res;
}

int lunokfs_rmdir(const char* name) {
    xSemaphoreTake( lfsMutex, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    lLog("LUNOKIOT rmdir: '%s'\n",name);
    int res = rmdir(name);
    xSemaphoreGive(lfsMutex);
    return res;
}

void VFSInit() {
    lSysLog("VFS: lunokFS start\n");
    esp_vfs_t lunokFS;
    lunokFS.flags = ESP_VFS_FLAG_DEFAULT;
    lunokFS.write = &lunokfs_write;
    lunokFS.open = &lunokfs_open;
    lunokFS.fstat = &lunokfs_fstat;
    lunokFS.close = &lunokfs_close;
    lunokFS.read = &lunokfs_read;
    lunokFS.lseek = &lunokfs_lseek;
    lunokFS.stat = &lunokfs_stat;
    lunokFS.link = &lunokfs_link;
    lunokFS.unlink = &lunokfs_unlink;
    lunokFS.opendir = &lunokfs_opendir;
    lunokFS.closedir = &lunokfs_closedir;
    lunokFS.readdir = &lunokfs_readdir;
    lunokFS.telldir = &lunokfs_telldir;
    lunokFS.seekdir = &lunokfs_seekdir;
    lunokFS.mkdir = &lunokfs_mkdir;
    lunokFS.rmdir = &lunokfs_rmdir;

    esp_vfs_register("/lunokIoT", &lunokFS, NULL);
    lSysLog("VFS: lunokFS loaded\n");
    /*
    FILE * desc =fopen("/lunokIoT/littlefs/test.txt","r+");
    const char * testDataW="Hello world\n";
    char * readbuff=(char*)ps_malloc(4096);
    fwrite(testDataW, strlen(testDataW),sizeof(char),desc);
    fread(readbuff,strlen(testDataW),sizeof(char),desc);
    fclose(desc);
    free(readbuff);
    */
    lSysLog("VFS: lunokFS TEST END\n");

}
