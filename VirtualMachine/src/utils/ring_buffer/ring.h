// Copyright (C) 2003 Project Majingaa. (http://majingaa.dyndns.org/)
// This file is part of majingaa-hos
// majingaa-hos is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
// majingaa-hos is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with majingaa-hos; see the file COPYING.  If not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
/*************************************************************
 * ring.h -- Ring buffer library
 *************************************************************/
//	February 5th, 2006		Add ret_dt structure
//	June     29th, 2009		change type explanation
//	July     25th, 2009		for PIC24USB
//  April    24th, 2010		for mbed board / NXP LPC1768
//	September 19th, 2010

#ifndef _RING_H_
#define _RING_H_

#include "queue.h"
// Buffer size
#define SIZE_COMBUF_S	64
#define SIZE_COMBUF_B	128
#define SIZE_COMBUF_H	512

typedef struct {
	uint16_t   	*buff;
	uint16_t	size;
	uint16_t	head;
	uint16_t	tail;
	uint8_t 	dt_got;
} ring_t;

uint16_t	ring_init (ring_t *ring, uint16_t *buff, uint16_t size);
uint16_t	ring_is_full (ring_t *ring);
uint16_t	ring_is_empty (ring_t *ring);
uint16_t	ring_putc (ring_t *ring, uint16_t c);
uint16_t	ring_getc (ring_t *ring);
uint16_t	ring_get_capacity (ring_t *ring);
void		ring_clear (ring_t *ring);

#endif

