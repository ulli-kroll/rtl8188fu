/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
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
/*============================================================ */
/* Description: */
/* */
/* This file is for 92CE/92CU dynamic mechanism only */
/* */
/* */
/*============================================================ */
#define _RTL8188F_DM_C_

/*============================================================ */
/* include files */
/*============================================================ */
#include <rtl8188f_hal.h>

/*============================================================ */
/* Global var */
/*============================================================ */


static VOID
dm_CheckProtection(
	IN	PADAPTER	Adapter
)
{
#if 0
	PMGNT_INFO		pMgntInfo = &(Adapter->MgntInfo);
	u1Byte			CurRate, RateThreshold;

	if (pMgntInfo->pHTInfo->bCurBW40MHz)
		RateThreshold = MGN_MCS1;
	else
		RateThreshold = MGN_MCS3;

	if (Adapter->TxStats.CurrentInitTxRate <= RateThreshold) {
		pMgntInfo->bDmDisableProtect = TRUE;
		DbgPrint("Forced disable protect: %x\n", Adapter->TxStats.CurrentInitTxRate);
	} else {
		pMgntInfo->bDmDisableProtect = FALSE;
		DbgPrint("Enable protect: %x\n", Adapter->TxStats.CurrentInitTxRate);
	}
#endif
}

static VOID
dm_CheckStatistics(
	IN	PADAPTER	Adapter
)
{
#if 0
	if (!Adapter->MgntInfo.bMediaConnect)
		return;

	/*2008.12.10 tynli Add for getting Current_Tx_Rate_Reg flexibly. */
	rtw_hal_get_hwreg(Adapter, HW_VAR_INIT_TX_RATE, (pu1Byte)(&Adapter->TxStats.CurrentInitTxRate));

	/* Calculate current Tx Rate(Successful transmited!!) */

	/* Calculate current Rx Rate(Successful received!!) */

	/*for tx tx retry count */
	rtw_hal_get_hwreg(Adapter, HW_VAR_RETRY_COUNT, (pu1Byte)(&Adapter->TxStats.NumTxRetryCount));
#endif
}

/* */
/* Initialize GPIO setting registers */
/* */
static void
dm_InitGPIOSetting(
	IN	PADAPTER	Adapter
)
{
	PHAL_DATA_TYPE		pHalData = GET_HAL_DATA(Adapter);

	u8	tmp1byte;

	tmp1byte = rtw_read8(Adapter, REG_GPIO_MUXCFG);
	tmp1byte &= (GPIOSEL_GPIO | ~GPIOSEL_ENBT);

	rtw_write8(Adapter, REG_GPIO_MUXCFG, tmp1byte);
}
/*============================================================ */
/* functions */
/*============================================================ */
static void Init_ODM_ComInfo_8188f(PADAPTER	Adapter)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	PDM_ODM_T		pDM_Odm = &(pHalData->odmpriv);
	u32 SupportAbility = 0;
	u8	cut_ver, fab_ver;

	Init_ODM_ComInfo(Adapter);

	ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_PACKAGE_TYPE, pHalData->PackageType);

	fab_ver = ODM_TSMC;
	cut_ver = GET_CVID_CUT_VERSION(pHalData->VersionID);

	DBG_871X("%s(): fab_ver=%d cut_ver=%d\n", __func__, fab_ver, cut_ver);
	ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_FAB_VER, fab_ver);
	ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_CUT_VER, cut_ver);

	SupportAbility =	ODM_RF_CALIBRATION		|
						ODM_RF_TX_PWR_TRACK	/*| */
						;

	ODM_CmnInfoUpdate(pDM_Odm, ODM_CMNINFO_ABILITY, SupportAbility);
}

static void Update_ODM_ComInfo_8188f(PADAPTER	Adapter)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	PDM_ODM_T		pDM_Odm = &(pHalData->odmpriv);
	u32 SupportAbility = 0;

	SupportAbility = 0
					 | ODM_BB_DIG
					 | ODM_BB_RA_MASK
					 | ODM_BB_DYNAMIC_TXPWR
					 | ODM_BB_FA_CNT
					 | ODM_BB_RSSI_MONITOR
					 | ODM_BB_CCK_PD
					 /* | ODM_BB_PWR_SAVE */
					 | ODM_BB_CFO_TRACKING
					 | ODM_MAC_EDCA_TURBO
					 | ODM_RF_TX_PWR_TRACK
					 | ODM_RF_CALIBRATION
					 | ODM_BB_NHM_CNT
					 /*| ODM_BB_PWR_TRAIN */
					 ;

	if (rtw_odm_adaptivity_needed(Adapter) == _TRUE) {
		rtw_odm_adaptivity_config_msg(RTW_DBGDUMP, Adapter);
		SupportAbility |= ODM_BB_ADAPTIVITY;
	}


	ODM_CmnInfoUpdate(pDM_Odm, ODM_CMNINFO_ABILITY, SupportAbility);
}

