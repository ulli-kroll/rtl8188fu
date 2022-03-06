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

//
// ODM IO Relative API.
//

u1Byte
ODM_Read1Byte(
	IN 	PDM_ODM_T		pDM_Odm,
	IN	u4Byte			RegAddr
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return rtw_read8(Adapter,RegAddr);
#endif	

}


u2Byte
ODM_Read2Byte(
	IN 	PDM_ODM_T		pDM_Odm,
	IN	u4Byte			RegAddr
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return rtw_read16(Adapter,RegAddr);
#endif	

}


u4Byte
ODM_Read4Byte(
	IN 	PDM_ODM_T		pDM_Odm,
	IN	u4Byte			RegAddr
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return rtw_read32(Adapter,RegAddr);
#endif	

}


VOID
ODM_Write1Byte(
	IN 	PDM_ODM_T		pDM_Odm,
	IN	u4Byte			RegAddr,
	IN	u1Byte			Data
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	rtw_write8(Adapter,RegAddr, Data);
#endif
	
}


VOID
ODM_Write2Byte(
	IN 	PDM_ODM_T		pDM_Odm,
	IN	u4Byte			RegAddr,
	IN	u2Byte			Data
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	rtw_write16(Adapter,RegAddr, Data);
#endif	

}


VOID
ODM_Write4Byte(
	IN 	PDM_ODM_T		pDM_Odm,
	IN	u4Byte			RegAddr,
	IN	u4Byte			Data
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	rtw_write32(Adapter,RegAddr, Data);
#endif	

}


VOID
ODM_SetMACReg(	
	IN 	PDM_ODM_T	pDM_Odm,
	IN	u4Byte		RegAddr,
	IN	u4Byte		BitMask,
	IN	u4Byte		Data
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_CE))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PHY_SetBBReg(Adapter, RegAddr, BitMask, Data);
#endif	
}


u4Byte 
ODM_GetMACReg(	
	IN 	PDM_ODM_T	pDM_Odm,
	IN	u4Byte		RegAddr,
	IN	u4Byte		BitMask
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_CE))
	return PHY_QueryMacReg(pDM_Odm->Adapter, RegAddr, BitMask);
#endif	
}


VOID
ODM_SetBBReg(	
	IN 	PDM_ODM_T	pDM_Odm,
	IN	u4Byte		RegAddr,
	IN	u4Byte		BitMask,
	IN	u4Byte		Data
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_CE))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PHY_SetBBReg(Adapter, RegAddr, BitMask, Data);
#endif	
}


u4Byte 
ODM_GetBBReg(	
	IN 	PDM_ODM_T	pDM_Odm,
	IN	u4Byte		RegAddr,
	IN	u4Byte		BitMask
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_CE))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return PHY_QueryBBReg(Adapter, RegAddr, BitMask);
#endif	
}


VOID
ODM_SetRFReg(	
	IN 	PDM_ODM_T			pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E	eRFPath,
	IN	u4Byte				RegAddr,
	IN	u4Byte				BitMask,
	IN	u4Byte				Data
	)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_CE)
	PHY_SetRFReg(pDM_Odm->Adapter, eRFPath, RegAddr, BitMask, Data);
#endif	
}


u4Byte 
ODM_GetRFReg(	
	IN 	PDM_ODM_T			pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E	eRFPath,
	IN	u4Byte				RegAddr,
	IN	u4Byte				BitMask
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_CE))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return PHY_QueryRFReg(Adapter, eRFPath, RegAddr, BitMask);
#endif	
}




//
// ODM Memory relative API.
//
VOID
ODM_AllocateMemory(	
	IN 	PDM_ODM_T	pDM_Odm,
	OUT	PVOID		*pPtr,
	IN	u4Byte		length
	)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_CE )
	*pPtr = rtw_zvmalloc(length);
#endif	
}

// length could be ignored, used to detect memory leakage.
VOID
ODM_FreeMemory(	
	IN 	PDM_ODM_T	pDM_Odm,
	OUT	PVOID		pPtr,
	IN	u4Byte		length
	)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_CE )	
	rtw_vmfree(pPtr, length);
#endif	
}

VOID
ODM_MoveMemory(	
	IN 	PDM_ODM_T	pDM_Odm,
	OUT PVOID		pDest,
	IN  PVOID		pSrc,
	IN  u4Byte		Length
	)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_CE )	
	_rtw_memcpy(pDest, pSrc, Length);
#endif	
}

void ODM_Memory_Set(
	IN	PDM_ODM_T	pDM_Odm,
	IN	PVOID		pbuf,
	IN	s1Byte		value,
	IN	u4Byte		length
)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_CE )	
	_rtw_memset(pbuf,value, length);
