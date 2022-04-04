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
ODM_ChangeDynamicInitGainThresh(
	IN	PVOID		pDM_VOID,
	IN	u4Byte		DM_Type,
	IN	u4Byte		DM_Value
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T			pDM_DigTable = &pDM_Odm->DM_DigTable;

	if (DM_Type == DIG_TYPE_THRESH_HIGH)
	{
		pDM_DigTable->RssiHighThresh = DM_Value;		
	}
	else if (DM_Type == DIG_TYPE_THRESH_LOW)
	{
		pDM_DigTable->RssiLowThresh = DM_Value;
	}
	else if (DM_Type == DIG_TYPE_ENABLE)
	{
		pDM_DigTable->Dig_Enable_Flag	= TRUE;
	}	
	else if (DM_Type == DIG_TYPE_DISABLE)
	{
		pDM_DigTable->Dig_Enable_Flag = FALSE;
	}	
	else if (DM_Type == DIG_TYPE_BACKOFF)
	{
		if(DM_Value > 30)
			DM_Value = 30;
		pDM_DigTable->BackoffVal = (u1Byte)DM_Value;
	}
	else if(DM_Type == DIG_TYPE_RX_GAIN_MIN)
	{
		if(DM_Value == 0)
			DM_Value = 0x1;
		pDM_DigTable->rx_gain_range_min = (u1Byte)DM_Value;
	}
	else if(DM_Type == DIG_TYPE_RX_GAIN_MAX)
	{
		if(DM_Value > 0x50)
			DM_Value = 0x50;
		pDM_DigTable->rx_gain_range_max = (u1Byte)DM_Value;
	}
}	// DM_ChangeDynamicInitGainThresh //

int 
getIGIForDiff(int value_IGI)
{
	#define ONERCCA_LOW_TH		0x30
	#define ONERCCA_LOW_DIFF		8

	if (value_IGI < ONERCCA_LOW_TH) {
		if ((ONERCCA_LOW_TH - value_IGI) < ONERCCA_LOW_DIFF)
			return ONERCCA_LOW_TH;
		else
			return value_IGI + ONERCCA_LOW_DIFF;
	} else {
		return value_IGI;
	}
}

VOID
odm_FAThresholdCheck(
	IN		PVOID			pDM_VOID,
	IN		BOOLEAN			bDFSBand,
	IN		BOOLEAN			bPerformance,
	IN		u4Byte			RxTp,
	IN		u4Byte			TxTp,
	OUT		u4Byte*			dm_FA_thres
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	
	if(pDM_Odm->bLinked && (bPerformance||bDFSBand))
	{
		{
			// For NIC
			dm_FA_thres[0] = DM_DIG_FA_TH0;
			dm_FA_thres[1] = DM_DIG_FA_TH1;
			dm_FA_thres[2] = DM_DIG_FA_TH2;
		}
	}
	else
	{
		{
			dm_FA_thres[0] = 2000;
			dm_FA_thres[1] = 4000;
			dm_FA_thres[2] = 5000;
		}
	}
	return;
}

u1Byte
odm_ForbiddenIGICheck(
	IN		PVOID			pDM_VOID,
	IN		u1Byte			DIG_Dynamic_MIN,
	IN		u1Byte			CurrentIGI
	)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;
	PFALSE_ALARM_STATISTICS 	pFalseAlmCnt = (PFALSE_ALARM_STATISTICS)PhyDM_Get_Structure( pDM_Odm, PHYDM_FALSEALMCNT);
	u1Byte						rx_gain_range_min = pDM_DigTable->rx_gain_range_min;

	if(pFalseAlmCnt->cnt_all > 10000)
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Abnormally false alarm case. \n"));

		if(pDM_DigTable->LargeFAHit != 3)
			pDM_DigTable->LargeFAHit++;
		
		if(pDM_DigTable->ForbiddenIGI < CurrentIGI)//if(pDM_DigTable->ForbiddenIGI < pDM_DigTable->CurIGValue)
		{
			pDM_DigTable->ForbiddenIGI = CurrentIGI;//pDM_DigTable->ForbiddenIGI = pDM_DigTable->CurIGValue;
			pDM_DigTable->LargeFAHit = 1;
		}

		if(pDM_DigTable->LargeFAHit >= 3)
		{
			if((pDM_DigTable->ForbiddenIGI + 2) > pDM_DigTable->rx_gain_range_max)
				rx_gain_range_min = pDM_DigTable->rx_gain_range_max;
			else
				rx_gain_range_min = (pDM_DigTable->ForbiddenIGI + 2);
			pDM_DigTable->Recover_cnt = 1800;
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Abnormally false alarm case: Recover_cnt = %d \n", pDM_DigTable->Recover_cnt));
		}
	}
	else
	{
		if(pDM_DigTable->Recover_cnt != 0)
		{
			pDM_DigTable->Recover_cnt --;
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Normal Case: Recover_cnt = %d \n", pDM_DigTable->Recover_cnt));
		}
		else
		{
			if(pDM_DigTable->LargeFAHit < 3)
			{
				if((pDM_DigTable->ForbiddenIGI - 2) < DIG_Dynamic_MIN) //DM_DIG_MIN)
				{
					pDM_DigTable->ForbiddenIGI = DIG_Dynamic_MIN; //DM_DIG_MIN;
					rx_gain_range_min = DIG_Dynamic_MIN; //DM_DIG_MIN;
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Normal Case: At Lower Bound\n"));
				}
				else
				{
					pDM_DigTable->ForbiddenIGI -= 2;
					rx_gain_range_min = (pDM_DigTable->ForbiddenIGI + 2);
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Normal Case: Approach Lower Bound\n"));
				}
			}
			else
			{
				pDM_DigTable->LargeFAHit = 0;
			}
		}
	}
	
	return rx_gain_range_min;

}