void
rtl8188f_InitHalDm(
	IN	PADAPTER	Adapter
)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	PDM_ODM_T		pDM_Odm = &(pHalData->odmpriv);

	u8	i;

#ifdef CONFIG_USB_HCI
	dm_InitGPIOSetting(Adapter);
#endif

	pHalData->DM_Type = DM_Type_ByDriver;

	Update_ODM_ComInfo_8188f(Adapter);

	ODM_DMInit(pDM_Odm);

}

#if 0 /* function never used */
static void
FindMinimumRSSI_8188f(
	IN	PADAPTER	pAdapter
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	struct mlme_priv	*pmlmepriv = &pAdapter->mlmepriv;

	/*1 1.Determine the minimum RSSI */



	if ((check_fwstate(pmlmepriv, _FW_LINKED) == _FALSE) &&
		(pHalData->EntryMinUndecoratedSmoothedPWDB == 0)) {
		pHalData->MinUndecoratedPWDBForDM = 0;
		/*ODM_RT_TRACE(pDM_Odm,COMP_BB_POWERSAVING, DBG_LOUD, ("Not connected to any\n")); */
	}
	if (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE) {	/* Default port */
#if 0
		if ((check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE) ||
			(check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == _TRUE) ||
			(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE)) {
			pHalData->MinUndecoratedPWDBForDM = pHalData->EntryMinUndecoratedSmoothedPWDB;
			/*ODM_RT_TRACE(pDM_Odm,COMP_BB_POWERSAVING, DBG_LOUD, ("AP Client PWDB = 0x%x\n", pHalData->MinUndecoratedPWDBForDM)); */
		} else { /*for STA mode */
			pHalData->MinUndecoratedPWDBForDM = pHalData->UndecoratedSmoothedPWDB;
			/*ODM_RT_TRACE(pDM_Odm,COMP_BB_POWERSAVING, DBG_LOUD, ("STA Default Port PWDB = 0x%x\n", pHalData->MinUndecoratedPWDBForDM)); */
		}
#else
		pHalData->MinUndecoratedPWDBForDM = pHalData->EntryMinUndecoratedSmoothedPWDB;
#endif
	} else { /* associated entry pwdb */
		pHalData->MinUndecoratedPWDBForDM = pHalData->EntryMinUndecoratedSmoothedPWDB;
		/*ODM_RT_TRACE(pDM_Odm,COMP_BB_POWERSAVING, DBG_LOUD, ("AP Ext Port or disconnect PWDB = 0x%x\n", pHalData->MinUndecoratedPWDBForDM)); */
	}

	/*odm_FindMinimumRSSI_Dmsp(pAdapter); */
	/*DBG_8192C("%s=>MinUndecoratedPWDBForDM(%d)\n",__func__,pHalData->MinUndecoratedPWDBForDM); */
	/*ODM_RT_TRACE(pDM_Odm,COMP_DIG, DBG_LOUD, ("MinUndecoratedPWDBForDM =%d\n",pHalData->MinUndecoratedPWDBForDM)); */
}
#endif

VOID
rtl8188f_HalDmWatchDog(
	IN	PADAPTER	Adapter
)
{
	BOOLEAN		bFwCurrentInPSMode = _FALSE;
	BOOLEAN		bFwPSAwake = _TRUE;
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);


	if (!rtw_is_hw_init_completed(Adapter))
		goto skip_dm;

#ifdef CONFIG_LPS
	bFwCurrentInPSMode = adapter_to_pwrctl(Adapter)->bFwCurrentInPSMode;
	rtw_hal_get_hwreg(Adapter, HW_VAR_FWLPS_RF_ON, (u8 *)(&bFwPSAwake));
#endif

	if ((rtw_is_hw_init_completed(Adapter))
		&& ((!bFwCurrentInPSMode) && bFwPSAwake)) {
		/* */
		/* Calculate Tx/Rx statistics. */
		/* */
		dm_CheckStatistics(Adapter);
		rtw_hal_check_rxfifo_full(Adapter);
		/* */
		/* Dynamically switch RTS/CTS protection. */
		/* */
		/*dm_CheckProtection(Adapter); */

	}

	/*ODM */
	if (rtw_is_hw_init_completed(Adapter)) {
		u8	bLinked = _FALSE;
		u8	bsta_state = _FALSE;
		u8	bBtDisabled = _TRUE;

		if (rtw_linked_check(Adapter)) {
			bLinked = _TRUE;
			if (check_fwstate(&Adapter->mlmepriv, WIFI_STATION_STATE))
				bsta_state = _TRUE;
		}

		ODM_CmnInfoUpdate(&pHalData->odmpriv , ODM_CMNINFO_LINK, bLinked);
		ODM_CmnInfoUpdate(&pHalData->odmpriv , ODM_CMNINFO_STATION_STATE, bsta_state);

		/*FindMinimumRSSI_8188f(Adapter); */
		/*ODM_CmnInfoUpdate(&pHalData->odmpriv ,ODM_CMNINFO_RSSI_MIN, pHalData->MinUndecoratedPWDBForDM); */

		ODM_CmnInfoUpdate(&pHalData->odmpriv, ODM_CMNINFO_BT_ENABLED, ((bBtDisabled == _TRUE) ? _FALSE : _TRUE));

		ODM_DMWatchdog(&pHalData->odmpriv);
	}

