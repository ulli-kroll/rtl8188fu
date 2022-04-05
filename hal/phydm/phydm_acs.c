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


u1Byte
ODM_GetAutoChannelSelectResult(
	IN		PVOID			pDM_VOID,
	IN		u1Byte			Band
)
{
	PDM_ODM_T				pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PACS					pACS = &pDM_Odm->DM_ACS;

	if(Band == ODM_BAND_2_4G)
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ACS, ODM_DBG_LOUD, ("[ACS] ODM_GetAutoChannelSelectResult(): CleanChannel_2G(%d)\n", pACS->CleanChannel_2G));
		return (u1Byte)pACS->CleanChannel_2G;	
	}
	else
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ACS, ODM_DBG_LOUD, ("[ACS] ODM_GetAutoChannelSelectResult(): CleanChannel_5G(%d)\n", pACS->CleanChannel_5G));
		return (u1Byte)pACS->CleanChannel_5G;	
	}
}

VOID
odm_AutoChannelSelectSetting(
	IN		PVOID			pDM_VOID,
	IN		BOOLEAN			IsEnable
)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u2Byte						period = 0x2710;// 40ms in default
	u2Byte						NHMType = 0x7;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ACS, ODM_DBG_LOUD, ("odm_AutoChannelSelectSetting()=========> \n"));

	if(IsEnable)
	{//20 ms
		period = 0x1388;
		NHMType = 0x1;
	}

	else if (pDM_Odm->SupportICType & ODM_IC_11N_SERIES)
	{
		//PHY parameters initialize for n series
		ODM_Write2Byte(pDM_Odm, ODM_REG_NHM_TIMER_11N+2, period);	//0x894[31:16]=0x2710	Time duration for NHM unit: 4us, 0x2710=40ms
		//ODM_SetBBReg(pDM_Odm, ODM_REG_NHM_TH9_TH10_11N, BIT10|BIT9|BIT8, NHMType);	//0x890[9:8]=3			enable CCX		
	}
}

VOID
odm_AutoChannelSelectInit(
	IN		PVOID			pDM_VOID
)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PACS						pACS = &pDM_Odm->DM_ACS;
	u1Byte						i;

	if(!(pDM_Odm->SupportAbility & ODM_BB_NHM_CNT))
		return;

	if(pACS->bForceACSResult)
		return;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ACS, ODM_DBG_LOUD, ("odm_AutoChannelSelectInit()=========> \n"));

	pACS->CleanChannel_2G = 1;
	pACS->CleanChannel_5G = 36;

	for (i = 0; i < ODM_MAX_CHANNEL_2G; ++i)
	{
		pACS->Channel_Info_2G[0][i] = 0;
		pACS->Channel_Info_2G[1][i] = 0;
	}

}

VOID
odm_AutoChannelSelectReset(
	IN		PVOID			pDM_VOID
)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PACS						pACS = &pDM_Odm->DM_ACS;

	if(!(pDM_Odm->SupportAbility & ODM_BB_NHM_CNT))
		return;

	if(pACS->bForceACSResult)
		return;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ACS, ODM_DBG_LOUD, ("odm_AutoChannelSelectReset()=========> \n"));

	odm_AutoChannelSelectSetting(pDM_Odm,TRUE);// for 20ms measurement
	Phydm_NHMCounterStatisticsReset(pDM_Odm);
}

