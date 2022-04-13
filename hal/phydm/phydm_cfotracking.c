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
#include "mp_precomp.h"
#include "phydm_precomp.h"

VOID
odm_SetCrystalCap(
	IN		PVOID					pDM_VOID,
	IN		u1Byte					CrystalCap
)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PCFO_TRACKING				pCfoTrack = (PCFO_TRACKING)PhyDM_Get_Structure( pDM_Odm, PHYDM_CFOTRACK);
	BOOLEAN 					bEEPROMCheck;
	PADAPTER					Adapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE				*pHalData = GET_HAL_DATA(Adapter);

	bEEPROMCheck = (pHalData->EEPROMVersion >= 0x01)?TRUE:FALSE;

	if(pCfoTrack->CrystalCap == CrystalCap)
		return;

	pCfoTrack->CrystalCap = CrystalCap;

	if (pDM_Odm->SupportICType & (ODM_RTL8188F)) {
		/* write 0x24[22:17] = 0x24[16:11] = CrystalCap */
		CrystalCap = CrystalCap & 0x3F;
		ODM_SetBBReg(pDM_Odm, REG_AFE_XTAL_CTRL, 0x007ff800, (CrystalCap|(CrystalCap << 6)));
	} else {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CFO_TRACKING, ODM_DBG_LOUD, ("odm_SetCrystalCap(): Use default setting.\n"));
		ODM_SetBBReg(pDM_Odm, REG_MAC_PHY_CTRL, 0xFFF000, (CrystalCap|(CrystalCap << 6)));
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CFO_TRACKING, ODM_DBG_LOUD, ("odm_SetCrystalCap(): CrystalCap = 0x%x\n", CrystalCap));
}

u1Byte
odm_GetDefaultCrytaltalCap(
	IN		PVOID					pDM_VOID
)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u1Byte						CrystalCap = 0x20;

	PADAPTER					Adapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE				*pHalData = GET_HAL_DATA(Adapter);

	CrystalCap = pHalData->CrystalCap;

	CrystalCap = CrystalCap & 0x3f;

	return CrystalCap;
}

VOID
odm_SetATCStatus(
	IN		PVOID					pDM_VOID,
	IN		BOOLEAN					ATCStatus
)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PCFO_TRACKING				pCfoTrack = (PCFO_TRACKING)PhyDM_Get_Structure( pDM_Odm, PHYDM_CFOTRACK);

	if(pCfoTrack->bATCStatus == ATCStatus)
		return;
	
	ODM_SetBBReg(pDM_Odm, ODM_REG(BB_ATC,pDM_Odm), ODM_BIT(BB_ATC,pDM_Odm), ATCStatus);
	pCfoTrack->bATCStatus = ATCStatus;
}

BOOLEAN
odm_GetATCStatus(
	IN		PVOID					pDM_VOID
)
{
	BOOLEAN						ATCStatus;
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;

	ATCStatus = (BOOLEAN)ODM_GetBBReg(pDM_Odm, ODM_REG(BB_ATC,pDM_Odm), ODM_BIT(BB_ATC,pDM_Odm));
	return ATCStatus;
}

VOID
ODM_CfoTrackingReset(
	IN		PVOID					pDM_VOID
)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PCFO_TRACKING				pCfoTrack = (PCFO_TRACKING)PhyDM_Get_Structure( pDM_Odm, PHYDM_CFOTRACK);

	pCfoTrack->DefXCap = odm_GetDefaultCrytaltalCap(pDM_Odm);
	pCfoTrack->bAdjust = TRUE;

	if(pCfoTrack->CrystalCap > pCfoTrack->DefXCap)
	{
		odm_SetCrystalCap(pDM_Odm, pCfoTrack->CrystalCap - 1);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CFO_TRACKING, ODM_DBG_LOUD,
			("ODM_CfoTrackingReset(): approch default value (0x%x)\n", pCfoTrack->CrystalCap));
	} else if (pCfoTrack->CrystalCap < pCfoTrack->DefXCap)
	{
		odm_SetCrystalCap(pDM_Odm, pCfoTrack->CrystalCap + 1);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CFO_TRACKING, ODM_DBG_LOUD,
			("ODM_CfoTrackingReset(): approch default value (0x%x)\n", pCfoTrack->CrystalCap));
	}

	odm_SetATCStatus(pDM_Odm, TRUE);
}

