#ifndef _ADF_DEV_HD_H
#define _ADF_DEV_HD_H 1

/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 *
 *  adf_dev_hd.h
 *
 *  $Id$
 *
 *  Harddisk devices code
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

#include"prefix.h"

#include "adf_str.h"
#include "hd_blk.h"

RETCODE adfMountHd(struct Device *dev);
RETCODE adfMountHdFile(struct Device *dev);

RETCODE adfCreateHdHeader(struct Device* dev, int n, struct Partition** partList );
PREFIX RETCODE adfCreateHd(struct Device* dev, int n, struct Partition** partList );
PREFIX RETCODE adfCreateHdFile(struct Device* dev, char* volName, int volType);

RETCODE adfReadRDSKblock( struct Device* dev, struct bRDSKblock* blk );
RETCODE adfWriteRDSKblock(struct Device *dev, struct bRDSKblock* rdsk);
RETCODE adfReadPARTblock( struct Device* dev, int32_t nSect, struct bPARTblock* blk );
RETCODE adfWritePARTblock(struct Device *dev, int32_t nSect, struct bPARTblock* part);
RETCODE adfReadFSHDblock( struct Device* dev, int32_t nSect, struct bFSHDblock* blk);
RETCODE adfWriteFSHDblock(struct Device *dev, int32_t nSect, struct bFSHDblock* fshd);
RETCODE adfReadLSEGblock(struct Device* dev, int32_t nSect, struct bLSEGblock* blk);
RETCODE adfWriteLSEGblock(struct Device *dev, int32_t nSect, struct bLSEGblock* lseg);


#endif /* _ADF_HD_H */

/*##########################################################################*/
