/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for Petit FatFs (C)ChaN, 2014      */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "../memoryCard.h"

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (void)
{
	if (memCard_initCard())
        return 0x00;

	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Partial Sector                                                   */
/*-----------------------------------------------------------------------*/

DRESULT disk_readp (
	BYTE* buff,		/* Pointer to the destination object */
	DWORD sector,	/* Sector number (LBA) */
	UINT offset,	/* Offset in the sector */
	UINT count		/* Byte count (bit15:destination) */
)
{
	if (!memCard_readFromDisk(sector, offset, buff, count))
    {
        return RES_ERROR;
    }

	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Partial Sector                                                  */
/*-----------------------------------------------------------------------*/

DRESULT disk_writep (
	BYTE* buff,		/* Pointer to the data to be written, NULL:Initiate/Finalize write operation */
	DWORD sc		/* Sector number (LBA) or Number of bytes to send */
)
{
	DRESULT res = RES_OK;


	if (!buff) {
		if (sc) {

			// Initiate write process
            if (!memCard_prepareWrite(sc))
            {
                res = RES_NOTRDY;
            }

		} else {

			// Finalize write process
            if (memCard_writeBlock() != CARD_NO_ERROR)
            {
                res = RES_ERROR;
            }
		}
	} else {

		// Send data to the disk
        if (!memCard_queueWrite(buff, sc))
        {
            res = RES_ERROR;
        }

	}

	return res;
}