VOID
ODM_CfoTrackingInit(
	IN		PVOID					pDM_VOID
)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PCFO_TRACKING				pCfoTrack = (PCFO_TRACKING)PhyDM_Get_Structure( pDM_Odm, PHYDM_CFOTRACK);

	pCfoTrack->DefXCap = pCfoTrack->CrystalCap = odm_GetDefaultCrytaltalCap(pDM_Odm);
	pCfoTrack->bATCStatus = odm_GetATCStatus(pDM_Odm);
	pCfoTrack->bAdjust = TRUE;
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CFO_TRACKING, ODM_DBG_LOUD, ("ODM_CfoTracking_init()=========> \n"));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CFO_TRACKING, ODM_DBG_LOUD, ("ODM_CfoTracking_init(): bATCStatus = %d, CrystalCap = 0x%x \n",pCfoTrack->bATCStatus, pCfoTrack->DefXCap));
}

VOID
ODM_CfoTracking(
	IN		PVOID					pDM_VOID
)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PCFO_TRACKING				pCfoTrack = (PCFO_TRACKING)PhyDM_Get_Structure( pDM_Odm, PHYDM_CFOTRACK);
	int							CFO_kHz_A, CFO_kHz_B, CFO_ave = 0;
	int							CFO_ave_diff;
	int							CrystalCap = (int)pCfoTrack->CrystalCap;
	u1Byte						Adjust_Xtal = 1;

	//4 Support ability
	if(!(pDM_Odm->SupportAbility & ODM_BB_CFO_TRACKING))
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CFO_TRACKING, ODM_DBG_LOUD, ("ODM_CfoTracking(): Return: SupportAbility ODM_BB_CFO_TRACKING is disabled\n"));
		return;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CFO_TRACKING, ODM_DBG_LOUD, ("ODM_CfoTracking()=========> \n"));

	{	
		//4 No link or more than one entry
		ODM_CfoTrackingReset(pDM_Odm);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CFO_TRACKING, ODM_DBG_LOUD, ("ODM_CfoTracking(): Reset: bLinked = %d, bOneEntryOnly = %d\n", 
			pDM_Odm->bLinked, pDM_Odm->bOneEntryOnly));
	}
}

VOID
ODM_ParsingCFO(
	IN		PVOID			pDM_VOID,
	IN		PVOID			pPktinfo_VOID,
	IN		s1Byte* 			pcfotail
	)
{
	PDM_ODM_T				pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PODM_PACKET_INFO_T		pPktinfo = (PODM_PACKET_INFO_T)pPktinfo_VOID;
	PCFO_TRACKING			pCfoTrack = (PCFO_TRACKING)PhyDM_Get_Structure( pDM_Odm, PHYDM_CFOTRACK);
	u1Byte					i;

	if(!(pDM_Odm->SupportAbility & ODM_BB_CFO_TRACKING))
		return;

	if(pPktinfo->bPacketMatchBSSID)
	{				
		//3 Update CFO report for path-A & path-B
		// Only paht-A and path-B have CFO tail and short CFO
		for(i = ODM_RF_PATH_A; i <= ODM_RF_PATH_B; i++)   
		{
			pCfoTrack->CFO_tail[i] = (int)pcfotail[i];
	 	}

		//3 Update packet counter
		if(pCfoTrack->packetCount == 0xffffffff)
			pCfoTrack->packetCount = 0;
		else
	 		pCfoTrack->packetCount++;
	}
}

