#ifndef _newdisk_h_
#define _newdisk_h_

#include "part.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "cache.h"
#include "fs.h"
#include "pathparser.h"
#include "filemt.h"

#define SOCL sizeof(ClusterNo)

#define USE_CACHE

struct Disk
{
    // partition
    Partition* partition;

    // meta data
    struct _meta
    {
        ClusterNo
            freeNode,
            fatSize,
            rootDir,
            rootSize;
    } meta;

    // cached FAT
    ClusterNo* FAT;

    // cache
    CacheLRU cache;

    // opened files
    filemt::OpenedFile* filetable;
    Semaphore un_mountS;
    bool un_mountB;

    Disk(Partition* _p)
        : cache(CACHE_SIZE)
        , un_mountS(CreateSemaphore(0,0,1,0))
        , un_mountB(false)
    {
        char w_buffer[2048];
        ClusterNo* buffer = (ClusterNo*)w_buffer;

        // reading meta data
        _p->readCluster(0, w_buffer);
        meta.freeNode   = buffer[0];
        meta.fatSize    = buffer[1];
        meta.rootDir    = buffer[2];
        meta.rootSize   = buffer[3];

        // storing entire FAT table
        FAT = new ClusterNo[meta.fatSize];
        uint16_t FATClsCnt = (meta.fatSize + 511) / 512;
        uint16_t left = meta.fatSize;
        for (uint16_t i = 0; i < FATClsCnt; i++)
        {
            _p->readCluster(1+i, w_buffer);
            memcpy(FAT + (meta.fatSize - left), w_buffer, SOCL*(left<512 ? left : 512));
            left -= 512;
        }

        // caching some data
        // TODO

        // assign partition
        partition = _p;

        // opened files
        filetable = 0;
    }

    ~Disk();
};

// cluster-wise
int readCluster(Disk& _d, ClusterNo _id, char* _buffer);
int writeCluster(Disk& _d, ClusterNo _id, const char* _buffer);
ClusterNo allocate(Disk& _d);
ClusterNo offset(Disk& _d);

// managment-wise
bool createEntry(Disk& _d, char* _fname);                   // uradjeno
bool getEntry(Disk& _d, Entry& _e, char* _fname);           // uradjeno
bool deleteEntry(Disk& _d, char* _path);                    // uradjeno
void listDir(Disk& _d, Entry& _dir, Entry *& _entries);     // uradjeno

// debug
void tree(Disk& _d, bool info=true);
bool matchName(Entry& e, char* name);


#endif