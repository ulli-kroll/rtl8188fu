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

#if(defined(CONFIG_PATH_DIVERSITY))
#define USE_PATH_A_AS_DEFAULT_ANT   //for 8814 dynamic TX path selection

#define	NUM_RESET_DTP_PERIOD 5
#define	ANT_DECT_RSSI_TH 3 

#define PATH_A 1
#define PATH_B 2
#define PATH_C 3
#define PATH_D 4

#define PHYDM_AUTO_PATH	0
#define PHYDM_FIX_PATH		1

#define NUM_CHOOSE2_FROM4 6
#define NUM_CHOOSE3_FROM4 4


#define		PHYDM_A		 BIT0
#define		PHYDM_B		 BIT1
#define		PHYDM_C		 BIT2
#define		PHYDM_D		 BIT3
#define		PHYDM_AB	 (BIT0 | BIT1)  // 0
#define		PHYDM_AC	 (BIT0 | BIT2)  // 1
#define		PHYDM_AD	 (BIT0 | BIT3)  // 2
#define		PHYDM_BC	 (BIT1 | BIT2)  // 3
#define		PHYDM_BD	 (BIT1 | BIT3)  // 4
#define		PHYDM_CD	 (BIT2 | BIT3)  // 5

#define		PHYDM_ABC	 (BIT0 | BIT1 | BIT2) /* 0*/
#define		PHYDM_ABD	 (BIT0 | BIT1 | BIT3) /* 1*/
#define		PHYDM_ACD	 (BIT0 | BIT2 | BIT3) /* 2*/
#define		PHYDM_BCD	 (BIT1 | BIT2 | BIT3) /* 3*/

#define		PHYDM_ABCD	 (BIT0 | BIT1 | BIT2 | BIT3)


typedef enum dtp_state
{
	PHYDM_DTP_INIT=1,
	PHYDM_DTP_RUNNING_1

}PHYDM_DTP_STATE;

typedef enum path_div_type
{
	PHYDM_2R_PATH_DIV = 1,
	PHYDM_4R_PATH_DIV = 2
}PHYDM_PATH_DIV_TYPE;

VOID
phydm_process_rssi_for_path_div(	
	IN OUT		PVOID			pDM_VOID,	
	IN			PVOID			p_phy_info_void,
	IN			PVOID			p_pkt_info_void
	);

typedef struct _ODM_PATH_DIVERSITY_
{
	u1Byte	RespTxPath;
	u1Byte	PathSel[ODM_ASSOCIATE_ENTRY_NUM];
	u4Byte	PathA_Sum[ODM_ASSOCIATE_ENTRY_NUM];
	u4Byte	PathB_Sum[ODM_ASSOCIATE_ENTRY_NUM];
	u2Byte	PathA_Cnt[ODM_ASSOCIATE_ENTRY_NUM];
	u2Byte	PathB_Cnt[ODM_ASSOCIATE_ENTRY_NUM];
	u1Byte	path_div_type;
}PATHDIV_T, *pPATHDIV_T;


#endif //#if(defined(CONFIG_PATH_DIVERSITY))

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