skip_dm:

	/* Check GPIO to determine current RF on/off and Pbc status. */
	/* Check Hardware Radio ON/OFF or not */
	/*if(Adapter->MgntInfo.PowerSaveControl.bGpioRfSw) */
	/*{ */
	/*RTPRINT(FPWR, PWRHW, ("dm_CheckRfCtrlGPIO\n")); */
	/*	dm_CheckRfCtrlGPIO(Adapter); */
	/*} */
	return;
}

void rtl8188f_hal_dm_in_lps(PADAPTER padapter)
{
	u32	PWDB_rssi = 0;
	struct mlme_priv 	*pmlmepriv = &padapter->mlmepriv;
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(padapter);
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	pDIG_T	pDM_DigTable = &pDM_Odm->DM_DigTable;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *psta = NULL;

	DBG_871X("%s, RSSI_Min=%d\n", __func__, pDM_Odm->RSSI_Min);

	/*update IGI */
	rtl8188fu_dm_write_dig(pDM_Odm, pDM_Odm->RSSI_Min);


	/*set rssi to fw */
	psta = rtw_get_stainfo(pstapriv, get_bssid(pmlmepriv));
	if (psta && (psta->rssi_stat.UndecoratedSmoothedPWDB > 0)) {
		PWDB_rssi = (psta->mac_id | (psta->rssi_stat.UndecoratedSmoothedPWDB << 16));

		rtl8188f_set_rssi_cmd(padapter, (u8 *)&PWDB_rssi);
	}

}

void rtl8188f_HalDmWatchDog_in_LPS(IN	PADAPTER	Adapter)
{
	u8	bLinked = _FALSE;
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	struct mlme_priv 	*pmlmepriv = &Adapter->mlmepriv;
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	pDIG_T	pDM_DigTable = &pDM_Odm->DM_DigTable;
	struct sta_priv *pstapriv = &Adapter->stapriv;
	struct sta_info *psta = NULL;

	if (!rtw_is_hw_init_completed(Adapter))
		goto skip_lps_dm;


	if (rtw_linked_check(Adapter))
		bLinked = _TRUE;

	ODM_CmnInfoUpdate(&pHalData->odmpriv , ODM_CMNINFO_LINK, bLinked);

	if (bLinked == _FALSE)
		goto skip_lps_dm;

	if (!(pDM_Odm->SupportAbility & ODM_BB_RSSI_MONITOR))
		goto skip_lps_dm;


	/*ODM_DMWatchdog(&pHalData->odmpriv); */
	/*Do DIG by RSSI In LPS-32K */

	/*.1 Find MIN-RSSI */
	psta = rtw_get_stainfo(pstapriv, get_bssid(pmlmepriv));
	if (psta == NULL)
		goto skip_lps_dm;

	pHalData->EntryMinUndecoratedSmoothedPWDB = psta->rssi_stat.UndecoratedSmoothedPWDB;

	DBG_871X("CurIGValue=%d, EntryMinUndecoratedSmoothedPWDB = %d\n", pDM_DigTable->CurIGValue, pHalData->EntryMinUndecoratedSmoothedPWDB);

	if (pHalData->EntryMinUndecoratedSmoothedPWDB <= 0)
		goto skip_lps_dm;

	pHalData->MinUndecoratedPWDBForDM = pHalData->EntryMinUndecoratedSmoothedPWDB;

	pDM_Odm->RSSI_Min = pHalData->MinUndecoratedPWDBForDM;

	/*if(pDM_DigTable->CurIGValue != pDM_Odm->RSSI_Min) */
	if ((pDM_DigTable->CurIGValue > pDM_Odm->RSSI_Min + 5) ||
		(pDM_DigTable->CurIGValue < pDM_Odm->RSSI_Min - 5))

	{
#ifdef CONFIG_LPS
		rtw_dm_in_lps_wk_cmd(Adapter);
#endif
	}


skip_lps_dm:

	return;

}

void rtl8188f_init_dm_priv(IN PADAPTER Adapter)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	PDM_ODM_T 		podmpriv = &pHalData->odmpriv;
	Init_ODM_ComInfo_8188f(Adapter);
	ODM_InitAllTimers(podmpriv);
}

void rtl8188f_deinit_dm_priv(IN PADAPTER Adapter)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	PDM_ODM_T 		podmpriv = &pHalData->odmpriv;
	ODM_CancelAllTimers(podmpriv);

}