VOID
odm_InbandNoiseCalculate (	
	IN		PVOID		pDM_VOID
	)
{
	return;
}

VOID
odm_DigForBtHsMode(
	IN		PVOID		pDM_VOID
	)
{
}

VOID
rtl8188fu_dm_write_dig(
	IN	PVOID			pDM_VOID,
	IN	u1Byte			CurrentIGI
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T			pDM_DigTable = &pDM_Odm->DM_DigTable;

	if (pDM_DigTable->bStopDIG) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("ODM_Write_DIG(): Stop Writing IGI\n"));
		return;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_TRACE, ("ODM_Write_DIG(): ODM_REG(IGI_A,pDM_Odm)=0x%x, ODM_BIT(IGI,pDM_Odm)=0x%x\n",
		ODM_REG(IGI_A,pDM_Odm),ODM_BIT(IGI,pDM_Odm)));

	//1 Check initial gain by upper bound		
	if ((!pDM_DigTable->bPSDInProgress) && pDM_Odm->bLinked)
	{
		if (CurrentIGI > pDM_DigTable->rx_gain_range_max) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_TRACE, ("ODM_Write_DIG(): CurrentIGI(0x%02x) is larger than upper bound !!\n", CurrentIGI));
			CurrentIGI = pDM_DigTable->rx_gain_range_max;
		}
		if (pDM_Odm->SupportAbility & ODM_BB_ADAPTIVITY && pDM_Odm->adaptivity_flag == TRUE)
		{
			if(CurrentIGI > pDM_Odm->Adaptivity_IGI_upper)
				CurrentIGI = pDM_Odm->Adaptivity_IGI_upper;
	
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("ODM_Write_DIG(): Adaptivity case: Force upper bound to 0x%x !!!!!!\n", CurrentIGI));
		}
	}

	if(pDM_DigTable->CurIGValue != CurrentIGI)
	{

		/*Add by YuChen for USB IO too slow issue*/
		if ((pDM_Odm->SupportAbility & ODM_BB_ADAPTIVITY) && (CurrentIGI > pDM_DigTable->CurIGValue))
			Phydm_Adaptivity(pDM_Odm, CurrentIGI);

		//1 Set IGI value
		if(pDM_Odm->SupportPlatform & (ODM_CE))
		{ 
			ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_A,pDM_Odm), ODM_BIT(IGI,pDM_Odm), CurrentIGI);

			if(pDM_Odm->RFType > ODM_1T1R)
				ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_B,pDM_Odm), ODM_BIT(IGI,pDM_Odm), CurrentIGI);

		}
		pDM_DigTable->CurIGValue = CurrentIGI;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_TRACE, ("ODM_Write_DIG(): CurrentIGI(0x%02x).\n", CurrentIGI));
	
}

VOID
odm_PauseDIG(
	IN		PVOID					pDM_VOID,
	IN		PHYDM_PAUSE_TYPE		PauseType,
	IN		PHYDM_PAUSE_LEVEL		pause_level,
	IN		u1Byte					IGIValue
)
{
	PDM_ODM_T			pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T				pDM_DigTable = &pDM_Odm->DM_DigTable;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG()=========> level = %d\n", pause_level));

	if ((pDM_DigTable->pause_dig_level == 0) && (!(pDM_Odm->SupportAbility & ODM_BB_DIG) || !(pDM_Odm->SupportAbility & ODM_BB_FA_CNT))) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, 
			("odm_PauseDIG(): Return: SupportAbility DIG or FA is disabled !!\n"));
		return;
	}

	if (pause_level > DM_DIG_MAX_PAUSE_TYPE) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, 
			("odm_PauseDIG(): Return: Wrong pause level !!\n"));
		return;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): pause level = 0x%x, Current value = 0x%x\n", pDM_DigTable->pause_dig_level, IGIValue));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): pause value = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", 
		pDM_DigTable->pause_dig_value[7], pDM_DigTable->pause_dig_value[6], pDM_DigTable->pause_dig_value[5], pDM_DigTable->pause_dig_value[4],
		pDM_DigTable->pause_dig_value[3], pDM_DigTable->pause_dig_value[2], pDM_DigTable->pause_dig_value[1], pDM_DigTable->pause_dig_value[0]));
	
	switch (PauseType) {
	/* Pause DIG */
	case PHYDM_PAUSE:
	{
		/* Disable DIG */
		ODM_CmnInfoUpdate(pDM_Odm, ODM_CMNINFO_ABILITY, pDM_Odm->SupportAbility & (~ODM_BB_DIG));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): Pause DIG !!\n"));
		
		/* Backup IGI value */
		if (pDM_DigTable->pause_dig_level == 0) {
			pDM_DigTable->IGIBackup = pDM_DigTable->CurIGValue;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): Backup IGI  = 0x%x, new IGI = 0x%x\n", pDM_DigTable->IGIBackup, IGIValue));
		}

		/* Record IGI value */
		pDM_DigTable->pause_dig_value[pause_level] = IGIValue;

		/* Update pause level */
		pDM_DigTable->pause_dig_level = (pDM_DigTable->pause_dig_level | BIT(pause_level));

		/* Write new IGI value */
		if (BIT(pause_level + 1) > pDM_DigTable->pause_dig_level) {
			rtl8188fu_dm_write_dig(pDM_Odm, IGIValue);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): IGI of higher level = 0x%x\n",  IGIValue));
		}
		break;
	}
	/* Resume DIG */
	case PHYDM_RESUME:
	{
		/* check if the level is illegal or not */
		if ((pDM_DigTable->pause_dig_level & (BIT(pause_level))) != 0) {
			pDM_DigTable->pause_dig_level = pDM_DigTable->pause_dig_level & (~(BIT(pause_level)));
			pDM_DigTable->pause_dig_value[pause_level] = 0;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): Resume DIG !!\n"));
		} else {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): Wrong resume level !!\n"));
			break;
		}

		/* Resume DIG */
		if (pDM_DigTable->pause_dig_level == 0) {
			/* Write backup IGI value */
			rtl8188fu_dm_write_dig(pDM_Odm, pDM_DigTable->IGIBackup);
			pDM_DigTable->bIgnoreDIG = TRUE;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): Write original IGI = 0x%x\n", pDM_DigTable->IGIBackup));

			/* Enable DIG */
			ODM_CmnInfoUpdate(pDM_Odm, ODM_CMNINFO_ABILITY, pDM_Odm->SupportAbility | ODM_BB_DIG);	
			break;
		}

		if (BIT(pause_level) > pDM_DigTable->pause_dig_level) {
			u1Byte		max_level;
		
			/* Calculate the maximum level now */
			for (max_level = (pause_level - 1); max_level >= 0; max_level--) {
				if ((pDM_DigTable->pause_dig_level & BIT(max_level)) > 0)
					break;
			}
		
			/* write IGI of lower level */
			rtl8188fu_dm_write_dig(pDM_Odm, pDM_DigTable->pause_dig_value[max_level]);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): Write IGI (0x%x) of level (%d)\n",  
				 pDM_DigTable->pause_dig_value[max_level], max_level));
			break;
		}
		break;
	}
	default:
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): Wrong  type !!\n"));
		break;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): pause level = 0x%x, Current value = 0x%x\n", pDM_DigTable->pause_dig_level, IGIValue));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): pause value = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", 
		pDM_DigTable->pause_dig_value[7], pDM_DigTable->pause_dig_value[6], pDM_DigTable->pause_dig_value[5], pDM_DigTable->pause_dig_value[4],
		pDM_DigTable->pause_dig_value[3], pDM_DigTable->pause_dig_value[2], pDM_DigTable->pause_dig_value[1], pDM_DigTable->pause_dig_value[0]));

}

