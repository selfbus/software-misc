/** \file NandFlash.c
 *  \brief NAND Flash driver library. 
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	This module contains functions for the external NAND Flash device to 
 *	- initialize the memory device,
 *	- read data from Flash,
 *	- erase and write,
 *	- copy data from a SD Card file
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *  Copyright (c) 2013 Stefan Haller <stefanhaller.sverige@gmail.com>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */

#include "NandFlash.h"
#include "FATSingleOpt/dos.h"


// Global Variable
static uint8_t flash_qfi_mode;

/**
 * \brief Writes data into external NAND Flash memory.
 * \sa file_2_nand_flash()
 * \param sector Flash sector number
 * \param start start offset address in sector
 * \param size amount of WORD to write into Flash
 * \param data pointer to data array
 * \return Number of Flash sectors written
 *
 */
int write_nand_flash (uint8_t sector, uint16_t offset, uint16_t size, uint8_t* data)
{
uint16_t	ws, i;
	
	if (offset+size <= FLASH_SECTOR_SIZE)
		ws = size;
	else
		ws = FLASH_SECTOR_SIZE - offset;

	/* write data from buffer into Flash memory */
	i = 0;
	while (ws) {
		/* enable write permission */
		FLASH_SELECT_SECTOR (0);
		OUTB(CPLD_BASE_ADDR + UPPER_DATA_WR_ADDR, 0x00);
		OUTB((FLASH_BASE_ADDRESS + 0x555), 0xAA);
		OUTB((FLASH_BASE_ADDRESS + 0x2AA), 0x55);
		OUTB((FLASH_BASE_ADDRESS + 0x555), 0xA0);

		// write data (lb/hb)
		FLASH_SELECT_SECTOR (sector);
		OUTB(CPLD_BASE_ADDR + UPPER_DATA_WR_ADDR, (*data++));
		OUTB(FLASH_BASE_ADDRESS + offset++, (*data++));

		/* poll for ready signal from Flash */
		//FIXME: should allow escape path on timeout
		while (!FLASH_READY_STATE);
		ws--; i++;
	}
	return i;
}

/**
 * \brief Erases one sector of the external Flash memory.
 * \param sector Flash sector number
 */
void erase_flash_sector (uint8_t sector)
{
	/* issue erase command */
	FLASH_SELECT_SECTOR (0);
	OUTB(CPLD_BASE_ADDR + UPPER_DATA_WR_ADDR, 0x00);
	OUTB((FLASH_BASE_ADDRESS + 0x555), 0xAA);
	OUTB((FLASH_BASE_ADDRESS + 0x2AA), 0x55);
	OUTB((FLASH_BASE_ADDRESS + 0x555), 0x80);
	OUTB((FLASH_BASE_ADDRESS + 0x555), 0xAA);
	OUTB((FLASH_BASE_ADDRESS + 0x2AA), 0x55);
	FLASH_SELECT_SECTOR (sector);
	OUTB(FLASH_BASE_ADDRESS, 0x30);

	/* poll for ready signal from Flash */
	//FIXME: should allow escape path on timeout
	while (!FLASH_READY_STATE);
}

/**
 * \brief Erases the external Flash Chip memory.
 *  takes approx 64-128sec. for a S29GL064N
 */
void erase_flash_chip (void)
{
	/* issue erase command */
	FLASH_SELECT_SECTOR (0);
	OUTB(CPLD_BASE_ADDR + UPPER_DATA_WR_ADDR, 0x00);
	OUTB((FLASH_BASE_ADDRESS + 0x555), 0xAA);
	OUTB((FLASH_BASE_ADDRESS + 0x2AA), 0x55);
	OUTB((FLASH_BASE_ADDRESS + 0x555), 0x80);
	OUTB((FLASH_BASE_ADDRESS + 0x555), 0xAA);
	OUTB((FLASH_BASE_ADDRESS + 0x2AA), 0x55);
	OUTB((FLASH_BASE_ADDRESS + 0x555), 0x10);

	/* poll for ready signal from Flash */
	//FIXME: should allow escape path on timeout
	while (!FLASH_READY_STATE);
}


/**
 * \brief Sends the reset command to the external Flash Chip memory
 */
void reset_flash_chip (void)
{
	flash_qfi_mode = 0;
	/* issue erase command */
	FLASH_SELECT_SECTOR (0);
	OUTB(CPLD_BASE_ADDR + UPPER_DATA_WR_ADDR, 0x00);
	OUTB((FLASH_BASE_ADDRESS + 0x555), 0xF0);

	/* poll for ready signal from Flash */
	//FIXME: should allow escape path on timeout
	while (!FLASH_READY_STATE);
}


/**
 * \brief Reads the CFI Query Information
 * \param address QFI Flash address to read
 */