VOID
odm_AutoChannelSelect(
	IN		PVOID			pDM_VOID,
	IN		u1Byte			Channel
)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PACS						pACS = &pDM_Odm->DM_ACS;
	u1Byte						ChannelIDX = 0, SearchIDX = 0;
	u2Byte						MaxScore=0;

	if(!(pDM_Odm->SupportAbility & ODM_BB_NHM_CNT))
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_AutoChannelSelect(): Return: SupportAbility ODM_BB_NHM_CNT is disabled\n"));
		return;
	}

	if(pACS->bForceACSResult)
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_AutoChannelSelect(): Force 2G clean channel = %d, 5G clean channel = %d\n",
			pACS->CleanChannel_2G, pACS->CleanChannel_5G));
		return;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ACS, ODM_DBG_LOUD, ("odm_AutoChannelSelect(): Channel = %d=========> \n", Channel));

	Phydm_GetNHMCounterStatistics(pDM_Odm);
	odm_AutoChannelSelectSetting(pDM_Odm,FALSE);

	if(Channel >=1 && Channel <=14)
	{
		ChannelIDX = Channel - 1;
		pACS->Channel_Info_2G[1][ChannelIDX]++;
		
		if(pACS->Channel_Info_2G[1][ChannelIDX] >= 2)
			pACS->Channel_Info_2G[0][ChannelIDX] = (pACS->Channel_Info_2G[0][ChannelIDX] >> 1) + 
			(pACS->Channel_Info_2G[0][ChannelIDX] >> 2) + (pDM_Odm->NHM_cnt_0>>2);
		else
			pACS->Channel_Info_2G[0][ChannelIDX] = pDM_Odm->NHM_cnt_0;
	
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ACS, ODM_DBG_LOUD, ("odm_AutoChannelSelect(): NHM_cnt_0 = %d \n", pDM_Odm->NHM_cnt_0));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ACS, ODM_DBG_LOUD, ("odm_AutoChannelSelect(): Channel_Info[0][%d] = %d, Channel_Info[1][%d] = %d\n", ChannelIDX, pACS->Channel_Info_2G[0][ChannelIDX], ChannelIDX, pACS->Channel_Info_2G[1][ChannelIDX]));

		for(SearchIDX = 0; SearchIDX < ODM_MAX_CHANNEL_2G; SearchIDX++)
		{
			if(pACS->Channel_Info_2G[1][SearchIDX] != 0)
			{
				if(pACS->Channel_Info_2G[0][SearchIDX] >= MaxScore)
				{
					MaxScore = pACS->Channel_Info_2G[0][SearchIDX];
					pACS->CleanChannel_2G = SearchIDX+1;
				}
			}
		}
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ACS, ODM_DBG_LOUD, ("(1)odm_AutoChannelSelect(): 2G: CleanChannel_2G = %d, MaxScore = %d \n", 
			pACS->CleanChannel_2G, MaxScore));

	}
	else if(Channel >= 36)
	{
		// Need to do
		pACS->CleanChannel_5G = Channel;
	}
}

VOID
phydm_CLMInit(
	IN		PVOID			pDM_VOID,
	IN		u2Byte			sampleNum			/*unit : 4us*/
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;		

	if (pDM_Odm->SupportICType & ODM_IC_11N_SERIES) {
		ODM_SetBBReg(pDM_Odm, ODM_REG_CLM_TIME_PERIOD_11N, bMaskLWord, sampleNum);	/*4us sample 1 time*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_CLM_11N, BIT8, 0x1);								/*Enable CCX for CLM*/	
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ACS, ODM_DBG_LOUD, ("[%s] : CLM sampleNum = %d\n", __func__, sampleNum));
		
}

VOID
phydm_CLMtrigger(
	IN		PVOID			pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;

	if (pDM_Odm->SupportICType & ODM_IC_11N_SERIES) {
		ODM_SetBBReg(pDM_Odm, ODM_REG_CLM_11N, BIT0, 0x0);	/*Trigger CLM*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_CLM_11N, BIT0, 0x1);
	}
}

BOOLEAN
phydm_checkCLMready(
	IN		PVOID			pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u4Byte			value32 = 0;
	BOOLEAN			ret = FALSE;
	
	if (pDM_Odm->SupportICType & ODM_IC_11N_SERIES)
		value32 = ODM_GetBBReg(pDM_Odm, ODM_REG_CLM_READY_11N, bMaskDWord);				/*make sure CLM calc is ready*/

	if (value32 & BIT16)
		ret = TRUE;
	else
		ret = FALSE;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ACS, ODM_DBG_LOUD, ("[%s] : CLM ready = %d\n", __func__, ret));

	return ret;
}

u2Byte
phydm_getCLMresult(
	IN		PVOID			pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u4Byte			value32 = 0;
	u2Byte			results = 0;
	
	if (pDM_Odm->SupportICType & ODM_IC_11N_SERIES)
		value32 = ODM_GetBBReg(pDM_Odm, ODM_REG_CLM_RESULT_11N, bMaskDWord);				/*read CLM calc result*/

	results = (u2Byte)(value32 & bMaskLWord);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ACS, ODM_DBG_LOUD, ("[%s] : CLM result = %d\n", __func__, results));
	
	return results;
/*results are number of CCA times in sampleNum*/
}

