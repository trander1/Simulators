// Copyright (C) 2003 Project Majingaa. (http://majingaa.dyndns.org/)
// This file is part of majingaa-hos
// majingaa-hos is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
// majingaa-hos is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with majingaa-hos; see the file COPYING.  If not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA. 
/*************************************************************
 * ring.c -- Ring Buffer Library
 *************************************************************/
//			February 5th, 2006		Modify return structure
//			July      26th, 2009	for PIC24USB
//			April     24th, 2010	for mbed board / NXP LPC1768
//			May        1st, 2010

#include <stdint.h>
#include "ring.h"

uint16_t ring_init (ring_t *ring, uint16_t *buff, uint16_t size){	
	ring->buff = buff;
	ring->size = size;
	ring->head = ring->tail = 0;
	return 0;
}

uint16_t ring_is_full (ring_t *ring){
	if (ring->tail == ring->head-1 ||
		(ring->tail == ring->size-1 && ring->head == 0)) {
		return 1;
	}else {
		return 0;
	}
}

uint16_t ring_is_empty (ring_t *ring){
	if (ring->head == ring->tail){
		return 1;
	}
	return 0;
}

uint16_t ring_putc (ring_t *ring, uint16_t c){
	if (ring_is_full(ring)){
		return 1;
	}
	ring->buff[ring->tail++] = c;
	if (ring->tail == ring->size){
		ring->tail = 0;
	}
	return 0;
}

uint16_t ring_getc (ring_t *ring){
	if (ring_is_empty(ring)){
		return 1;
	}
	ring->dt_got = ring->buff[ring->head++];
	if (ring->head == ring->size){
		ring->head = 0;
	}
	if (ring->head == ring->tail) {	// Initialize ring
		ring->head = ring->tail = 0;
	}
	return ring->dt_got;
}

uint16_t ring_get_capacity (ring_t *ring){
	if (ring->head < ring->tail) {
		return ring->tail - ring->head;
	} else {
		return ring->head - ring->tail;
	}
}

void ring_clear (ring_t *ring){
	ring->head = ring->tail = 0;
}