uint8_t read_flash_qfi_info (uint8_t address)
{
	uint8_t hb;

	// Enter QFI mode if not done before
	if(!flash_qfi_mode) {
		// enter QFI Query mode
		flash_qfi_mode = 1;
		FLASH_SELECT_SECTOR (0);
		OUTB(CPLD_BASE_ADDR + UPPER_DATA_WR_ADDR, 0x00);
		OUTB((FLASH_BASE_ADDRESS + 0x55), 0x98);
	
		// poll for ready signal from Flash
		//FIXME: should allow escape path on timeout
		while (!FLASH_READY_STATE);
	}

	NutEnterCritical();	// TODO: Do we need this for a single byte reading??
	// read low byte, high byte is not used in QFI mode
	hb = INB(FLASH_BASE_ADDRESS + address);
	NutExitCritical ();

	return hb;
}


 /* \brief Erase amount of blocks in external flash memory
 *         The flash can be erased in blocks of 64K
 *         128 blocks = 64MBit
 * \sa erase_flash_sector()
 * \param blocks Amount of Flash blocks to erase
 * \param start_block First block to erase
 */
uint8_t erase_complete_flash(uint8_t blocks, uint8_t start_block)
{	
	uint8_t erased_blocks;

	if( (start_block+blocks-1) > FLASH_MAX_SECTOR)
	{
		#ifdef LCD_DEBUG
			printf_P(PSTR("\nFlash erase failed, out of range!\n"));
		#endif
		return 1;	// Error out of range
	}
	
	/* invalidate Flash content to prevent any function accessing inconsistent Flash data */
	set_flash_content_invalid();

	/* enable Flash wait */
	XMCRA |= (1<<SRW11); // wait
	MCUCR |= (1<<SRW10); // wait
	
	init_download_progress(blocks);
	
	for(erased_blocks=0; erased_blocks <= blocks; erased_blocks++)
	{
		show_erase_progress(erased_blocks);
		// Zap Block
		erase_flash_sector (start_block+erased_blocks);	
	}

	/* disable Flash wait */
	XMCRA &= 0xff ^ (1<<SRW11); // no wait
	MCUCR &= 0xff ^ (1<<SRW10); // no wait

	return 0; /*! erase successfully completed */
}


/**
 * \brief Moves contents of a SD Card file into the external Flash memory.
 * \sa write_nand_flash()
 * \sa erase_flash_sector()
 * \param *filename File name string
 * \param start_address Absolute Flash memory address, at which the SD Card file image should start in the Flash memory.
 * \return 0=ok, 1=file error, 2=out of mem
 *
 */
uint8_t file_2_nand_flash ( char *filename, uint32_t start_address )
{
#define FILE_READ_BUFF_SIZE	512
uint8_t * buffer; /*! pointer to the file data buffer */
uint8_t * bptr;
int16_t read_bytes, written_words;
uint16_t flash_address; 
uint8_t	 flash_sector, last_sector;
uint32_t downloadtotal; /*! total file size to be downloaded into the Flash memory */
unsigned char result;

  	downloadtotal = 0;

	/* try to open the file */
	result=Fopen(filename,F_READ);
	if(result!=F_OK) {
		return 1; /*! file open error */
	}

	/* allocate memory to store the data read from SD card */
	buffer = (uint8_t*) malloc (FILE_READ_BUFF_SIZE);
	if (!buffer) {
   		Fclose(); /* close the file */
		return 2; /*! out of memory */
	}

	/* invalidate Flash content to prevent any function accessing inconsistent Flash data */
	set_flash_content_invalid();

	/* enable Flash wait */
	XMCRA |= (1<<SRW11); // wait
	MCUCR |= (1<<SRW10); // wait

	last_sector = 0xff;
	while (result==F_OK) {
				
		read_bytes = Fread(buffer,FILE_READ_BUFF_SIZE);
		downloadtotal += read_bytes;
		show_download_progress (downloadtotal);
		if (read_bytes < FILE_READ_BUFF_SIZE) {
           result=F_ERROR; // end of file reached ?
		}
		if (read_bytes > 0) {
			bptr = buffer;
			while (read_bytes) {
				/* calculate Flash address */
				flash_address = start_address & 0x7FFF;
				flash_sector = (start_address >> 15) & 0x7F;
				// erase new sector if used now
				if (flash_sector != last_sector) {
					erase_flash_sector (flash_sector);
					last_sector = flash_sector;
				}

				// write buffer to Flash (word count)
				written_words = write_nand_flash (flash_sector, flash_address, (read_bytes+1) >> 1, bptr);
				if (written_words << 1 > read_bytes) {
					read_bytes = 0;
					result = F_ERROR;
				}
				else
					read_bytes -= written_words << 1;
				start_address += written_words;
				bptr += written_words << 1;
			}
		}
	}

	/* close file and release data buffer */
   	Fclose();
	free (buffer);

	/* disable Flash wait */
	XMCRA &= 0xff ^ (1<<SRW11); // no wait
	MCUCR &= 0xff ^ (1<<SRW10); // no wait
	
	return 0; /*! download successfully completed */
}

