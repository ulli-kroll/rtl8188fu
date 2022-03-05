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

//============================================================
// include files
//============================================================
#include "mp_precomp.h"
#include "phydm_precomp.h"

VOID 
odm_DynamicTxPowerInit(
	IN		PVOID					pDM_VOID	
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	
}

VOID
odm_DynamicTxPowerSavePowerIndex(
	IN		PVOID					pDM_VOID	
	)
{	
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
#if (DM_ODM_SUPPORT_TYPE & (ODM_CE))
	u1Byte		index;
	u4Byte		Power_Index_REG[6] = {0xc90, 0xc91, 0xc92, 0xc98, 0xc99, 0xc9a};
	
#endif
}

VOID
odm_DynamicTxPowerRestorePowerIndex(
	IN		PVOID					pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
#if (DM_ODM_SUPPORT_TYPE & (ODM_CE))
	u1Byte			index;
	PADAPTER		Adapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u4Byte			Power_Index_REG[6] = {0xc90, 0xc91, 0xc92, 0xc98, 0xc99, 0xc9a};
#endif
}

VOID
odm_DynamicTxPowerWritePowerIndex(
	IN		PVOID					pDM_VOID, 
	IN 	u1Byte		Value)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u1Byte			index;
	u4Byte			Power_Index_REG[6] = {0xc90, 0xc91, 0xc92, 0xc98, 0xc99, 0xc9a};
	
	for(index = 0; index< 6; index++)
		//PlatformEFIOWrite1Byte(Adapter, Power_Index_REG[index], Value);
		ODM_Write1Byte(pDM_Odm, Power_Index_REG[index], Value);

}


VOID 
odm_DynamicTxPower(
	IN		PVOID					pDM_VOID
	)
{
	// 
	// For AP/ADSL use prtl8192cd_priv
	// For CE/NIC use PADAPTER
	//
	//PADAPTER		pAdapter = pDM_Odm->Adapter;
//	prtl8192cd_priv	priv		= pDM_Odm->priv;
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	if (!(pDM_Odm->SupportAbility & ODM_BB_DYNAMIC_TXPWR))
		return;
	//
	// 2011/09/29 MH In HW integration first stage, we provide 4 different handle to operate
	// at the same time. In the stage2/3, we need to prive universal interface and merge all
	// HW dynamic mechanism.
	//
	switch	(pDM_Odm->SupportPlatform)
	{
		case	ODM_CE:
			odm_DynamicTxPowerNIC(pDM_Odm);
			break;	
		case	ODM_AP:
			odm_DynamicTxPowerAP(pDM_Odm);
			break;		

		case	ODM_ADSL:
			//odm_DIGAP(pDM_Odm);
			break;	
	}

	
}


VOID 
odm_DynamicTxPowerNIC(
	IN		PVOID					pDM_VOID
	)
{	
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	
	if (!(pDM_Odm->SupportAbility & ODM_BB_DYNAMIC_TXPWR))
		return;
	
#if (DM_ODM_SUPPORT_TYPE & (ODM_CE))

	if(pDM_Odm->SupportICType == ODM_RTL8192C)	
	{
		odm_DynamicTxPower_92C(pDM_Odm);
	}
	else if(pDM_Odm->SupportICType == ODM_RTL8192D)
	{
		odm_DynamicTxPower_92D(pDM_Odm);
	}
	else if (pDM_Odm->SupportICType == ODM_RTL8821)
	{
	}
#endif	
}

VOID 
odm_DynamicTxPowerAP(
	IN		PVOID					pDM_VOID

	)
{	
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
#if (DM_ODM_SUPPORT_TYPE == ODM_AP)

//#if ((RTL8192C_SUPPORT==1) || (RTL8192D_SUPPORT==1) || (RTL8188E_SUPPORT==1) || (RTL8812E_SUPPORT==1))


	prtl8192cd_priv	priv		= pDM_Odm->priv;
	s4Byte i;
	s2Byte pwr_thd = TX_POWER_NEAR_FIELD_THRESH_AP;

	if(!priv->pshare->rf_ft_var.tx_pwr_ctrl)
		return;
	
#if ((RTL8812E_SUPPORT==1) || (RTL8881A_SUPPORT==1) || (RTL8814A_SUPPORT==1))
	if (pDM_Odm->SupportICType & (ODM_RTL8812 | ODM_RTL8881A | ODM_RTL8814A))
		pwr_thd = TX_POWER_NEAR_FIELD_THRESH_8812;
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
	if(CHIP_VER_92X_SERIES(priv))
	{
#ifdef HIGH_POWER_EXT_PA
	if(pDM_Odm->ExtPA)
		tx_power_control(priv);
#endif		
	}
#endif	
	/*
	 *	Check if station is near by to use lower tx power
	 */

	if ((priv->up_time % 3) == 0 )  {
		int disable_pwr_ctrl = ((pDM_Odm->FalseAlmCnt.Cnt_all > 1000 ) || ((pDM_Odm->FalseAlmCnt.Cnt_all > 300 ) && ((RTL_R8(0xc50) & 0x7f) >= 0x32))) ? 1 : 0;
			
		for(i=0; i<ODM_ASSOCIATE_ENTRY_NUM; i++){
			PSTA_INFO_T pstat = pDM_Odm->pODM_StaInfo[i];
			if(IS_STA_VALID(pstat) ) {
					if(disable_pwr_ctrl)
						pstat->hp_level = 0;
					 else if ((pstat->hp_level == 0) && (pstat->rssi > pwr_thd))
					pstat->hp_level = 1;
						else if ((pstat->hp_level == 1) && (pstat->rssi < (pwr_thd-8)))
					pstat->hp_level = 0;
			}
		}

#if defined(CONFIG_WLAN_HAL_8192EE)
		if (GET_CHIP_VER(priv) == VERSION_8192E) {
			if( !disable_pwr_ctrl && (pDM_Odm->RSSI_Min != 0xff) ) {
				if(pDM_Odm->RSSI_Min > pwr_thd)
					RRSR_power_control_11n(priv,  1 );
				else if(pDM_Odm->RSSI_Min < (pwr_thd-8))
					RRSR_power_control_11n(priv,  0 );
			} else {
					RRSR_power_control_11n(priv,  0 );
			}
		}
#endif			

#ifdef CONFIG_WLAN_HAL_8814AE
		if (GET_CHIP_VER(priv) == VERSION_8814A) {
			if (!disable_pwr_ctrl && (pDM_Odm->RSSI_Min != 0xff)) {
				if (pDM_Odm->RSSI_Min > pwr_thd)
					RRSR_power_control_14(priv,  1);
				else if (pDM_Odm->RSSI_Min < (pwr_thd-8))
					RRSR_power_control_14(priv,  0);
			} else {
					RRSR_power_control_14(priv,  0);
			}
		}
#endif		

	}
//#endif	

#endif	
}


VOID 
odm_DynamicTxPower_92C(
	IN		PVOID					pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;

}


VOID 
odm_DynamicTxPower_92D(
	IN		PVOID					pDM_VOID
	)
{
#if (RTL8192D_SUPPORT==1)
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
#endif
}

VOID 
odm_DynamicTxPower_8821(
	IN		PVOID			pDM_VOID,	
	IN		pu1Byte			pDesc,
	IN		u1Byte			macId	
	)
{
#if (RTL8821A_SUPPORT == 1)
#endif	/*#if (RTL8821A_SUPPORT==1)*/
}

