#ifndef _ADF_VOL_H
#define _ADF_VOL_H 1

/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 *
 *  adf_vol.h
 *
 *  $Id$
 *
 *  This file is part of ADFLib.
 *
 *  ADFLib is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  ADFLib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "prefix.h"

#include "adf_str.h"
#include "adf_defs.h"

/* ----- VOLUME ----- */

struct Volume {
    struct Device* dev;

    SECTNUM firstBlock;     /* first block of data area (from beginning of device) */
    SECTNUM lastBlock;      /* last block of data area  (from beginning of device) */
    SECTNUM rootBlock;      /* root block (from firstBlock) */

    char dosType;           /* FFS/OFS, DIRCACHE, INTERNATIONAL */
    BOOL bootCode;
    BOOL readOnly;
    int datablockSize;      /* 488 or 512 */
    int blockSize;			/* 512 */

    char *volName;

    BOOL mounted;

    int32_t bitmapSize;             /* in blocks */
    SECTNUM *bitmapBlocks;       /* bitmap blocks pointers */
    struct bBitmapBlock **bitmapTable;
    BOOL *bitmapBlocksChg;

    SECTNUM curDirPtr;
};


PREFIX RETCODE adfInstallBootBlock(struct Volume *vol,uint8_t*);

PREFIX BOOL isSectNumValid(struct Volume *vol, SECTNUM nSect);

PREFIX struct Volume* adfMount( struct Device *dev, int nPart, BOOL readOnly );
PREFIX void adfUnMount(struct Volume *vol);
PREFIX void adfVolumeInfo(struct Volume *vol);
struct Volume* adfCreateVol( struct Device* dev, int32_t start, int32_t len, 
    char* volName, int volType );

/*void adfReadBitmap(struct Volume* , int32_t nBlock, struct bRootBlock* root);
void adfUpdateBitmap(struct Volume*);
*/
PREFIX RETCODE adfReadBlock(struct Volume* , int32_t nSect, uint8_t* buf);
PREFIX RETCODE adfWriteBlock(struct Volume* , int32_t nSect, uint8_t* buf);

#endif /* _ADF_VOL_H */

/*##########################################################################*/