BOOLEAN 
odm_DigAbort(
	IN		PVOID			pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T			pDM_DigTable = &pDM_Odm->DM_DigTable;


	//SupportAbility
	if(!(pDM_Odm->SupportAbility & ODM_BB_FA_CNT))
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Return: SupportAbility ODM_BB_FA_CNT is disabled\n"));
		return	TRUE;
	}

	//SupportAbility
	if(!(pDM_Odm->SupportAbility & ODM_BB_DIG))
	{	
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Return: SupportAbility ODM_BB_DIG is disabled\n"));
		return	TRUE;
	}

	//ScanInProcess
	if(*(pDM_Odm->pbScanInProcess))
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Return: In Scan Progress \n"));
	    	return	TRUE;
	}

	if(pDM_DigTable->bIgnoreDIG)
	{
		pDM_DigTable->bIgnoreDIG = FALSE;
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Return: Ignore DIG \n"));
	    	return	TRUE;
	}

	//add by Neil Chen to avoid PSD is processing
	if(pDM_Odm->bDMInitialGainEnable == FALSE)
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Return: PSD is Processing \n"));
		return	TRUE;
	}

#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	#ifdef CONFIG_SPECIAL_SETTING_FOR_FUNAI_TV	
	if((pDM_Odm->bLinked) && (pDM_Odm->Adapter->registrypriv.force_igi !=0))
	{	
		printk("pDM_Odm->RSSI_Min=%d \n",pDM_Odm->RSSI_Min);
		ODM_Write_DIG(pDM_Odm,pDM_Odm->Adapter->registrypriv.force_igi);
		return	TRUE;
	}
	#endif
#else
	if (!(priv->up_time > 5))
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Return: Not In DIG Operation Period \n"));
		return	TRUE;
	}
#endif

	return	FALSE;
}

VOID
odm_DIGInit(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;

	pDM_DigTable->bStopDIG = FALSE;
	pDM_DigTable->bIgnoreDIG = FALSE;
	pDM_DigTable->bPSDInProgress = FALSE;
	pDM_DigTable->CurIGValue = (u1Byte) ODM_GetBBReg(pDM_Odm, ODM_REG(IGI_A,pDM_Odm), ODM_BIT(IGI,pDM_Odm));
	pDM_DigTable->RssiLowThresh 	= DM_DIG_THRESH_LOW;
	pDM_DigTable->RssiHighThresh 	= DM_DIG_THRESH_HIGH;
	pDM_DigTable->FALowThresh	= DM_FALSEALARM_THRESH_LOW;
	pDM_DigTable->FAHighThresh	= DM_FALSEALARM_THRESH_HIGH;
	pDM_DigTable->BackoffVal = DM_DIG_BACKOFF_DEFAULT;
	pDM_DigTable->BackoffVal_range_max = DM_DIG_BACKOFF_MAX;
	pDM_DigTable->BackoffVal_range_min = DM_DIG_BACKOFF_MIN;
	pDM_DigTable->PreCCK_CCAThres = 0xFF;
	pDM_DigTable->CurCCK_CCAThres = 0x83;
	pDM_DigTable->ForbiddenIGI = DM_DIG_MIN_NIC;
	pDM_DigTable->LargeFAHit = 0;
	pDM_DigTable->Recover_cnt = 0;
	pDM_DigTable->bMediaConnect_0 = FALSE;
	pDM_DigTable->bMediaConnect_1 = FALSE;

	//To Initialize pDM_Odm->bDMInitialGainEnable == FALSE to avoid DIG error
	pDM_Odm->bDMInitialGainEnable = TRUE;

	pDM_DigTable->DIG_Dynamic_MIN_0 = DM_DIG_MIN_NIC;
	pDM_DigTable->DIG_Dynamic_MIN_1 = DM_DIG_MIN_NIC;

	//To Initi BT30 IGI
	pDM_DigTable->BT30_CurIGI=0x32;

	ODM_Memory_Set(pDM_Odm, pDM_DigTable->pause_dig_value, 0, (DM_DIG_MAX_PAUSE_TYPE + 1));
	pDM_DigTable->pause_dig_level = 0;
	ODM_Memory_Set(pDM_Odm, pDM_DigTable->pause_cckpd_value, 0, (DM_DIG_MAX_PAUSE_TYPE + 1));
	pDM_DigTable->pause_cckpd_level = 0;

	if(pDM_Odm->board_type & (ODM_BOARD_EXT_PA|ODM_BOARD_EXT_LNA))
	{
		pDM_DigTable->rx_gain_range_max = DM_DIG_MAX_NIC;
		pDM_DigTable->rx_gain_range_min = DM_DIG_MIN_NIC;
	}
	else
	{
		pDM_DigTable->rx_gain_range_max = DM_DIG_MAX_NIC;
		pDM_DigTable->rx_gain_range_min = DM_DIG_MIN_NIC;
	}
	
}


