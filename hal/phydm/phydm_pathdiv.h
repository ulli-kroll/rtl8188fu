/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *                                        
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
 
#ifndef	__PHYDMPATHDIV_H__
#define    __PHYDMPATHDIV_H__
/*#define PATHDIV_VERSION "2.0" //2014.11.04*/
#define PATHDIV_VERSION	"3.0" /*2015.01.13 Dino*/

VOID
phydm_c2h_dtp_handler(
	 IN	PVOID	pDM_VOID,
	 IN 	pu1Byte   CmdBuf,
	 IN 	u1Byte	CmdLen
	);

VOID
odm_PathDiversityInit(
	IN	PVOID	pDM_VOID
	);

VOID
odm_PathDiversity(
	IN	PVOID	pDM_VOID
	);

VOID
odm_pathdiv_debug(
	IN		PVOID		pDM_VOID,
	IN		u4Byte		*const dm_value,
	IN		u4Byte		*_used,
	OUT		char		*output,
	IN		u4Byte		*_out_len
	);



//1 [OLD IC]--------------------------------------------------------------------------------






 
 
 #endif		 //#ifndef  __ODMPATHDIV_H__

