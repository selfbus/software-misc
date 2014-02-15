/**
 * \file task.h
 *
 * \brief Thread priority definitions
 * This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */#ifndef _TASK_H_
#define _TASK_H_

/* Thread stack sizes */
#define NUT_THREAD_EIB_TX_STACK 			0x200
#define NUT_THREAD_EIBSERVICE_STACK 		0x200
#define NUT_THREAD_POLL_TOUCH_STACK			0x200

/* Thread priorities */
#define NUT_THREAD_PRIORITY_EIB_LL_SERVICE		50
#define NUT_THREAD_PRIORITY_EIB_TL_SERVICE		55
#define NUT_THREAD_PRIORITY_EIB_SERVE_TX		60
#define NUT_THREAD_PRIORITY_MAIN				70

#endif // _TASK_H_