VOID 
odm_DIG(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;

	// Common parameters
	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;
	PFALSE_ALARM_STATISTICS		pFalseAlmCnt = (PFALSE_ALARM_STATISTICS)PhyDM_Get_Structure( pDM_Odm, PHYDM_FALSEALMCNT);
	BOOLEAN						FirstConnect,FirstDisConnect;
	u1Byte						DIG_MaxOfMin, DIG_Dynamic_MIN;
	u1Byte						dm_dig_max, dm_dig_min;
	u1Byte						CurrentIGI = pDM_DigTable->CurIGValue;
	u1Byte						offset;
	u4Byte						dm_FA_thres[3];
	u4Byte						TxTp = 0, RxTp = 0;
	BOOLEAN						bDFSBand = FALSE;
	BOOLEAN						bPerformance = TRUE, bFirstTpTarget = FALSE, bFirstCoverage = FALSE;

	if(odm_DigAbort(pDM_Odm) == TRUE)
		return;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG()===========================>\n\n"));
	

	//1 Update status
	{	
		DIG_Dynamic_MIN = pDM_DigTable->DIG_Dynamic_MIN_0;
		FirstConnect = (pDM_Odm->bLinked) && (pDM_DigTable->bMediaConnect_0 == FALSE);
		FirstDisConnect = (!pDM_Odm->bLinked) && (pDM_DigTable->bMediaConnect_0 == TRUE);
	}


	//1 Boundary Decision
	{
		//2 For WIN\CE
		dm_dig_max = 0x5A;
		dm_dig_min = DM_DIG_MIN_NIC;

		DIG_MaxOfMin = DM_DIG_MAX_AP;
	}
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Absolutly upper bound = 0x%x, lower bound = 0x%x\n",dm_dig_max, dm_dig_min));

	//1 Adjust boundary by RSSI
	if(pDM_Odm->bLinked && bPerformance)
	{
		//2 Modify DIG upper bound
		//4 Modify DIG upper bound for 92E, 8723A\B, 8821 & 8812 BT
		offset = 15;

		if((pDM_Odm->RSSI_Min + offset) > dm_dig_max )
			pDM_DigTable->rx_gain_range_max = dm_dig_max;
		else if((pDM_Odm->RSSI_Min + offset) < dm_dig_min )
			pDM_DigTable->rx_gain_range_max = dm_dig_min;
		else
			pDM_DigTable->rx_gain_range_max = pDM_Odm->RSSI_Min + offset;

#if (DM_ODM_SUPPORT_TYPE & (ODM_CE))
		//2 Modify DIG lower bound
		//if(pDM_Odm->bOneEntryOnly)
		{
			if(pDM_Odm->RSSI_Min < dm_dig_min)
				DIG_Dynamic_MIN = dm_dig_min;
			else if (pDM_Odm->RSSI_Min > DIG_MaxOfMin)
				DIG_Dynamic_MIN = DIG_MaxOfMin;
			else
				DIG_Dynamic_MIN = pDM_Odm->RSSI_Min;
		}
#else
		{
			//4 For AP
#ifdef __ECOS
			HAL_REORDER_BARRIER();
#else
			rmb();
#endif
			if (bDFSBand)
			{
				DIG_Dynamic_MIN = dm_dig_min;
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): DFS band: Force lower bound to 0x%x after link !!!!!!\n", dm_dig_min));
			}
			else 
			{
				if(pDM_Odm->RSSI_Min < dm_dig_min)
					DIG_Dynamic_MIN = dm_dig_min;
				else if (pDM_Odm->RSSI_Min > DIG_MaxOfMin)
					DIG_Dynamic_MIN = DIG_MaxOfMin;
				else
					DIG_Dynamic_MIN = pDM_Odm->RSSI_Min;
			}
		}
#endif
	}
	else
	{
		{
			pDM_DigTable->rx_gain_range_max = DM_DIG_MAX_OF_MIN;
		}
		DIG_Dynamic_MIN = dm_dig_min;
	}
	
	//1 Force Lower Bound for AntDiv
	if(pDM_Odm->bLinked && !pDM_Odm->bOneEntryOnly)
	{
		if((pDM_Odm->SupportICType & ODM_ANTDIV_SUPPORT) && (pDM_Odm->SupportAbility & ODM_BB_ANT_DIV))
		{
			if (pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV || pDM_Odm->AntDivType == CG_TRX_SMART_ANTDIV) {
				if (pDM_DigTable->AntDiv_RSSI_max > DIG_MaxOfMin)
					DIG_Dynamic_MIN = DIG_MaxOfMin;
				else
					DIG_Dynamic_MIN = (u1Byte) pDM_DigTable->AntDiv_RSSI_max;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Antenna diversity case: Force lower bound to 0x%x !!!!!!\n", DIG_Dynamic_MIN));
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Antenna diversity case: RSSI_max = 0x%x !!!!!!\n", pDM_DigTable->AntDiv_RSSI_max));
			}
		}
	}
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Adjust boundary by RSSI Upper bound = 0x%x, Lower bound = 0x%x\n",
		pDM_DigTable->rx_gain_range_max, DIG_Dynamic_MIN));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Link status: bLinked = %d, RSSI = %d, bFirstConnect = %d, bFirsrDisConnect = %d\n\n",
		pDM_Odm->bLinked, pDM_Odm->RSSI_Min, FirstConnect, FirstDisConnect));

	//1 Modify DIG lower bound, deal with abnormal case
	//2 Abnormal false alarm case
	{
		if(!pDM_Odm->bLinked)
		{
			pDM_DigTable->rx_gain_range_min = DIG_Dynamic_MIN;

			if (FirstDisConnect)
				pDM_DigTable->ForbiddenIGI = DIG_Dynamic_MIN;
		}
		else
			pDM_DigTable->rx_gain_range_min = odm_ForbiddenIGICheck(pDM_Odm, DIG_Dynamic_MIN, CurrentIGI);
	}

	//2 Abnormal # beacon case
