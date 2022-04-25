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


#define 	CALCULATE_SWINGTALBE_OFFSET(_offset, _direction, _size, _deltaThermal) \
					do {\
						for(_offset = 0; _offset < _size; _offset++)\
						{\
							if(_deltaThermal < thermalThreshold[_direction][_offset])\
							{\
								if(_offset != 0)\
									_offset--;\
								break;\
							}\
						}			\
						if(_offset >= _size)\
							_offset = _size-1;\
					} while(0)

void ConfigureTxpowerTrack(
	IN		PVOID					pDM_VOID,
	OUT	PTXPWRTRACK_CFG	pConfig
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;

	ConfigureTxpowerTrack_8188F(pConfig);
}

//======================================================================
// <20121113, Kordan> This function should be called when TxAGC changed.
// Otherwise the previous compensation is gone, because we record the 
// delta of temperature between two TxPowerTracking watch dogs.
//
// NOTE: If Tx BB swing or Tx scaling is varified during run-time, still 
//       need to call this function.
//======================================================================
VOID
ODM_ClearTxPowerTrackingState(
	IN		PVOID					pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(pDM_Odm->Adapter);
	u1Byte 			p = 0;
	PODM_RF_CAL_T	pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);
	
	pRFCalibrateInfo->swing_idx_cck_base = pRFCalibrateInfo->default_cck_index;
	pRFCalibrateInfo->swing_idx_cck = pRFCalibrateInfo->default_cck_index;
	pDM_Odm->RFCalibrateInfo.cck_index = 0;
	
	for (p = ODM_RF_PATH_A; p < MAX_RF_PATH; ++p)
	{
		pRFCalibrateInfo->swing_idx_ofdm_base[p] = pRFCalibrateInfo->default_ofdm_index;
		pRFCalibrateInfo->swing_idx_ofdm[p] = pRFCalibrateInfo->default_ofdm_index;
		pRFCalibrateInfo->ofdm_index[p] = pRFCalibrateInfo->default_ofdm_index;

		pRFCalibrateInfo->power_index_offset[p] = 0;
		pRFCalibrateInfo->delta_power_index[p] = 0;
		pRFCalibrateInfo->delta_power_index_last[p] = 0;

		pRFCalibrateInfo->absolute_ofdm_swing_idx[p] = 0;    /* Initial Mix mode power tracking*/
		pRFCalibrateInfo->remnant_ofdm_swing_idx[p] = 0;
		pRFCalibrateInfo->KfreeOffset[p] = 0;
	}
	
	pRFCalibrateInfo->modify_txagc_flag_path_a = FALSE;       /*Initial at Modify Tx Scaling Mode*/
	pRFCalibrateInfo->modify_txagc_flag_path_b = FALSE;       /*Initial at Modify Tx Scaling Mode*/
	pRFCalibrateInfo->Modify_TxAGC_Flag_PathC = FALSE;       /*Initial at Modify Tx Scaling Mode*/
	pRFCalibrateInfo->Modify_TxAGC_Flag_PathD = FALSE;       /*Initial at Modify Tx Scaling Mode*/
	pRFCalibrateInfo->remnant_cck_idx = 0;
	pRFCalibrateInfo->ThermalValue = pHalData->EEPROMThermalMeter;
	
	pRFCalibrateInfo->Modify_TxAGC_Value_OFDM=0;		//modify by Mingzhi.Guo
}