/**
 * \brief Reads a 16 bit value from the external Flash memory (sector/offset).
 * \sa read_flash_int()
 * \sa read_flash_abs()
 * \param sector Flash memory sector
 * \param offset Offset address in the selected sector
 * \return 16 bit memory data, swapped bytes
 *
 * This functions reads a 16 bit memory word from the specified address. Since the external memory
 * is organized in words of 16 bits, a full memory word is read. The offset address specifies the
 * memory word address, which is half of the respective byte address. Low- and high-bytes are swapped
 * by this function to correspond to the little endianess of gcc:
 * Flash(addr)[16:0] => [7:0][16:8]
 */
uint16_t read_flash (uint8_t sector, uint16_t offset) {

uint8_t	lb;
uint16_t hb;

	// select sector
	FLASH_SELECT_SECTOR (sector);
	// disable dma functions
	OUTB (CPLD_BASE_ADDR + MODE_CTRL_ADDR, 0x00);

	NutEnterCritical();
	// read low byte
	hb = INB(FLASH_BASE_ADDRESS + offset);
	// read high byte
	lb = INB(CPLD_BASE_ADDR + UPPER_DATA_RD_ADDR); 

	NutExitCritical ();

	return lb | (hb << 8);
}

/**
 * \brief Reads a 16 bit value from the external Flash memory (sector/offset). Can be called from Interrupt function.
 * \sa read_flash()
 * \sa read_flash_abs()
 * \param sector Flash memory sector
 * \param offset is 8000..0xffff, this function does NOT add the FLASH_BASE_ADDRESS
 * \return 16 bit memory data, swapped bytes.
 *
 * Reads 16 bit value from Flash (sector/offset). Since it saves and restores the currently selected DMA mode and Flash page,
 * it is save to call read_flash_int() from an interrupt function. Please note that the offset address has to consider the
 * Flash base address of 0x8000 already, since this function does not add it!
 *
 * This functions reads a 16 bit memory word from the specified address. Since the external memory
 * is organized in words of 16 bits, a full memory word is read. The offset address specifies the
 * memory word address, which is half of the respective byte address. Low- and high-bytes are swapped
 * by this function to correspond to the little endianess of gcc:
 * Flash(addr)[16:0] => [7:0][16:8]
 */
uint16_t read_flash_int (uint8_t sector, uint16_t offset) {

uint8_t	dma_setting_save;
uint8_t	sector_setting_save;
uint8_t	lb;
uint16_t hb;

	/* save currently selected Flash sector and DMA mode */
	sector_setting_save = FLASH_RETURN_SECTOR;
	dma_setting_save = INB (CPLD_BASE_ADDR + MODE_CTRL_ADDR);

	/* select the target Flash sector */
	FLASH_SELECT_SECTOR (sector);
	/* disable DMA functions during data read */
	OUTB (CPLD_BASE_ADDR + MODE_CTRL_ADDR, 0x00);
	/* read high byte */
	hb = INB(offset);
	/* read low byte */
	lb = INB(CPLD_BASE_ADDR + UPPER_DATA_RD_ADDR); 
	/* restore Flash sector and DMA settings */
	OUTB (CPLD_BASE_ADDR + MODE_CTRL_ADDR, dma_setting_save);
	FLASH_SELECT_SECTOR (sector_setting_save);

	/* return read value */
	return lb | (hb << 8);
}

/**
 * \brief Reads a 16 bit value from the external Flash memory (32 bit linear address)
 * \sa read_flash()
 * \sa read_flash_int()
 * \param addr this is the absolute 32 bit linear byte address, from which the 16 bit memory word will be returned.
 * \return 16 bit memory data, swapped bytes
 *
 * This functions reads a 16 bit memory word from the specified address. It supports even byte addresses only.
 * An odd byte address will be truncated to the respective even address. Low- and high-bytes are swapped
 * by this function to correspond to the little endianess of gcc:
 * Flash(addr)[16:0] => [7:0][16:8]
 */
uint16_t read_flash_abs (uint32_t addr){
uint8_t sector;
uint16_t offset;

	/* calculate Flash sector and offset */
	sector = FLASH_GET_SECTOR(addr);
	offset = FLASH_GET_OFFSET(addr);

	return read_flash (sector, offset);
}

/**
 * \brief intializes the Flash memory control
 *
 * Configures Flash control GPIO pins for Reset output and Busy input.
 * Issues Flash RESET as a precaution to terminate ongoing programming cycles.
 * Waits for the Flash memory to become ready for access by the ATMEGA.
 * Preselects the first Flash sector.
 */
void init_nand_flash ()
{
	// Not in QFI mode
	flash_qfi_mode = 0;
	
	/* init Flash control pins */
	INIT_FLASH_RESET
	INIT_FLASH_BUSY
	
	/* Reset Flash state machine */
	SET_FLASH_RESET_ACTIVE
	FLASH_500ns_DELAY
	SET_FLASH_RESET_INACTIVE
	/* wait until Flash is ready. Max 20us */
	//FIXME: should allow escape path on timeout
	while (!FLASH_READY_STATE);
	/* select Flash bank 0 */
	FLASH_SELECT_SECTOR (0);
}