#if (DM_ODM_SUPPORT_TYPE & (ODM_CE))
	if(pDM_Odm->bLinked && !FirstConnect)
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("Beacon Num (%d)\n", pDM_Odm->PhyDbgInfo.NumQryBeaconPkt));
		if((pDM_Odm->PhyDbgInfo.NumQryBeaconPkt < 5) && (pDM_Odm->bsta_state))
		{
			pDM_DigTable->rx_gain_range_min = dm_dig_min;
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Abnrormal #beacon (%d) case in STA mode: Force lower bound to 0x%x !!!!!!\n\n",
				pDM_Odm->PhyDbgInfo.NumQryBeaconPkt, pDM_DigTable->rx_gain_range_min));
		}
	}
#endif

	//2 Abnormal lower bound case
	if(pDM_DigTable->rx_gain_range_min > pDM_DigTable->rx_gain_range_max)
	{
		pDM_DigTable->rx_gain_range_min = pDM_DigTable->rx_gain_range_max;
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Abnrormal lower bound case: Force lower bound to 0x%x !!!!!!\n\n",pDM_DigTable->rx_gain_range_min));
	}

	
	//1 False alarm threshold decision
	odm_FAThresholdCheck(pDM_Odm, bDFSBand, bPerformance, RxTp, TxTp, dm_FA_thres);
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): False alarm threshold = %d, %d, %d \n\n", dm_FA_thres[0], dm_FA_thres[1], dm_FA_thres[2]));

	//1 Adjust initial gain by false alarm
	if(pDM_Odm->bLinked && bPerformance)
	{
		//2 After link
		ODM_RT_TRACE(pDM_Odm,	ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Adjust IGI after link\n"));

		if(bFirstTpTarget || (FirstConnect && bPerformance))
		{	
			pDM_DigTable->LargeFAHit = 0;
			
			{
				if(pDM_Odm->RSSI_Min < DIG_MaxOfMin)
				{
					if(CurrentIGI < pDM_Odm->RSSI_Min)
						CurrentIGI = pDM_Odm->RSSI_Min;
				}
				else
				{
					if(CurrentIGI < DIG_MaxOfMin)
						CurrentIGI = DIG_MaxOfMin;
				}

#if (DM_ODM_SUPPORT_TYPE & (ODM_CE))
#endif
			}

			ODM_RT_TRACE(pDM_Odm,	ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): First connect case: IGI does on-shot to 0x%x\n", CurrentIGI));

		}
		else
		{
			if(pFalseAlmCnt->cnt_all > dm_FA_thres[2])
				CurrentIGI = CurrentIGI + 4;
			else if (pFalseAlmCnt->cnt_all > dm_FA_thres[1])
				CurrentIGI = CurrentIGI + 2;
			else if(pFalseAlmCnt->cnt_all < dm_FA_thres[0])
				CurrentIGI = CurrentIGI - 2;

			//4 Abnormal # beacon case
#if (DM_ODM_SUPPORT_TYPE & (ODM_CE))
			if((pDM_Odm->PhyDbgInfo.NumQryBeaconPkt < 5) && (pFalseAlmCnt->cnt_all < DM_DIG_FA_TH1) && (pDM_Odm->bsta_state))
			{						
				CurrentIGI = pDM_DigTable->rx_gain_range_min;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Abnormal #beacon (%d) case: IGI does one-shot to 0x%x\n", 
					pDM_Odm->PhyDbgInfo.NumQryBeaconPkt, CurrentIGI));
			}
#endif
		}
	}	
	else
	{
		//2 Before link
		ODM_RT_TRACE(pDM_Odm,	ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Adjust IGI before link\n"));
		
		if(FirstDisConnect || bFirstCoverage)
		{
			CurrentIGI = dm_dig_min;
			ODM_RT_TRACE(pDM_Odm,	ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): First disconnect case: IGI does on-shot to lower bound\n"));
		}
		else
		{
			if(pFalseAlmCnt->cnt_all > dm_FA_thres[2])
				CurrentIGI = CurrentIGI + 4;
			else if (pFalseAlmCnt->cnt_all > dm_FA_thres[1])
				CurrentIGI = CurrentIGI + 2;
			else if(pFalseAlmCnt->cnt_all < dm_FA_thres[0])
				CurrentIGI = CurrentIGI - 2;
		}
	}

	//1 Check initial gain by upper/lower bound
	if(CurrentIGI < pDM_DigTable->rx_gain_range_min)
		CurrentIGI = pDM_DigTable->rx_gain_range_min;
	
	if(CurrentIGI > pDM_DigTable->rx_gain_range_max)
		CurrentIGI = pDM_DigTable->rx_gain_range_max;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): CurIGValue=0x%x, TotalFA = %d\n\n", CurrentIGI, pFalseAlmCnt->cnt_all));	

	//1 High power RSSI threshold

	//1 Update status
	{
		{
			rtl8188fu_dm_write_dig(pDM_Odm, CurrentIGI);//ODM_Write_DIG(pDM_Odm, pDM_DigTable->CurIGValue);
			pDM_DigTable->bMediaConnect_0 = pDM_Odm->bLinked;
			pDM_DigTable->DIG_Dynamic_MIN_0 = DIG_Dynamic_MIN;
		}
	}
}

