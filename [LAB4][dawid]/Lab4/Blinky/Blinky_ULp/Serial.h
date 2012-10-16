/*----------------------------------------------------------------------------
 * Name:    Serial.h
 * Purpose: Low level serial definitions
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2010 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#ifndef __SERIAL_H
#define __SERIAL_H
extern int CharAv0(void);
extern int CharAv1(void);

extern void SER_Init0      (void);
extern int  SER_GetChar0   (void);
extern int  SER_PutChar0   (int c);
extern void SER_Init1      (void);
extern int  SER_GetChar1   (void);
extern int  SER_PutChar1   (int c);

#endif