VOID
ODM_TXPowerTrackingCallback_ThermalMeter(
	IN PADAPTER	Adapter
	)
{

	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	PODM_RF_CAL_T	pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);

	u1Byte			ThermalValue = 0, delta, delta_LCK, delta_IQK, p = 0, i = 0;
	u1Byte			ThermalValue_AVG_count = 0;
	u4Byte			ThermalValue_AVG = 0;	

	u1Byte			OFDM_min_index = 0;  // OFDM BB Swing should be less than +3.0dB, which is required by Arthur
	BOOLEAN			bTSSIenable = FALSE;

	TXPWRTRACK_CFG 	c;

	//4 1. The following TWO tables decide the final index of OFDM/CCK swing table.
	pu1Byte			deltaSwingTableIdx_TUP_A;
	pu1Byte			deltaSwingTableIdx_TDOWN_A;
	pu1Byte			deltaSwingTableIdx_TUP_B;
	pu1Byte			deltaSwingTableIdx_TDOWN_B;
	/*for 8814 add by Yu Chen*/
	
	//4 2. Initilization ( 7 steps in total )

	ConfigureTxpowerTrack(pDM_Odm, &c);

	rtl8188fu_get_delta_swing_table(pDM_Odm, (pu1Byte*)&deltaSwingTableIdx_TUP_A, (pu1Byte*)&deltaSwingTableIdx_TDOWN_A,
						  (pu1Byte*)&deltaSwingTableIdx_TUP_B, (pu1Byte*)&deltaSwingTableIdx_TDOWN_B);
	
	pDM_Odm->RFCalibrateInfo.bTXPowerTrackingInit = TRUE;
    
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, 
		("===>ODM_TXPowerTrackingCallback_ThermalMeter Start\n pRFCalibrateInfo->BbSwingIdxCckBase: %d, pRFCalibrateInfo->BbSwingIdxOfdmBase[A]: %d, pRFCalibrateInfo->DefaultOfdmIndex: %d\n", 
		pRFCalibrateInfo->swing_idx_cck_base, pRFCalibrateInfo->BbSwingIdxOfdmBase[ODM_RF_PATH_A], pRFCalibrateInfo->default_ofdm_index));

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, 
		("pHalData->EEPROMThermalMeter %d\n", pHalData->EEPROMThermalMeter));
	ThermalValue = (u1Byte)ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_T_METER_8188F, 0xfc00);	//0x42: RF Reg[15:10] 88E
	if(pHalData->EEPROMThermalMeter == 0 || 
		pHalData->EEPROMThermalMeter == 0xFF)
		return;


	//4 3. Initialize ThermalValues of RFCalibrateInfo

	if(pDM_Odm->RFCalibrateInfo.bReloadtxpowerindex)
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,("reload ofdm index for band switch\n"));				
	}

	//4 4. Calculate average thermal meter
	
	pDM_Odm->RFCalibrateInfo.ThermalValue_AVG[pDM_Odm->RFCalibrateInfo.ThermalValue_AVG_index] = ThermalValue;
	pDM_Odm->RFCalibrateInfo.ThermalValue_AVG_index++;
	if(pDM_Odm->RFCalibrateInfo.ThermalValue_AVG_index == AVG_THERMAL_NUM_8188F)   //Average times =  c.AverageThermalNum
		pDM_Odm->RFCalibrateInfo.ThermalValue_AVG_index = 0;

	for(i = 0; i < AVG_THERMAL_NUM_8188F; i++)
	{
		if(pDM_Odm->RFCalibrateInfo.ThermalValue_AVG[i])
		{
			ThermalValue_AVG += pDM_Odm->RFCalibrateInfo.ThermalValue_AVG[i];
			ThermalValue_AVG_count++;
		}
	}

	if(ThermalValue_AVG_count)               //Calculate Average ThermalValue after average enough times
	{
		ThermalValue = (u1Byte)(ThermalValue_AVG / ThermalValue_AVG_count);
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
			("AVG Thermal Meter = 0x%X, EFUSE Thermal Base = 0x%X\n", ThermalValue, pHalData->EEPROMThermalMeter));					
	}
			
	//4 5. Calculate delta, delta_LCK, delta_IQK.

	//"delta" here is used to determine whether thermal value changes or not.
	delta 	  = (ThermalValue > pDM_Odm->RFCalibrateInfo.ThermalValue)?(ThermalValue - pDM_Odm->RFCalibrateInfo.ThermalValue):(pDM_Odm->RFCalibrateInfo.ThermalValue - ThermalValue);
	delta_LCK = (ThermalValue > pDM_Odm->RFCalibrateInfo.ThermalValue_LCK)?(ThermalValue - pDM_Odm->RFCalibrateInfo.ThermalValue_LCK):(pDM_Odm->RFCalibrateInfo.ThermalValue_LCK - ThermalValue);
	delta_IQK = (ThermalValue > pDM_Odm->RFCalibrateInfo.ThermalValue_IQK)?(ThermalValue - pDM_Odm->RFCalibrateInfo.ThermalValue_IQK):(pDM_Odm->RFCalibrateInfo.ThermalValue_IQK - ThermalValue);

	if (pDM_Odm->RFCalibrateInfo.ThermalValue_IQK == 0xff) {	/*no PG, use thermal value for IQK*/
		pDM_Odm->RFCalibrateInfo.ThermalValue_IQK = ThermalValue;
		delta_IQK = (ThermalValue > pDM_Odm->RFCalibrateInfo.ThermalValue_IQK)?(ThermalValue - pDM_Odm->RFCalibrateInfo.ThermalValue_IQK):(pDM_Odm->RFCalibrateInfo.ThermalValue_IQK - ThermalValue);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("no PG, use ThermalValue for IQK\n"));
	}
	
	/*4 6. If necessary, do LCK.*/	
	if (pDM_Odm->RFCalibrateInfo.ThermalValue_LCK == 0xff) {
		/*no PG , do LCK at initial status*/
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("no PG, do LCK\n"));
		pDM_Odm->RFCalibrateInfo.ThermalValue_LCK = ThermalValue;
		rtl8188fu_phy_lc_calibrate(pDM_Odm);
		delta_LCK = (ThermalValue > pDM_Odm->RFCalibrateInfo.ThermalValue_LCK)?(ThermalValue - pDM_Odm->RFCalibrateInfo.ThermalValue_LCK):(pDM_Odm->RFCalibrateInfo.ThermalValue_LCK - ThermalValue);
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("(delta, delta_LCK, delta_IQK) = (%d, %d, %d)\n", delta, delta_LCK, delta_IQK));
	/*DBG_871X("(delta, delta_LCK, delta_IQK) = (%d, %d, %d), %d\n", delta, delta_LCK, delta_IQK, c.Threshold_IQK);*/

	/* 4 6. If necessary, do LCK.*/

	if (delta_LCK >= IQK_THRESHOLD) {
		/* Delta temperature is equal to or larger than 20 centigrade.*/
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("delta_LCK(%d) >= Threshold_IQK(%d)\n", delta_LCK, IQK_THRESHOLD));
		pDM_Odm->RFCalibrateInfo.ThermalValue_LCK = ThermalValue;
		rtl8188fu_phy_lc_calibrate(pDM_Odm);
	}
	//3 7. If necessary, move the index of swing table to adjust Tx power.	
	
	if (delta > 0) 	{
		//"delta" here is used to record the absolute value of differrence.
		delta = ThermalValue > pHalData->EEPROMThermalMeter?(ThermalValue - pHalData->EEPROMThermalMeter):(pHalData->EEPROMThermalMeter - ThermalValue);		
		if (delta >= TXPWR_TRACK_TABLE_SIZE)
			delta = TXPWR_TRACK_TABLE_SIZE - 1;

		//4 7.1 The Final Power Index = BaseIndex + power_index_offset
		
		if(ThermalValue > pHalData->EEPROMThermalMeter) {
			for (p = ODM_RF_PATH_A; p < MAX_PATH_NUM_8188F; p++)  {
				pDM_Odm->RFCalibrateInfo.delta_power_index_last[p] = pDM_Odm->RFCalibrateInfo.delta_power_index[p];	/* recording power index offset */
				switch (p) {
				case ODM_RF_PATH_B:
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
						("deltaSwingTableIdx_TUP_B[%d] = %d\n", delta, deltaSwingTableIdx_TUP_B[delta])); 

					pDM_Odm->RFCalibrateInfo.delta_power_index[p] = deltaSwingTableIdx_TUP_B[delta];
					pRFCalibrateInfo->absolute_ofdm_swing_idx[p] = deltaSwingTableIdx_TUP_B[delta];       /* Record delta swing for mix mode power tracking */
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
						("******Temp is higher and pRFCalibrateInfo->absolute_ofdm_swing_idx[ODM_RF_PATH_B] = %d\n", pRFCalibrateInfo->absolute_ofdm_swing_idx[p]));  

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Temp is higher and pRFCalibrateInfo->absolute_ofdm_swing_idx[ODM_RF_PATH_A] = %d\n", pRFCalibrateInfo->absolute_ofdm_swing_idx[ODM_RF_PATH_A]));  


				break;

				default:
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, 
						("deltaSwingTableIdx_TUP_A[%d] = %d\n", delta, deltaSwingTableIdx_TUP_A[delta]));

					pDM_Odm->RFCalibrateInfo.delta_power_index[p] = deltaSwingTableIdx_TUP_A[delta];
					pRFCalibrateInfo->absolute_ofdm_swing_idx[p] = deltaSwingTableIdx_TUP_A[delta];        /* Record delta swing for mix mode power tracking */
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
						("******Temp is higher and pDM_Odm->absolute_ofdm_swing_idx[ODM_RF_PATH_A] = %d\n", pRFCalibrateInfo->absolute_ofdm_swing_idx[p]));
				break;
				}		
			}
		} else {
			for (p = ODM_RF_PATH_A; p < MAX_PATH_NUM_8188F; p++) {
				pDM_Odm->RFCalibrateInfo.delta_power_index_last[p] = pDM_Odm->RFCalibrateInfo.delta_power_index[p];	/* recording poer index offset */
				switch (p) {
				case ODM_RF_PATH_B:
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
						("deltaSwingTableIdx_TDOWN_B[%d] = %d\n", delta, deltaSwingTableIdx_TDOWN_B[delta]));  
					pDM_Odm->RFCalibrateInfo.delta_power_index[p] = -1 * deltaSwingTableIdx_TDOWN_B[delta];
					pRFCalibrateInfo->absolute_ofdm_swing_idx[p] = -1 * deltaSwingTableIdx_TDOWN_B[delta];        /* Record delta swing for mix mode power tracking */
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
						("******Temp is lower and pDM_Odm->absolute_ofdm_swing_idx[ODM_RF_PATH_B] = %d\n", pRFCalibrateInfo->absolute_ofdm_swing_idx[p])); 
				break;
					
				default:
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
						("deltaSwingTableIdx_TDOWN_A[%d] = %d\n", delta, deltaSwingTableIdx_TDOWN_A[delta]));  
					pDM_Odm->RFCalibrateInfo.delta_power_index[p] = -1 * deltaSwingTableIdx_TDOWN_A[delta];
					pRFCalibrateInfo->absolute_ofdm_swing_idx[p] =  -1 * deltaSwingTableIdx_TDOWN_A[delta];        /* Record delta swing for mix mode power tracking */
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
						("******Temp is lower and pDM_Odm->absolute_ofdm_swing_idx[ODM_RF_PATH_A] = %d\n", pRFCalibrateInfo->absolute_ofdm_swing_idx[p]));  
				break;
				}	
			}
		}
		
		for (p = ODM_RF_PATH_A; p < MAX_PATH_NUM_8188F; p++) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
			("\n\n=========================== [Path-%d] Calculating power_index_offset===========================\n", p));  
			if (pDM_Odm->RFCalibrateInfo.delta_power_index[p] == pDM_Odm->RFCalibrateInfo.delta_power_index_last[p])         /* If Thermal value changes but lookup table value still the same */
				pDM_Odm->RFCalibrateInfo.power_index_offset[p] = 0;
			else
				pDM_Odm->RFCalibrateInfo.power_index_offset[p] = pDM_Odm->RFCalibrateInfo.delta_power_index[p] - pDM_Odm->RFCalibrateInfo.delta_power_index_last[p];      /* Power Index Diff between 2 times Power Tracking */

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
				("[Path-%d] power_index_offset(%d) = delta_power_index(%d) - delta_power_index_last(%d)\n", p, pDM_Odm->RFCalibrateInfo.power_index_offset[p], pDM_Odm->RFCalibrateInfo.delta_power_index[p], pDM_Odm->RFCalibrateInfo.delta_power_index_last[p]));		
		
			pDM_Odm->RFCalibrateInfo.ofdm_index[p] = pRFCalibrateInfo->swing_idx_ofdm_base[p] + pDM_Odm->RFCalibrateInfo.power_index_offset[p];
			pDM_Odm->RFCalibrateInfo.cck_index = pRFCalibrateInfo->swing_idx_cck_base + pDM_Odm->RFCalibrateInfo.power_index_offset[p];

			pRFCalibrateInfo->swing_idx_cck = pDM_Odm->RFCalibrateInfo.cck_index;	
			pRFCalibrateInfo->swing_idx_ofdm[p] = pDM_Odm->RFCalibrateInfo.ofdm_index[p];



			/* *************Print BB Swing Base and Index Offset************* */

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
				("The 'CCK' final index(%d) = BaseIndex(%d) + power_index_offset(%d)\n", pRFCalibrateInfo->swing_idx_cck, pRFCalibrateInfo->swing_idx_cck_base, pDM_Odm->RFCalibrateInfo.power_index_offset[p]));
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
				("The 'OFDM' final index(%d) = BaseIndex[%d](%d) + power_index_offset(%d)\n", pRFCalibrateInfo->BbSwingIdxOfdm[p], p, pRFCalibrateInfo->BbSwingIdxOfdmBase[p], pDM_Odm->RFCalibrateInfo.power_index_offset[p]));

		    //4 7.1 Handle boundary conditions of index.
		
			if(pDM_Odm->RFCalibrateInfo.ofdm_index[p] > OFDM_TABLE_SIZE-1) {
				pDM_Odm->RFCalibrateInfo.ofdm_index[p] = OFDM_TABLE_SIZE-1;
			} else if (pDM_Odm->RFCalibrateInfo.ofdm_index[p] < OFDM_min_index) {
				pDM_Odm->RFCalibrateInfo.ofdm_index[p] = OFDM_min_index;
			}
		}
		
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
			("\n\n========================================================================================================\n"));  
		if(pDM_Odm->RFCalibrateInfo.cck_index > CCK_TABLE_SIZE_88F-1)
			pDM_Odm->RFCalibrateInfo.cck_index = CCK_TABLE_SIZE_88F-1;
		else if (pDM_Odm->RFCalibrateInfo.cck_index <= 0)
			pDM_Odm->RFCalibrateInfo.cck_index = 0;
	} else {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
			("The thermal meter is unchanged or TxPowerTracking OFF(%d): ThermalValue: %d , pDM_Odm->RFCalibrateInfo.ThermalValue: %d\n", 
			pDM_Odm->RFCalibrateInfo.TxPowerTrackControl, ThermalValue, pDM_Odm->RFCalibrateInfo.ThermalValue));
		
		for (p = ODM_RF_PATH_A; p < MAX_PATH_NUM_8188F; p++) 		
			pDM_Odm->RFCalibrateInfo.power_index_offset[p] = 0;
	}

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
		("TxPowerTracking: [CCK] Swing Current Index: %d, Swing Base Index: %d\n", 
		pDM_Odm->RFCalibrateInfo.cck_index, pRFCalibrateInfo->swing_idx_cck_base));       /*Print Swing base & current*/
			
	for (p = ODM_RF_PATH_A; p < MAX_PATH_NUM_8188F; p++) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
			("TxPowerTracking: [OFDM] Swing Current Index: %d, Swing Base Index[%d]: %d\n",
			pDM_Odm->RFCalibrateInfo.ofdm_index[p], p, pRFCalibrateInfo->BbSwingIdxOfdmBase[p]));
	}
	
	if ((pDM_Odm->RFCalibrateInfo.power_index_offset[ODM_RF_PATH_A] != 0 ||
		pDM_Odm->RFCalibrateInfo.power_index_offset[ODM_RF_PATH_B] != 0) && 
		(pHalData->EEPROMThermalMeter != 0xff)) {
		//4 7.2 Configure the Swing Table to adjust Tx Power.
		
		//
		// 2012/04/23 MH According to Luke's suggestion, we can not write BB digital
		// to increase TX power. Otherwise, EVM will be bad.
		//
		// 2012/04/25 MH Add for tx power tracking to set tx power in tx agc for 88E.
		if (ThermalValue > pDM_Odm->RFCalibrateInfo.ThermalValue) {
			for (p = ODM_RF_PATH_A; p < MAX_PATH_NUM_8188F; p++) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
					("Temperature Increasing(%d): delta_pi: %d , delta_t: %d, Now_t: %d, EFUSE_t: %d, Last_t: %d\n", 
					p, pDM_Odm->RFCalibrateInfo.power_index_offset[p], delta, ThermalValue, pHalData->EEPROMThermalMeter, pDM_Odm->RFCalibrateInfo.ThermalValue));	
			}
		} else if (ThermalValue < pDM_Odm->RFCalibrateInfo.ThermalValue) { // Low temperature
			for (p = ODM_RF_PATH_A; p < MAX_PATH_NUM_8188F; p++) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
					("Temperature Decreasing(%d): delta_pi: %d , delta_t: %d, Now_t: %d, EFUSE_t: %d, Last_t: %d\n",
					p, pDM_Odm->RFCalibrateInfo.power_index_offset[p], delta, ThermalValue, pHalData->EEPROMThermalMeter, pDM_Odm->RFCalibrateInfo.ThermalValue));				
			}
		}

		if (ThermalValue > pHalData->EEPROMThermalMeter) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
				("Temperature(%d) higher than PG value(%d)\n", ThermalValue, pHalData->EEPROMThermalMeter));			

				for (p = ODM_RF_PATH_A; p < MAX_PATH_NUM_8188F; p++)
						rtl8188fu_dm_tx_power_track_set_power(pDM_Odm, MIX_MODE, p, 0);
		} else {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("**********Enter POWER Tracking MIX_MODE**********\n"));
				for (p = ODM_RF_PATH_A; p < MAX_PATH_NUM_8188F; p++)
					rtl8188fu_dm_tx_power_track_set_power(pDM_Odm, MIX_MODE, p, 0);
		}

		pRFCalibrateInfo->swing_idx_cck_base = pRFCalibrateInfo->swing_idx_cck;    /*Record last time Power Tracking result as base.*/
		for (p = ODM_RF_PATH_A; p < MAX_PATH_NUM_8188F; p++)
				pRFCalibrateInfo->swing_idx_ofdm_base[p] = pRFCalibrateInfo->swing_idx_ofdm[p];

	 	ODM_RT_TRACE(pDM_Odm,ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
					("pDM_Odm->RFCalibrateInfo.ThermalValue = %d ThermalValue= %d\n", pDM_Odm->RFCalibrateInfo.ThermalValue, ThermalValue));
		
		pDM_Odm->RFCalibrateInfo.ThermalValue = ThermalValue;         /*Record last Power Tracking Thermal Value*/

	}

	/* Delta temperature is equal to or larger than 20 centigrade (When threshold is 8).*/
	if (delta_IQK >= IQK_THRESHOLD) {
		if (!pDM_Odm->RFCalibrateInfo.bIQKInProgress) {
			pDM_Odm->RFCalibrateInfo.ThermalValue_IQK = ThermalValue;
			rtl8188fu_phy_iq_calibrate(Adapter, FALSE, FALSE);
		 }
	}

			
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("<===ODM_TXPowerTrackingCallback_ThermalMeter End\n"));
	
	pDM_Odm->RFCalibrateInfo.TXPowercount = 0;
}


u1Byte ODM_GetRightChnlPlaceforIQK(u1Byte chnl)
{
	u1Byte	channel_all[ODM_TARGET_CHNL_NUM_2G_5G] = 
	{1,2,3,4,5,6,7,8,9,10,11,12,13,14,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,100,102,104,106,108,110,112,114,116,118,120,122,124,126,128,130,132,134,136,138,140,149,151,153,155,157,159,161,163,165};
	u1Byte	place = chnl;

	
	if(chnl > 14)
	{
		for(place = 14; place<sizeof(channel_all); place++)
		{
			if(channel_all[place] == chnl)
			{
				return place-13;
			}
		}
	}	
	return 0;

}

VOID
odm_IQCalibrate(
		IN	PDM_ODM_T	pDM_Odm 
		)
{
	PADAPTER	Adapter = pDM_Odm->Adapter;

	
}

void phydm_rf_init(IN	PVOID		pDM_VOID)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	odm_TXPowerTrackingInit(pDM_Odm);

	ODM_ClearTxPowerTrackingState(pDM_Odm);	

}