VOID
odm_DIGbyRSSI_LPS(
	IN		PVOID		pDM_VOID
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_CE))
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PFALSE_ALARM_STATISTICS		pFalseAlmCnt = (PFALSE_ALARM_STATISTICS)PhyDM_Get_Structure( pDM_Odm, PHYDM_FALSEALMCNT);

	u1Byte	RSSI_Lower=DM_DIG_MIN_NIC;   //0x1E or 0x1C
	u1Byte	CurrentIGI=pDM_Odm->RSSI_Min;

	if(odm_DigAbort(pDM_Odm) == TRUE)
		return;

	CurrentIGI=CurrentIGI+RSSI_OFFSET_DIG;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIGbyRSSI_LPS()==>\n"));

	// Using FW PS mode to make IGI
	//Adjust by  FA in LPS MODE
	if(pFalseAlmCnt->cnt_all> DM_DIG_FA_TH2_LPS)
		CurrentIGI = CurrentIGI+4;
	else if (pFalseAlmCnt->cnt_all > DM_DIG_FA_TH1_LPS)
		CurrentIGI = CurrentIGI+2;
	else if(pFalseAlmCnt->cnt_all < DM_DIG_FA_TH0_LPS)
		CurrentIGI = CurrentIGI-2;	


	//Lower bound checking

	//RSSI Lower bound check
	if((pDM_Odm->RSSI_Min-10) > DM_DIG_MIN_NIC)
		RSSI_Lower =(pDM_Odm->RSSI_Min-10);
	else
		RSSI_Lower =DM_DIG_MIN_NIC;

	//Upper and Lower Bound checking
	 if(CurrentIGI > DM_DIG_MAX_NIC)
	 	CurrentIGI=DM_DIG_MAX_NIC;
	 else if(CurrentIGI < RSSI_Lower)
		CurrentIGI =RSSI_Lower;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIGbyRSSI_LPS(): pFalseAlmCnt->cnt_all = %d\n",pFalseAlmCnt->cnt_all));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIGbyRSSI_LPS(): pDM_Odm->RSSI_Min = %d\n",pDM_Odm->RSSI_Min));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIGbyRSSI_LPS(): CurrentIGI = 0x%x\n",CurrentIGI));

	rtl8188fu_dm_write_dig(pDM_Odm, CurrentIGI);//ODM_Write_DIG(pDM_Odm, pDM_DigTable->CurIGValue);
#endif
}

//3============================================================
//3 FASLE ALARM CHECK
//3============================================================

VOID 
rtl9188fu_dm_false_alarm_counter_statistics(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PFALSE_ALARM_STATISTICS 	FalseAlmCnt = (PFALSE_ALARM_STATISTICS)PhyDM_Get_Structure( pDM_Odm, PHYDM_FALSEALMCNT);
	u4Byte 						ret_value;


	if(!(pDM_Odm->SupportAbility & ODM_BB_FA_CNT))
		return;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics()======>\n"));

#if (ODM_IC_11N_SERIES_SUPPORT == 1) 
	if(pDM_Odm->SupportICType & ODM_IC_11N_SERIES)
	{

		//hold ofdm counter
		ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_HOLDC_11N, BIT31, 1); //hold page C counter
		ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RSTD_11N, BIT31, 1); //hold page D counter
	
		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_FA_TYPE1_11N, bMaskDWord);
		FalseAlmCnt->cnt_fast_fsync_fail = (ret_value&0xffff);
		FalseAlmCnt->cnt_sb_search_fail = ((ret_value&0xffff0000)>>16);		

		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_FA_TYPE2_11N, bMaskDWord);
		FalseAlmCnt->cnt_ofdm_cca = (ret_value&0xffff); 
		FalseAlmCnt->cnt_parity_fail = ((ret_value&0xffff0000)>>16);	

		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_FA_TYPE3_11N, bMaskDWord);
		FalseAlmCnt->cnt_rate_illegal = (ret_value&0xffff);
		FalseAlmCnt->cnt_crc8_fail = ((ret_value&0xffff0000)>>16);

		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_FA_TYPE4_11N, bMaskDWord);
		FalseAlmCnt->cnt_mcs_fail = (ret_value&0xffff);

		FalseAlmCnt->cnt_ofdm_fail = 	FalseAlmCnt->cnt_parity_fail + FalseAlmCnt->cnt_rate_illegal +
								FalseAlmCnt->cnt_crc8_fail + FalseAlmCnt->cnt_mcs_fail +
								FalseAlmCnt->cnt_fast_fsync_fail + FalseAlmCnt->cnt_sb_search_fail;

		{
			//hold cck counter
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT12, 1); 
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT14, 1); 
		
			ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_CCK_FA_LSB_11N, bMaskByte0);
			FalseAlmCnt->cnt_cck_fail = ret_value;

			ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_CCK_FA_MSB_11N, bMaskByte3);
			FalseAlmCnt->cnt_cck_fail +=  (ret_value& 0xff)<<8;

			ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_CCK_CCA_CNT_11N, bMaskDWord);
			FalseAlmCnt->cnt_cck_cca = ((ret_value&0xFF)<<8) |((ret_value&0xFF00)>>8);
		}
	
		FalseAlmCnt->cnt_all = (	FalseAlmCnt->cnt_fast_fsync_fail + 
							FalseAlmCnt->cnt_sb_search_fail +
							FalseAlmCnt->cnt_parity_fail +
							FalseAlmCnt->cnt_rate_illegal +
							FalseAlmCnt->cnt_crc8_fail +
							FalseAlmCnt->cnt_mcs_fail +
							FalseAlmCnt->cnt_cck_fail);	

		FalseAlmCnt->cnt_cca_all = FalseAlmCnt->cnt_ofdm_cca + FalseAlmCnt->cnt_cck_cca;

		{
			//reset false alarm counter registers
			ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RSTC_11N, BIT31, 1);
			ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RSTC_11N, BIT31, 0);
			ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RSTD_11N, BIT27, 1);
			ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RSTD_11N, BIT27, 0);

			//update ofdm counter
			ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_HOLDC_11N, BIT31, 0); //update page C counter
			ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RSTD_11N, BIT31, 0); //update page D counter

			//reset CCK CCA counter
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT13|BIT12, 0); 
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT13|BIT12, 2); 
			//reset CCK FA counter
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT15|BIT14, 0); 
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT15|BIT14, 2); 
		}
		

		ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Cnt_Fast_Fsync=%d, Cnt_SB_Search_fail=%d\n",
			FalseAlmCnt->cnt_fast_fsync_fail, FalseAlmCnt->cnt_sb_search_fail));
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Cnt_Parity_Fail=%d, Cnt_Rate_Illegal=%d\n",
			FalseAlmCnt->cnt_parity_fail, FalseAlmCnt->cnt_rate_illegal));
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Cnt_Crc8_fail=%d, Cnt_Mcs_fail=%d\n",
		FalseAlmCnt->cnt_crc8_fail, FalseAlmCnt->cnt_mcs_fail));
	}