#endif
}
s4Byte ODM_CompareMemory(
	IN 	PDM_ODM_T		pDM_Odm,
	IN	PVOID           pBuf1,
	IN	PVOID           pBuf2,
	IN	u4Byte          length
       )
{
#if (DM_ODM_SUPPORT_TYPE & ODM_CE )	
	return _rtw_memcmp(pBuf1,pBuf2,length);
#endif	
}



//
// ODM MISC relative API.
//
VOID
ODM_AcquireSpinLock(	
	IN 	PDM_ODM_T			pDM_Odm,
	IN	RT_SPINLOCK_TYPE	type
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER Adapter = pDM_Odm->Adapter;
	rtw_odm_acquirespinlock(Adapter, type);
#endif	
}
VOID
ODM_ReleaseSpinLock(	
	IN 	PDM_ODM_T			pDM_Odm,
	IN	RT_SPINLOCK_TYPE	type
	)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_CE )
	PADAPTER Adapter = pDM_Odm->Adapter;
	rtw_odm_releasespinlock(Adapter, type);
#endif	
}

//
// Work item relative API. FOr MP driver only~!
//
VOID
ODM_InitializeWorkItem(	
	IN 	PDM_ODM_T					pDM_Odm,
	IN	PRT_WORK_ITEM				pRtWorkItem,
	IN	RT_WORKITEM_CALL_BACK		RtWorkItemCallback,
	IN	PVOID						pContext,
	IN	const char*					szID
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	
#endif	
}


VOID
ODM_StartWorkItem(	
	IN	PRT_WORK_ITEM	pRtWorkItem
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)

#endif	
}


VOID
ODM_StopWorkItem(	
	IN	PRT_WORK_ITEM	pRtWorkItem
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)

#endif	
}


VOID
ODM_FreeWorkItem(	
	IN	PRT_WORK_ITEM	pRtWorkItem
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)

#endif	
}


VOID
ODM_ScheduleWorkItem(	
	IN	PRT_WORK_ITEM	pRtWorkItem
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)

#endif	
}


VOID
ODM_IsWorkItemScheduled(	
	IN	PRT_WORK_ITEM	pRtWorkItem
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)

#endif	
}



//
// ODM Timer relative API.
//
VOID
ODM_StallExecution(	
	IN	u4Byte	usDelay
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	rtw_udelay_os(usDelay);
#endif	
}

VOID
ODM_delay_ms(IN u4Byte	ms)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	rtw_mdelay_os(ms);
#endif			
}

VOID
ODM_delay_us(IN u4Byte	us)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	rtw_udelay_os(us);
#endif			
}

VOID
ODM_sleep_ms(IN u4Byte	ms)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	rtw_msleep_os(ms);
#endif		
}

VOID
ODM_sleep_us(IN u4Byte	us)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	rtw_usleep_os(us);
#endif		
}

VOID
ODM_SetTimer(	
	IN 	PDM_ODM_T		pDM_Odm,
	IN	PRT_TIMER 		pTimer, 
	IN	u4Byte 			msDelay
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	_set_timer(pTimer,msDelay ); //ms
#endif	

}

VOID
ODM_InitializeTimer(
	IN 	PDM_ODM_T			pDM_Odm,
	IN	PRT_TIMER 			pTimer, 
	IN	RT_TIMER_CALL_BACK	CallBackFunc, 
	IN	PVOID				pContext,
	IN	const char*			szID
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER Adapter = pDM_Odm->Adapter;
	_init_timer(pTimer,Adapter->pnetdev,CallBackFunc,pDM_Odm);
#endif	
}


VOID
ODM_CancelTimer(
	IN 	PDM_ODM_T		pDM_Odm,
	IN	PRT_TIMER		pTimer
	)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_CE)
	_cancel_timer_ex(pTimer);
#endif
}


VOID
ODM_ReleaseTimer(
	IN 	PDM_ODM_T		pDM_Odm,
	IN	PRT_TIMER		pTimer
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)

#endif
}

BOOLEAN
phydm_actingDetermine(
	IN PDM_ODM_T		pDM_Odm,
	IN PHYDM_ACTING_TYPE	type
	)
{
	BOOLEAN		ret = FALSE;
	PADAPTER	Adapter = pDM_Odm->Adapter;

#if (DM_ODM_SUPPORT_TYPE & ODM_CE)
	struct mlme_priv			*pmlmepriv = &(Adapter->mlmepriv);

	if (type == PhyDM_ACTING_AS_AP)
		ret = check_fwstate(pmlmepriv, WIFI_AP_STATE);
	else if (type == PhyDM_ACTING_AS_IBSS)
		ret = check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) || check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE);
#endif

	return ret;

}


