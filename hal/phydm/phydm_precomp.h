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

#ifndef	__ODM_PRECOMP_H__
#define __ODM_PRECOMP_H__

#include "phydm_types.h"
#include "phydm_features.h"

#define		TEST_FALG___		1

#if (DM_ODM_SUPPORT_TYPE ==ODM_CE) 
#define 	RTL8192CE_SUPPORT 				0
#define 	RTL8192CU_SUPPORT 				0
#define 	RTL8192C_SUPPORT 				0	

#define 	RTL8192DE_SUPPORT 				0
#define 	RTL8192DU_SUPPORT 				0
#define 	RTL8192D_SUPPORT 				0	

#define 	RTL8723AU_SUPPORT				0
#define 	RTL8723AS_SUPPORT				0
#define 	RTL8723AE_SUPPORT				0
#define 	RTL8723A_SUPPORT				0
#define 	RTL8723_FPGA_VERIFICATION		0
#endif

//2 Config Flags and Structs - defined by each ODM Type

#if (DM_ODM_SUPPORT_TYPE ==ODM_CE)
	#define __PACK
	#define __WLAN_ATTRIB_PACK__
#endif

//2 OutSrc Header Files
 
#include "phydm.h" 
#include "phydm_hwconfig.h"
#include "phydm_debug.h"
#include "phydm_regdefine11ac.h"
#include "phydm_regdefine11n.h"
#include "phydm_interface.h"
#include "phydm_reg.h"


#if (DM_ODM_SUPPORT_TYPE & ODM_CE)

VOID
PHY_SetTxPowerLimit(
	IN	PDM_ODM_T	pDM_Odm,
	IN	u8	*Regulation,
	IN	u8	*Band,
	IN	u8	*Bandwidth,
	IN	u8	*RateSection,
	IN	u8	*RfPath,
	IN	u8	*Channel,
	IN	u8	*PowerLimit
);

#endif

#if (RTL8188F_SUPPORT == 1) 
#include "rtl8188f/halhwimg8188f_mac.h"
#include "rtl8188f/halhwimg8188f_rf.h"
#include "rtl8188f/halhwimg8188f_bb.h"
#include "rtl8188f/hal8188freg.h"
#include "rtl8188f/phydm_rtl8188f.h"
#include "rtl8188f/phydm_regconfig8188f.h"
#include "rtl8188f/halphyrf_8188f.h" /* for IQK,LCK,Power-tracking */
#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
#include "rtl8188f_hal.h"
#endif
#endif

#endif	// __ODM_PRECOMP_H__