#endif

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Cnt_OFDM_CCA=%d\n", FalseAlmCnt->cnt_ofdm_cca));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Cnt_CCK_CCA=%d\n", FalseAlmCnt->cnt_cck_cca));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Cnt_CCA_all=%d\n", FalseAlmCnt->cnt_cca_all));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Cnt_Ofdm_fail=%d\n", FalseAlmCnt->cnt_ofdm_fail));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Cnt_Cck_fail=%d\n", FalseAlmCnt->cnt_cck_fail));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Cnt_Ofdm_fail=%d\n", FalseAlmCnt->cnt_ofdm_fail));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Total False Alarm=%d\n\n", FalseAlmCnt->cnt_all));
}

//3============================================================
//3 CCK Packet Detect Threshold
//3============================================================

VOID
odm_PauseCCKPacketDetection(
	IN		PVOID					pDM_VOID,
	IN		PHYDM_PAUSE_TYPE		PauseType,
	IN		PHYDM_PAUSE_LEVEL		pause_level,
	IN		u1Byte					CCKPDThreshold
)
{
	PDM_ODM_T			pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T				pDM_DigTable = &pDM_Odm->DM_DigTable;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CCK_PD, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection()=========> level = %d\n", pause_level));

	if ((pDM_DigTable->pause_cckpd_level == 0) && (!(pDM_Odm->SupportAbility & ODM_BB_CCK_PD) || !(pDM_Odm->SupportAbility & ODM_BB_FA_CNT))) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CCK_PD, ODM_DBG_LOUD, ("Return: SupportAbility ODM_BB_CCK_PD or ODM_BB_FA_CNT is disabled\n"));
		return;
	}

	if (pause_level > DM_DIG_MAX_PAUSE_TYPE) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CCK_PD, ODM_DBG_LOUD, 
			("odm_PauseCCKPacketDetection(): Return: Wrong pause level !!\n"));
		return;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CCK_PD, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): pause level = 0x%x, Current value = 0x%x\n", pDM_DigTable->pause_cckpd_level, CCKPDThreshold));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CCK_PD, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): pause value = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", 
		pDM_DigTable->pause_cckpd_value[7], pDM_DigTable->pause_cckpd_value[6], pDM_DigTable->pause_cckpd_value[5], pDM_DigTable->pause_cckpd_value[4],
		pDM_DigTable->pause_cckpd_value[3], pDM_DigTable->pause_cckpd_value[2], pDM_DigTable->pause_cckpd_value[1], pDM_DigTable->pause_cckpd_value[0]));

	switch (PauseType) {
	/* Pause CCK Packet Detection Threshold */
	case PHYDM_PAUSE:
	{
		/* Disable CCK PD */
		ODM_CmnInfoUpdate(pDM_Odm, ODM_CMNINFO_ABILITY, pDM_Odm->SupportAbility & (~ODM_BB_CCK_PD));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CCK_PD, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): Pause CCK packet detection threshold !!\n"));

		/* Backup original CCK PD threshold decided by CCK PD mechanism */
		if (pDM_DigTable->pause_cckpd_level == 0) {
			pDM_DigTable->CCKPDBackup = pDM_DigTable->CurCCK_CCAThres;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CCK_PD, ODM_DBG_LOUD, 
				("odm_PauseCCKPacketDetection(): Backup CCKPD  = 0x%x, new CCKPD = 0x%x\n", pDM_DigTable->CCKPDBackup, CCKPDThreshold));
		}

		/* Update pause level */
		pDM_DigTable->pause_cckpd_level = (pDM_DigTable->pause_cckpd_level | BIT(pause_level));

		/* Record CCK PD threshold */
		pDM_DigTable->pause_cckpd_value[pause_level] = CCKPDThreshold;
		
		/* Write new CCK PD threshold */
		if (BIT(pause_level + 1) > pDM_DigTable->pause_cckpd_level) {
			ODM_Write_CCK_CCA_Thres(pDM_Odm, CCKPDThreshold);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CCK_PD, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): CCKPD of higher level = 0x%x\n", CCKPDThreshold));
		}
		break;
	}
	/* Resume CCK Packet Detection Threshold */
	case PHYDM_RESUME:
	{	
		/* check if the level is illegal or not */
		if ((pDM_DigTable->pause_cckpd_level & (BIT(pause_level))) != 0) {
			pDM_DigTable->pause_cckpd_level = pDM_DigTable->pause_cckpd_level & (~(BIT(pause_level)));
			pDM_DigTable->pause_cckpd_value[pause_level] = 0;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CCK_PD, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): Resume CCK PD !!\n"));
		} else {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CCK_PD, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): Wrong resume level !!\n"));
			break;
		}

		/* Resume DIG */
		if (pDM_DigTable->pause_cckpd_level == 0) {
			/* Write backup IGI value */
			ODM_Write_CCK_CCA_Thres(pDM_Odm, pDM_DigTable->CCKPDBackup);
			/* pDM_DigTable->bIgnoreDIG = TRUE; */
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CCK_PD, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): Write original CCKPD = 0x%x\n", pDM_DigTable->CCKPDBackup));

			/* Enable DIG */
			ODM_CmnInfoUpdate(pDM_Odm, ODM_CMNINFO_ABILITY, pDM_Odm->SupportAbility | ODM_BB_CCK_PD);	
			break;
		}

		if (BIT(pause_level) > pDM_DigTable->pause_cckpd_level) {
			u1Byte	max_level;
		
			/* Calculate the maximum level now */
			for (max_level = (pause_level - 1); max_level >= 0; max_level--) {
				if ((pDM_DigTable->pause_cckpd_level & BIT(max_level)) > 0)
					break;
			}
		
			/* write CCKPD of lower level */
			ODM_Write_CCK_CCA_Thres(pDM_Odm, pDM_DigTable->pause_cckpd_value[max_level]);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CCK_PD, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): Write CCKPD (0x%x) of level (%d)\n", 
				pDM_DigTable->pause_cckpd_value[max_level], max_level));
			break;
		}
		break;
	}
	default:
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CCK_PD, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): Wrong  type !!\n"));
		break;
	}	
	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CCK_PD, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): pause level = 0x%x, Current value = 0x%x\n", pDM_DigTable->pause_cckpd_level, CCKPDThreshold));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CCK_PD, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): pause value = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", 
		pDM_DigTable->pause_cckpd_value[7], pDM_DigTable->pause_cckpd_value[6], pDM_DigTable->pause_cckpd_value[5], pDM_DigTable->pause_cckpd_value[4],
		pDM_DigTable->pause_cckpd_value[3], pDM_DigTable->pause_cckpd_value[2], pDM_DigTable->pause_cckpd_value[1], pDM_DigTable->pause_cckpd_value[0]));
}