u1Byte
phydm_trans_h2c_id(
	IN	PDM_ODM_T	pDM_Odm,
	IN	u1Byte		phydm_h2c_id
)
{
	u1Byte platform_h2c_id=0xff;

	
	switch(phydm_h2c_id)
	{
		//1 [0]
		case ODM_H2C_RSSI_REPORT:

			#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
				platform_h2c_id = H2C_RSSI_SETTING;

			#endif
			
				break;

		//1 [3]	
		case ODM_H2C_WIFI_CALIBRATION:
			#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
				
			#endif
			
				break;		
	
			
		//1 [4]
		case ODM_H2C_IQ_CALIBRATION:
			#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
			#endif
			
				break;
		//1 [5]
		case ODM_H2C_RA_PARA_ADJUST:

			#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
				
			#endif
			
				break;


		//1 [6]
		case PHYDM_H2C_DYNAMIC_TX_PATH:

			#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
			#endif
			
				break;

		/* [7]*/
		case PHYDM_H2C_FW_TRACE_EN:

			#if(DM_ODM_SUPPORT_TYPE & ODM_CE)

				
			#endif
			
				break;

		case PHYDM_H2C_TXBF:
		break;

		default:
			platform_h2c_id=0xff;
			break;	
	}	
	
	return platform_h2c_id;
	
}

//
// ODM FW relative API.
//

VOID
ODM_FillH2CCmd(
	IN	PDM_ODM_T		pDM_Odm,
	IN	u1Byte 			phydm_h2c_id,
	IN	u4Byte 			CmdLen,
	IN	pu1Byte			pCmdBuffer
)
{
	PADAPTER 	Adapter = pDM_Odm->Adapter;
	u1Byte		platform_h2c_id;

	platform_h2c_id=phydm_trans_h2c_id(pDM_Odm, phydm_h2c_id);

	if(platform_h2c_id==0xff)
	{
		ODM_RT_TRACE(pDM_Odm,PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("[H2C] Wrong H2C CMD-ID !! platform_h2c_id==0xff ,  PHYDM_ElementID=((%d )) \n",phydm_h2c_id));
		return;
	}

	#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
		rtw_hal_fill_h2c_cmd(Adapter, platform_h2c_id, CmdLen, pCmdBuffer);

	#endif
}

u1Byte
phydm_c2H_content_parsing(
	IN	PVOID			pDM_VOID,
	IN	u1Byte			c2hCmdId,
	IN	u1Byte			c2hCmdLen,
	IN	pu1Byte			tmpBuf
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u1Byte	Extend_c2hSubID = 0;
	u1Byte	find_c2h_cmd = TRUE;

	switch (c2hCmdId) {
	case PHYDM_C2H_DBG:
		if (pDM_Odm->SupportICType & (ODM_RTL8814A|ODM_RTL8822B))
			phydm_fw_trace_handler(pDM_Odm, tmpBuf, c2hCmdLen);
		
		break;

	case PHYDM_C2H_RA_RPT:
		phydm_c2h_ra_report_handler(pDM_Odm, tmpBuf, c2hCmdLen);
		break;

	case PHYDM_C2H_RA_PARA_RPT:
		ODM_C2HRaParaReportHandler(pDM_Odm, tmpBuf, c2hCmdLen);
		break;
		
	case PHYDM_C2H_DYNAMIC_TX_PATH_RPT:
		if (pDM_Odm->SupportICType & (ODM_RTL8814A))
			phydm_c2h_dtp_handler(pDM_Odm, tmpBuf, c2hCmdLen);
		
		break;
		
	case PHYDM_C2H_IQK_FINISH:
		break;

	case PHYDM_C2H_DBG_CODE:
		phydm_fw_trace_handler_code(pDM_Odm, tmpBuf, c2hCmdLen);
		break;	

	case PHYDM_C2H_EXTEND:
		Extend_c2hSubID = tmpBuf[0];
		if (Extend_c2hSubID == PHYDM_EXTEND_C2H_DBG_PRINT)
			phydm_fw_trace_handler_8051(pDM_Odm, tmpBuf, c2hCmdLen);
		
		break;

	default:
		find_c2h_cmd = FALSE;
		break;
	}

	return find_c2h_cmd;

}

u8Byte
ODM_GetCurrentTime(	
	IN 	PDM_ODM_T		pDM_Odm
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	return (u8Byte)rtw_get_current_time();
#endif
}

u8Byte
ODM_GetProgressingTime(	
	IN 	PDM_ODM_T		pDM_Odm,
	IN	u8Byte			Start_Time
	)
{
#if(DM_ODM_SUPPORT_TYPE & ODM_CE)
	return rtw_get_passing_time_ms((u4Byte)Start_Time);
#endif
}


