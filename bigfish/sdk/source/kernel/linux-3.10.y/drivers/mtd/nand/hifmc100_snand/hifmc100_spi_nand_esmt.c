/*****************************************************************************
 *    Copyright (c) 2014 by Hisilicon
 *    All rights reserved.
 *****************************************************************************/

/*****************************************************************************/
/*
  ESMT spi nand don't support QE enable.
*/
static int spi_nand_esmt_quad_enable(struct hifmc_op *spi)
{
	return 0;
}