VOID 
odm_CCKPacketDetectionThresh(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PFALSE_ALARM_STATISTICS 	FalseAlmCnt = (PFALSE_ALARM_STATISTICS)PhyDM_Get_Structure( pDM_Odm, PHYDM_FALSEALMCNT);
	u1Byte						CurCCK_CCAThres, RSSI_thd = 55;



	if((!(pDM_Odm->SupportAbility & ODM_BB_CCK_PD)) ||(!(pDM_Odm->SupportAbility & ODM_BB_FA_CNT)))
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_CCK_PD, ODM_DBG_LOUD, ("odm_CCKPacketDetectionThresh()  return==========\n"));
#ifdef MCR_WIRELESS_EXTEND
		ODM_Write_CCK_CCA_Thres(pDM_Odm, 0x43);
#endif
		return;
	}

#if (DM_ODM_SUPPORT_TYPE & (ODM_CE))
	if(pDM_Odm->external_lna_2g)
		return;
#endif

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CCK_PD, ODM_DBG_LOUD, ("odm_CCKPacketDetectionThresh()  ==========>\n"));

	if (pDM_Odm->bLinked)
	{
		if (pDM_Odm->RSSI_Min > RSSI_thd)
			CurCCK_CCAThres = 0xcd;
		else if ((pDM_Odm->RSSI_Min <= RSSI_thd) && (pDM_Odm->RSSI_Min > 10))
			CurCCK_CCAThres = 0x83;
		else
		{
			if(FalseAlmCnt->cnt_cck_fail > 1000)
				CurCCK_CCAThres = 0x83;
			else
				CurCCK_CCAThres = 0x40;
		}
	} else {
		if(FalseAlmCnt->cnt_cck_fail > 1000)
			CurCCK_CCAThres = 0x83;
		else
			CurCCK_CCAThres = 0x40;
	}
	
		ODM_Write_CCK_CCA_Thres(pDM_Odm, CurCCK_CCAThres);

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CCK_PD, ODM_DBG_LOUD, ("odm_CCKPacketDetectionThresh()  CurCCK_CCAThres = 0x%x\n",CurCCK_CCAThres));
}

VOID
ODM_Write_CCK_CCA_Thres(
	IN	PVOID			pDM_VOID,
	IN	u1Byte			CurCCK_CCAThres
	)
{
	PDM_ODM_T			pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T				pDM_DigTable = &pDM_Odm->DM_DigTable;

	if(pDM_DigTable->CurCCK_CCAThres!=CurCCK_CCAThres)		//modify by Guo.Mingzhi 2012-01-03
	{
		ODM_Write1Byte(pDM_Odm, ODM_REG(CCK_CCA,pDM_Odm), CurCCK_CCAThres);
	}
	pDM_DigTable->PreCCK_CCAThres = pDM_DigTable->CurCCK_CCAThres;
	pDM_DigTable->CurCCK_CCAThres = CurCCK_CCAThres;
}
