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
#include "../phydm_precomp.h"

#define		bMaskH3Bytes				0xffffff00

//#define SUCCESS 0
//#define FAIL -1


/*---------------------------Define Local Constant---------------------------*/
// 2010/04/25 MH Define the max tx power tracking tx agc power.
#define	ODM_TXPWRTRACK_MAX_IDX8188F		6

// MACRO definition for pRFCalibrateInfo->TxIQC_8188F[0]
#define 	PATH_S0                         1 // RF_PATH_B
#define     IDX_0xC94                       0
#define     IDX_0xC80                       1
#define     IDX_0xC4C                       2
#define     IDX_0xC14                       0
#define     IDX_0xCA0                       1
#define     KEY                             0
#define     VAL                             1

// MACRO definition for pRFCalibrateInfo->TxIQC_8188F[1]
#define 	PATH_S1                         0 // RF_PATH_A
#define     IDX_0xC9C                       0
#define     IDX_0xC88                       1
#define     IDX_0xC4C                       2
#define     IDX_0xC1C                       0
#define     IDX_0xC78                       1



/*---------------------------Define Local Constant---------------------------*/


//3============================================================
//3 Tx Power Tracking
//3============================================================


static void rtl8188fu_set_iqk_matrix(
	PDM_ODM_T pDM_Odm,
	s1Byte OFDM_index,
	u1Byte RFPath,
	s4Byte IqkResult_X,
	s4Byte IqkResult_Y
)
{
	s4Byte ele_A = 0, ele_D, ele_C = 0, value32;

	if (OFDM_index >= OFDM_TABLE_SIZE)
		OFDM_index = OFDM_TABLE_SIZE - 1;
	else if (OFDM_index < 0)
		OFDM_index = 0;

	ele_D = (OFDMSwingTable[OFDM_index] & 0xFFC00000) >> 22;

	//new element A = element D x X
	if ((IqkResult_X != 0)) {
		if ((IqkResult_X & 0x00000200) != 0)    //consider minus
			IqkResult_X = IqkResult_X | 0xFFFFFC00;
		ele_A = ((IqkResult_X * ele_D) >> 8) & 0x000003FF;

		//new element C = element D x Y
		if ((IqkResult_Y & 0x00000200) != 0)
			IqkResult_Y = IqkResult_Y | 0xFFFFFC00;
		ele_C = ((IqkResult_Y * ele_D) >> 8) & 0x000003FF;

		switch (RFPath) {
		case ODM_RF_PATH_A:
			//wirte new elements A, C, D to regC80 and regC94, element B is always 0
			value32 = (ele_D << 22) | ((ele_C & 0x3F) << 16) | ele_A;
			ODM_SetBBReg(pDM_Odm, rOFDM0_XATxIQImbalance, bMaskDWord, value32);

			value32 = (ele_C & 0x000003C0) >> 6;
			ODM_SetBBReg(pDM_Odm, rOFDM0_XCTxAFE, bMaskH4Bits, value32);

			value32 = ((IqkResult_X * ele_D) >> 7) & 0x01;
			ODM_SetBBReg(pDM_Odm, rOFDM0_ECCAThreshold, BIT24, value32);
			break;
		default:
			break;
		}
	} else {
		switch (RFPath) {
		case ODM_RF_PATH_A:
			ODM_SetBBReg(pDM_Odm, rOFDM0_XATxIQImbalance, bMaskDWord, OFDMSwingTable[OFDM_index]);
			ODM_SetBBReg(pDM_Odm, rOFDM0_XCTxAFE, bMaskH4Bits, 0x00);
			ODM_SetBBReg(pDM_Odm, rOFDM0_ECCAThreshold, BIT24, 0x00);
			break;

		case ODM_RF_PATH_B:
			ODM_SetBBReg(pDM_Odm, rOFDM0_XBTxIQImbalance, bMaskDWord, OFDMSwingTable[OFDM_index]);
			ODM_SetBBReg(pDM_Odm, rOFDM0_XDTxAFE, bMaskH4Bits, 0x00);
			ODM_SetBBReg(pDM_Odm, rOFDM0_ECCAThreshold, BIT28, 0x00);
			break;

		default:
			break;
		}
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("TxPwrTracking path B: X = 0x%x, Y = 0x%x ele_A = 0x%x ele_C = 0x%x ele_D = 0x%x 0xeb4 = 0x%x 0xebc = 0x%x\n",
				 (u4Byte)IqkResult_X, (u4Byte)IqkResult_Y, (u4Byte)ele_A, (u4Byte)ele_C, (u4Byte)ele_D, (u4Byte)IqkResult_X, (u4Byte)IqkResult_Y));
}

/*-----------------------------------------------------------------------------
 * Function:	odm_TxPwrTrackSetPwr88E()
 *
 * Overview:	88E change all channel tx power accordign to flag.
 *				OFDM & CCK are all different.
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	04/23/2012	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
VOID
ODM_TxPwrTrackSetPwr_8188F(
	IN PVOID pDM_VOID,
	PWRTRACK_METHOD Method,
	u1Byte RFPath,
	u1Byte ChannelMappedIndex
)
{
	PDM_ODM_T pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PADAPTER Adapter = pDM_Odm->Adapter;
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(Adapter);
	u1Byte PwrTrackingLimit_OFDM = 34; //+0dB
	u1Byte PwrTrackingLimit_CCK = CCK_TABLE_SIZE_88F-1;   //-2dB
	s1Byte Final_OFDM_Swing_Index = 0;
	s1Byte Final_CCK_Swing_Index = 0;
//	u1Byte	i = 0;
	PODM_RF_CAL_T pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);

#if 0
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(Adapter);
	PMGNT_INFO pMgntInfo = &(Adapter->MgntInfo);
	if (!pMgntInfo->ForcedDataRate) { //auto rate
		if (pDM_Odm->TxRate != 0xFF)
			TxRate = HwRateToMRate8812(pDM_Odm->TxRate);
	} else   //force rate
		TxRate = (u1Byte) pMgntInfo->ForcedDataRate;
#endif
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("===>ODM_TxPwrTrackSetPwr8188F\n"));

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("PwrTrackingLimit=%d\n", PwrTrackingLimit_OFDM));

	RT_TRACE(COMP_CMD, DBG_LOUD, ("Method=%d\n", Method));

	if (Method == TXAGC) {
		//u1Byte	rf = 0;
		PADAPTER Adapter = pDM_Odm->Adapter;

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("odm_TxPwrTrackSetPwr8188F CH=%d\n", *(pDM_Odm->pChannel)));

		pRFCalibrateInfo->remnant_ofdm_swing_idx[RFPath] = pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath];


	} else if (Method == BBSWING) {
		Final_OFDM_Swing_Index = pRFCalibrateInfo->default_ofdm_index + pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath];
		Final_CCK_Swing_Index = pRFCalibrateInfo->default_cck_index + pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath];

		// Adjust BB swing by OFDM IQ matrix
		if (Final_OFDM_Swing_Index >= PwrTrackingLimit_OFDM)
			Final_OFDM_Swing_Index = PwrTrackingLimit_OFDM;
		else if (Final_OFDM_Swing_Index <= 0)
			Final_OFDM_Swing_Index = 0;

		if (Final_CCK_Swing_Index >= CCK_TABLE_SIZE_88F)
			Final_CCK_Swing_Index = CCK_TABLE_SIZE_88F - 1;
		else if ((s1Byte)pRFCalibrateInfo->swing_idx_cck < 0)
			Final_CCK_Swing_Index = 0;

		if (RFPath == ODM_RF_PATH_A) {

			rtl8188fu_set_iqk_matrix(pDM_Odm, Final_OFDM_Swing_Index, ODM_RF_PATH_A,
				pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting[ChannelMappedIndex].Value[0][0],
				pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting[ChannelMappedIndex].Value[0][1]);

			ODM_Write1Byte(pDM_Odm, 0xa22, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][0]);
			ODM_Write1Byte(pDM_Odm, 0xa23, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][1]);
			ODM_Write1Byte(pDM_Odm, 0xa24, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][2]);
			ODM_Write1Byte(pDM_Odm, 0xa25, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][3]);
			ODM_Write1Byte(pDM_Odm, 0xa26, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][4]);
			ODM_Write1Byte(pDM_Odm, 0xa27, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][5]);
			ODM_Write1Byte(pDM_Odm, 0xa28, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][6]);
			ODM_Write1Byte(pDM_Odm, 0xa29, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][7]);
			ODM_Write1Byte(pDM_Odm, 0xa9a, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][8]);
			ODM_Write1Byte(pDM_Odm, 0xa9b, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][9]);
			ODM_Write1Byte(pDM_Odm, 0xa9c, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][10]);
			ODM_Write1Byte(pDM_Odm, 0xa9d, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][11]);
			ODM_Write1Byte(pDM_Odm, 0xaa0, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][12]);
			ODM_Write1Byte(pDM_Odm, 0xaa1, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][13]);
			ODM_Write1Byte(pDM_Odm, 0xaa2, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][14]);
			ODM_Write1Byte(pDM_Odm, 0xaa3, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][15]);
		}
	} else if (Method == MIX_MODE) 	{
		RT_TRACE(COMP_CMD, DBG_LOUD, ("Method is MIX_MODE ====> \n"));
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,("pRFCalibrateInfo->DefaultOfdmIndex=%d,  pRFCalibrateInfo->DefaultCCKIndex=%d, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]=%d, RF_Path = %d\n",
				pRFCalibrateInfo->default_ofdm_index, pRFCalibrateInfo->default_cck_index, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath],RFPath ));

		Final_OFDM_Swing_Index = pRFCalibrateInfo->default_ofdm_index + pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath];
		Final_CCK_Swing_Index = pRFCalibrateInfo->default_cck_index + pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath];
		if (RFPath == ODM_RF_PATH_A) {
			if (Final_OFDM_Swing_Index > PwrTrackingLimit_OFDM) {     //BBSwing higher then Limit
				pRFCalibrateInfo->remnant_ofdm_swing_idx[RFPath] = Final_OFDM_Swing_Index - PwrTrackingLimit_OFDM;

				rtl8188fu_set_iqk_matrix(pDM_Odm, PwrTrackingLimit_OFDM, RFPath,
								   pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting[ChannelMappedIndex].Value[0][0],
								   pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting[ChannelMappedIndex].Value[0][1]);

				pRFCalibrateInfo->modify_txagc_flag_path_a = TRUE;

				//Set TxAGC Page C{};

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
							 ("******Path_A Over BBSwing Limit , PwrTrackingLimit = %d , Remnant TxAGC Value = %d\n",
							  PwrTrackingLimit_OFDM, pRFCalibrateInfo->remnant_ofdm_swing_idx[RFPath]));
				} else if (Final_OFDM_Swing_Index < pRFCalibrateInfo->default_ofdm_index) {
					pRFCalibrateInfo->remnant_ofdm_swing_idx[RFPath] = Final_OFDM_Swing_Index - pRFCalibrateInfo->default_ofdm_index;
					rtl8188fu_set_iqk_matrix(pDM_Odm, pRFCalibrateInfo->default_ofdm_index, ODM_RF_PATH_A,
						 pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting[ChannelMappedIndex].Value[0][0],
						 pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting[ChannelMappedIndex].Value[0][1]);

					pRFCalibrateInfo->modify_txagc_flag_path_a = TRUE;
				/* Set TxAGC Page C{}; */

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
							 ("******Path_A Lower then BBSwing lower bound  28 , Remnant TxAGC Value = %d\n",
							  pRFCalibrateInfo->remnant_ofdm_swing_idx[RFPath]));
			} else {
				rtl8188fu_set_iqk_matrix(pDM_Odm, Final_OFDM_Swing_Index, ODM_RF_PATH_A,
								   pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting[ChannelMappedIndex].Value[0][0],
								   pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting[ChannelMappedIndex].Value[0][1]);

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
							 ("******Path_A Compensate with BBSwing , Final_OFDM_Swing_Index = %d\n", Final_OFDM_Swing_Index));

				if (pRFCalibrateInfo->modify_txagc_flag_path_a) {  //If TxAGC has changed, reset TxAGC again
					pRFCalibrateInfo->remnant_ofdm_swing_idx[RFPath] = 0;
					}
			}
			{
			//Set TxAGC Page C{};
				rtl8188fu_phy_set_txpower_index_by_rate_section(Adapter, ODM_RF_PATH_A, pHalData->CurrentChannel, OFDM );
				rtl8188fu_phy_set_txpower_index_by_rate_section(Adapter, ODM_RF_PATH_A, pHalData->CurrentChannel, HT_MCS0_MCS7 );
			}
			pRFCalibrateInfo->Modify_TxAGC_Value_OFDM=pRFCalibrateInfo->remnant_ofdm_swing_idx[ODM_RF_PATH_A] ;        //add by Mingzhi.Guo


			//MIX mode: CCK
			if(Final_CCK_Swing_Index > PwrTrackingLimit_CCK) {
				pRFCalibrateInfo->remnant_cck_idx = Final_CCK_Swing_Index - PwrTrackingLimit_CCK;

				ODM_RT_TRACE(pDM_Odm,ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,("******Path_A CCK Over Limit , PwrTrackingLimit_CCK = %d , pRFCalibrateInfo->Remnant_CCKSwingIdx  = %d \n", PwrTrackingLimit_CCK, pRFCalibrateInfo->remnant_cck_idx));
				ODM_Write1Byte(pDM_Odm, 0xa22, CCKSwingTable_Ch1_Ch14_88F[PwrTrackingLimit_CCK][0]);
				ODM_Write1Byte(pDM_Odm, 0xa23, CCKSwingTable_Ch1_Ch14_88F[PwrTrackingLimit_CCK][1]);
				ODM_Write1Byte(pDM_Odm, 0xa24, CCKSwingTable_Ch1_Ch14_88F[PwrTrackingLimit_CCK][2]);
				ODM_Write1Byte(pDM_Odm, 0xa25, CCKSwingTable_Ch1_Ch14_88F[PwrTrackingLimit_CCK][3]);
				ODM_Write1Byte(pDM_Odm, 0xa26, CCKSwingTable_Ch1_Ch14_88F[PwrTrackingLimit_CCK][4]);
				ODM_Write1Byte(pDM_Odm, 0xa27, CCKSwingTable_Ch1_Ch14_88F[PwrTrackingLimit_CCK][5]);
				ODM_Write1Byte(pDM_Odm, 0xa28, CCKSwingTable_Ch1_Ch14_88F[PwrTrackingLimit_CCK][6]);
				ODM_Write1Byte(pDM_Odm, 0xa29, CCKSwingTable_Ch1_Ch14_88F[PwrTrackingLimit_CCK][7]);
				ODM_Write1Byte(pDM_Odm, 0xa9a, CCKSwingTable_Ch1_Ch14_88F[PwrTrackingLimit_CCK][8]);
				ODM_Write1Byte(pDM_Odm, 0xa9b, CCKSwingTable_Ch1_Ch14_88F[PwrTrackingLimit_CCK][9]);
				ODM_Write1Byte(pDM_Odm, 0xa9c, CCKSwingTable_Ch1_Ch14_88F[PwrTrackingLimit_CCK][10]);
				ODM_Write1Byte(pDM_Odm, 0xa9d, CCKSwingTable_Ch1_Ch14_88F[PwrTrackingLimit_CCK][11]);
				ODM_Write1Byte(pDM_Odm, 0xaa0, CCKSwingTable_Ch1_Ch14_88F[PwrTrackingLimit_CCK][12]);
				ODM_Write1Byte(pDM_Odm, 0xaa1, CCKSwingTable_Ch1_Ch14_88F[PwrTrackingLimit_CCK][13]);
				ODM_Write1Byte(pDM_Odm, 0xaa2, CCKSwingTable_Ch1_Ch14_88F[PwrTrackingLimit_CCK][14]);
				ODM_Write1Byte(pDM_Odm, 0xaa3, CCKSwingTable_Ch1_Ch14_88F[PwrTrackingLimit_CCK][15]);

			} else if(Final_CCK_Swing_Index < 0) {   // Lowest CCK Index = 0
				pRFCalibrateInfo->remnant_cck_idx = Final_CCK_Swing_Index;

				ODM_RT_TRACE(pDM_Odm,ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,("******Path_A CCK Under Limit , PwrTrackingLimit_CCK = %d , pRFCalibrateInfo->Remnant_CCKSwingIdx  = %d \n", 0, pRFCalibrateInfo->remnant_cck_idx));
				ODM_Write1Byte(pDM_Odm, 0xa22, CCKSwingTable_Ch1_Ch14_88F[0][0]);
				ODM_Write1Byte(pDM_Odm, 0xa23, CCKSwingTable_Ch1_Ch14_88F[0][1]);
				ODM_Write1Byte(pDM_Odm, 0xa24, CCKSwingTable_Ch1_Ch14_88F[0][2]);
				ODM_Write1Byte(pDM_Odm, 0xa25, CCKSwingTable_Ch1_Ch14_88F[0][3]);
				ODM_Write1Byte(pDM_Odm, 0xa26, CCKSwingTable_Ch1_Ch14_88F[0][4]);
				ODM_Write1Byte(pDM_Odm, 0xa27, CCKSwingTable_Ch1_Ch14_88F[0][5]);
				ODM_Write1Byte(pDM_Odm, 0xa28, CCKSwingTable_Ch1_Ch14_88F[0][6]);
				ODM_Write1Byte(pDM_Odm, 0xa29, CCKSwingTable_Ch1_Ch14_88F[0][7]);
				ODM_Write1Byte(pDM_Odm, 0xa9a, CCKSwingTable_Ch1_Ch14_88F[0][8]);
				ODM_Write1Byte(pDM_Odm, 0xa9b, CCKSwingTable_Ch1_Ch14_88F[0][9]);
				ODM_Write1Byte(pDM_Odm, 0xa9c, CCKSwingTable_Ch1_Ch14_88F[0][10]);
				ODM_Write1Byte(pDM_Odm, 0xa9d, CCKSwingTable_Ch1_Ch14_88F[0][11]);
				ODM_Write1Byte(pDM_Odm, 0xaa0, CCKSwingTable_Ch1_Ch14_88F[0][12]);
				ODM_Write1Byte(pDM_Odm, 0xaa1, CCKSwingTable_Ch1_Ch14_88F[0][13]);
				ODM_Write1Byte(pDM_Odm, 0xaa2, CCKSwingTable_Ch1_Ch14_88F[0][14]);
				ODM_Write1Byte(pDM_Odm, 0xaa3, CCKSwingTable_Ch1_Ch14_88F[0][15]);

			} else {
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,("******Path_A CCK Compensate with BBSwing , Final_CCK_Swing_Index = %d \n", Final_CCK_Swing_Index));
				ODM_Write1Byte(pDM_Odm, 0xa22, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][0]);
				ODM_Write1Byte(pDM_Odm, 0xa23, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][1]);
				ODM_Write1Byte(pDM_Odm, 0xa24, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][2]);
				ODM_Write1Byte(pDM_Odm, 0xa25, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][3]);
				ODM_Write1Byte(pDM_Odm, 0xa26, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][4]);
				ODM_Write1Byte(pDM_Odm, 0xa27, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][5]);
				ODM_Write1Byte(pDM_Odm, 0xa28, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][6]);
				ODM_Write1Byte(pDM_Odm, 0xa29, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][7]);
				ODM_Write1Byte(pDM_Odm, 0xa9a, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][8]);
				ODM_Write1Byte(pDM_Odm, 0xa9b, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][9]);
				ODM_Write1Byte(pDM_Odm, 0xa9c, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][10]);
				ODM_Write1Byte(pDM_Odm, 0xa9d, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][11]);
				ODM_Write1Byte(pDM_Odm, 0xaa0, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][12]);
				ODM_Write1Byte(pDM_Odm, 0xaa1, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][13]);
				ODM_Write1Byte(pDM_Odm, 0xaa2, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][14]);
				ODM_Write1Byte(pDM_Odm, 0xaa3, CCKSwingTable_Ch1_Ch14_88F[Final_CCK_Swing_Index][15]);
				pRFCalibrateInfo->remnant_cck_idx = 0;
			}
			{
				//Set TxAGC Page C{};
				rtl8188fu_phy_set_txpower_index_by_rate_section(Adapter, ODM_RF_PATH_A, pHalData->CurrentChannel, CCK );
			}
			pRFCalibrateInfo->Modify_TxAGC_Value_CCK = pRFCalibrateInfo->remnant_cck_idx;

		}
	} else
		return;
} // odm_TxPwrTrackSetPwr8188F


VOID
rtl8188fu_get_delta_swing_table(
	IN PVOID pDM_VOID,
	OUT pu1Byte *TemperatureUP_A,
	OUT pu1Byte *TemperatureDOWN_A,
	OUT pu1Byte *TemperatureUP_B,
	OUT pu1Byte *TemperatureDOWN_B
)
{
	PDM_ODM_T			pDM_Odm	= (PDM_ODM_T)pDM_VOID;
	PADAPTER			Adapter		= pDM_Odm->Adapter;
	PHAL_DATA_TYPE	pHalData	= GET_HAL_DATA(Adapter);
	PODM_RF_CAL_T		pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);
	u1Byte				TxRate			= 0xFF;
	u1Byte				channel			= pHalData->CurrentChannel;

	{
		u2Byte	rate	 = *(pDM_Odm->pForcedDataRate);

		if (!rate) { /*auto rate*/
			if (rate != 0xFF) {
					TxRate = HwRateToMRate(pDM_Odm->TxRate);
			}
		} else { /*force rate*/
			TxRate = (u1Byte)rate;
		}
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Power Tracking TxRate=0x%X\n", TxRate));


	RT_TRACE(COMP_CMD, DBG_LOUD, ("GetDeltaSwingTable_8188F ====> channel is %d\n", channel));

	if ( 1 <= channel && channel <= 14) {
		if (IS_CCK_RATE(TxRate)) {
			*TemperatureUP_A = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKA_P;
			*TemperatureDOWN_A = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKA_N;
			*TemperatureUP_B = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKB_P;
			*TemperatureDOWN_B = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKB_N;
		} else {
			*TemperatureUP_A = pRFCalibrateInfo->DeltaSwingTableIdx_2GA_P;
			*TemperatureDOWN_A = pRFCalibrateInfo->DeltaSwingTableIdx_2GA_N;
			*TemperatureUP_B = pRFCalibrateInfo->DeltaSwingTableIdx_2GB_P;
			*TemperatureDOWN_B = pRFCalibrateInfo->DeltaSwingTableIdx_2GB_N;
		}
	} else {
		*TemperatureUP_A = (pu1Byte)DeltaSwingTableIdx_2GA_P_8188E;
		*TemperatureDOWN_A = (pu1Byte)DeltaSwingTableIdx_2GA_N_8188E;
		*TemperatureUP_B = (pu1Byte)DeltaSwingTableIdx_2GA_P_8188E;
		*TemperatureDOWN_B = (pu1Byte)DeltaSwingTableIdx_2GA_N_8188E;
	}

	return;
}


void ConfigureTxpowerTrack_8188F(
	PTXPWRTRACK_CFG pConfig
)
{
	RT_TRACE(COMP_CMD, DBG_LOUD, ("ConfigureTxpowerTrack_8188F ====> \n"));
}

//1 7.	IQK
#define MAX_TOLERANCE		5
#define IQK_DELAY_TIME		1		//ms

u1Byte          //bit0 = 1 => Tx OK, bit1 = 1 => Rx OK
phy_PathA_IQK_8188F(
	IN PADAPTER pAdapter,
	IN BOOLEAN configPathB
)
{
	u4Byte regEAC, regE94, regE9C/*, regEA4*/;
	u1Byte result = 0x00;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);
	PDM_ODM_T pDM_Odm = &pHalData->odmpriv;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path A IQK!\n"));

	//  enable path A PA in TXIQK mode
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x000000);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_WE_LUT, 0x80000, 0x1);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_RCK_OS, bRFRegOffsetMask, 0x20000);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_TXPA_G1, bRFRegOffsetMask, 0x0000f);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_TXPA_G2, bRFRegOffsetMask, 0x07ff7);   //0x07f77
	//PA,PAD gain adjust
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xdf, bRFRegOffsetMask, 0x980);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x56, bRFRegOffsetMask, 0x5102a); //0x5111e0


	//enter IQK mode
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x808000);


	//1 Tx IQK
	//path-A IQK setting
//	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path-A IQK setting!\n"));
	ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_A, bMaskDWord, 0x18008c1c);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_A, bMaskDWord, 0x38008c1c);
	ODM_SetBBReg(pDM_Odm, rTx_IQK_PI_A, bMaskDWord, 0x821403ff);    //0x821403e0
	ODM_SetBBReg(pDM_Odm, rRx_IQK_PI_A, bMaskDWord, 0x28160000);

	//LO calibration setting
//	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("LO calibration setting!\n"));
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Rsp, bMaskDWord, 0x00462911);

	//One shot, path A LOK & IQK
//	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("One shot, path A LOK & IQK!\n"));
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xf9000000);
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xf8000000);

	// delay x ms
//	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Delay %d ms for One shot, path A LOK & IQK.\n", IQK_DELAY_TIME_8188F));
//PlatformStallExecution(IQK_DELAY_TIME_8188F*1000);
	ODM_delay_ms(IQK_DELAY_TIME_8188F);

	//reload RF 0xdf
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x000000);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xdf, bRFRegOffsetMask, 0x180);

	//save LOK result
	pDM_Odm->RFCalibrateInfo.LOK_Result = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x8, bRFRegOffsetMask);

	// Check failed
	regEAC = ODM_GetBBReg(pDM_Odm, rRx_Power_After_IQK_A_2, bMaskDWord);
	regE94 = ODM_GetBBReg(pDM_Odm, rTx_Power_Before_IQK_A, bMaskDWord);
	regE9C = ODM_GetBBReg(pDM_Odm, rTx_Power_After_IQK_A, bMaskDWord);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xeac = 0x%x\n", regEAC));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xe94 = 0x%x, 0xe9c = 0x%x\n", regE94, regE9C));
	//monitor image power before & after IQK
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xe90(before IQK)= 0x%x, 0xe98(afer IQK) = 0x%x\n",
				 ODM_GetBBReg(pDM_Odm, 0xe90, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xe98, bMaskDWord)));

	if (!(regEAC & BIT28) &&
		(((regE94 & 0x03FF0000) >> 16) != 0x142) &&
		(((regE9C & 0x03FF0000) >> 16) != 0x42))
		result |= 0x01;

	return result;


}

u1Byte          //bit0 = 1 => Tx OK, bit1 = 1 => Rx OK
phy_PathA_RxIQK8188F(
	IN PADAPTER pAdapter,
	IN BOOLEAN configPathB
)
{
	u4Byte regEAC, regE94, regE9C, regEA4, u4tmp;
	u1Byte result = 0x00;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);
	PDM_ODM_T pDM_Odm = &pHalData->odmpriv;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path A Rx IQK!\n"));

	//1 Get TXIMR setting
	//modify RXIQK mode table
//	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path-A Rx IQK modify RXIQK mode table!\n"));
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x000000);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_WE_LUT, 0x80000, 0x1);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_RCK_OS, bRFRegOffsetMask, 0x30000);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_TXPA_G1, bRFRegOffsetMask, 0x0000f);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_TXPA_G2, bRFRegOffsetMask, 0xf1173);   //0xf117b

	//PA,PAD gain adjust
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xdf, bRFRegOffsetMask, 0x980);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x56, bRFRegOffsetMask, 0x5102a); //0x510f0

	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x808000);

	//IQK setting
	ODM_SetBBReg(pDM_Odm, rTx_IQK, bMaskDWord, 0x01007c00);
	ODM_SetBBReg(pDM_Odm, rRx_IQK, bMaskDWord, 0x01004800);

	//path-A IQK setting
	ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_A, bMaskDWord, 0x10008c1c);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_A, bMaskDWord, 0x30008c1c);
	ODM_SetBBReg(pDM_Odm, rTx_IQK_PI_A, bMaskDWord, 0x82160fff);    //0x821603e0
	ODM_SetBBReg(pDM_Odm, rRx_IQK_PI_A, bMaskDWord, 0x28160000);

	//LO calibration setting
//	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("LO calibration setting!\n"));
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Rsp, bMaskDWord, 0x00462911);

	//One shot, path A LOK & IQK
//	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("One shot, path A LOK & IQK!\n"));
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xf9000000);
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xf8000000);

	// delay x ms
//	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Delay %d ms for One shot, path A LOK & IQK.\n", IQK_DELAY_TIME_8188F));
//PlatformStallExecution(IQK_DELAY_TIME_8188F*1000);
	ODM_delay_ms(IQK_DELAY_TIME_8188F);


	//reload RF 0xdf
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x000000);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xdf, bRFRegOffsetMask, 0x180);



	// Check failed
	regEAC = ODM_GetBBReg(pDM_Odm, rRx_Power_After_IQK_A_2, bMaskDWord);
	regE94 = ODM_GetBBReg(pDM_Odm, rTx_Power_Before_IQK_A, bMaskDWord);
	regE9C = ODM_GetBBReg(pDM_Odm, rTx_Power_After_IQK_A, bMaskDWord);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xeac = 0x%x\n", regEAC));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xe94 = 0x%x, 0xe9c = 0x%x\n", regE94, regE9C));
	//monitor image power before & after IQK
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xe90(before IQK)= 0x%x, 0xe98(afer IQK) = 0x%x\n",
				 ODM_GetBBReg(pDM_Odm, 0xe90, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xe98, bMaskDWord)));

	if (!(regEAC & BIT28) &&
		(((regE94 & 0x03FF0000) >> 16) != 0x142) &&
		(((regE9C & 0x03FF0000) >> 16) != 0x42))
		result |= 0x01;
	else                            //if Tx not OK, ignore Rx
		return result;

	u4tmp = 0x80007C00 | (regE94 & 0x3FF0000) | ((regE9C & 0x3FF0000) >> 16);
	ODM_SetBBReg(pDM_Odm, rTx_IQK, bMaskDWord, u4tmp);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xe40 = 0x%x u4tmp = 0x%x\n", ODM_GetBBReg(pDM_Odm, rTx_IQK, bMaskDWord), u4tmp));


	//1 RX IQK
	//modify RXIQK mode table
//	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path-A Rx IQK modify RXIQK mode table 2!\n"));
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x000000);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_WE_LUT, 0x80000, 0x1);                 // 0xEF[19]   = 0x1
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_RCK_OS, bRFRegOffsetMask, 0x30000);  // 0x30[19:0] = 0x18000
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_TXPA_G1, bRFRegOffsetMask, 0x0000f); // 0x31[19:0] = 0x0000f
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_TXPA_G2, bRFRegOffsetMask, 0xf7ff2); // 0x32[19:0] = 0xf7ffa

	//PA,PAD gain adjust
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xdf, bRFRegOffsetMask, 0x980);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x56, bRFRegOffsetMask, 0x51000); //0x51000

	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x808000);

	//IQK setting
	ODM_SetBBReg(pDM_Odm, rRx_IQK, bMaskDWord, 0x01004800);

	//path-A IQK setting
	ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_A, bMaskDWord, 0x30008c1c);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_A, bMaskDWord, 0x10008c1c);
	ODM_SetBBReg(pDM_Odm, rTx_IQK_PI_A, bMaskDWord, 0x82160000);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_PI_A, bMaskDWord, 0x281613ff);    //0x281603e0


	//LO calibration setting
//	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("LO calibration setting!\n"));
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Rsp, bMaskDWord, 0x0046a911);

	//One shot, path A LOK & IQK
//	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("One shot, path A LOK & IQK!\n"));
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xf9000000);
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xf8000000);

	// delay x ms
//	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Delay %d ms for One shot, path A LOK & IQK.\n", IQK_DELAY_TIME_8188F));
//PlatformStallExecution(IQK_DELAY_TIME_8188F*1000);
	ODM_delay_ms(IQK_DELAY_TIME_8188F);

	//reload RF 0xdf
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x000000);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xdf, bRFRegOffsetMask, 0x180);

	//reload LOK value
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x8, bRFRegOffsetMask, pDM_Odm->RFCalibrateInfo.LOK_Result);

	// Check failed
	regEAC = ODM_GetBBReg(pDM_Odm, rRx_Power_After_IQK_A_2, bMaskDWord);
	regEA4 = ODM_GetBBReg(pDM_Odm, rRx_Power_Before_IQK_A_2, bMaskDWord);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xeac = 0x%x\n", regEAC));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xea4 = 0x%x, 0xeac = 0x%x\n", regEA4, regEAC));
	//monitor image power before & after IQK
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xea0(before IQK)= 0x%x, 0xea8(afer IQK) = 0x%x\n",
				 ODM_GetBBReg(pDM_Odm, 0xea0, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xea8, bMaskDWord)));


	if (!(regEAC & BIT27) &&     //if Tx is OK, check whether Rx is OK
		(((regEA4 & 0x03FF0000) >> 16) != 0x132) &&
		(((regEAC & 0x03FF0000) >> 16) != 0x36))
		result |= 0x02;
	else
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path A Rx IQK fail!!\n"));

	return result;


}

u1Byte              //bit0 = 1 => Tx OK, bit1 = 1 => Rx OK
phy_PathB_IQK_8188F(
	IN PADAPTER pAdapter
)
{
	u4Byte regEAC, regE94, regE9C/*, regEC4, regECC*/;
	u1Byte result = 0x00;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);
	PDM_ODM_T pDM_Odm = &pHalData->odmpriv;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path B IQK!\n"));

	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x000000);
	//switch to path B
	ODM_SetBBReg(pDM_Odm, 0x948, bMaskDWord, 0x00000080);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xb0, bRFRegOffsetMask, 0xefff0);
	//  in TXIQK mode
//	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_WE_LUT, bRFRegOffsetMask, 0x800a0 );
//	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_RCK_OS, bRFRegOffsetMask, 0x20000 );
//	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_TXPA_G1, bRFRegOffsetMask, 0x0003f );
//	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_TXPA_G2, bRFRegOffsetMask, 0xc7f87 );
//  enable path B PA in TXIQK mode
//	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xed, bRFRegOffsetMask, 0x00020 );
//	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x43, bRFRegOffsetMask, 0x40fc1 );


	//1 Tx IQK
	//path-A IQK setting
//	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path-B IQK setting!\n"));
	ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_A, bMaskDWord, 0x18008c1c);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_A, bMaskDWord, 0x38008c1c);
	ODM_SetBBReg(pDM_Odm, rTx_IQK_PI_A, bMaskDWord, 0x82140102);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_PI_A, bMaskDWord, 0x28160000);

	//LO calibration setting
//	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("LO calibration setting!\n"));
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Rsp, bMaskDWord, 0x00462911);


	//enter IQK mode
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x808000);

	//One shot, path B LOK & IQK
//	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("One shot, path B LOK & IQK!\n"));
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xf9000000);
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xf8000000);

	// delay x ms
//	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Delay %d ms for One shot, path B LOK & IQK.\n", IQK_DELAY_TIME_8188F));
//PlatformStallExecution(IQK_DELAY_TIME_8188F*1000);
	ODM_delay_ms(IQK_DELAY_TIME_8188F);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0x948 = 0x%x\n", ODM_GetBBReg(pDM_Odm, 0x948, bMaskDWord)));


	// Check failed
	regEAC = ODM_GetBBReg(pDM_Odm, rRx_Power_After_IQK_A_2, bMaskDWord);
	regE94 = ODM_GetBBReg(pDM_Odm, rTx_Power_Before_IQK_A, bMaskDWord);
	regE9C = ODM_GetBBReg(pDM_Odm, rTx_Power_After_IQK_A, bMaskDWord);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xeac = 0x%x\n", regEAC));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xe94 = 0x%x, 0xe9c = 0x%x\n", regE94, regE9C));
	//monitor image power before & after IQK
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xe90(before IQK)= 0x%x, 0xe98(afer IQK) = 0x%x\n",
				 ODM_GetBBReg(pDM_Odm, 0xe90, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xe98, bMaskDWord)));


	if (!(regEAC & BIT28) &&
		(((regE94 & 0x03FF0000) >> 16) != 0x142) &&
		(((regE9C & 0x03FF0000) >> 16) != 0x42))
		result |= 0x01;
	else
		return result;
#if 0
	if (!(regEAC & BIT30) &&
		(((regEC4 & 0x03FF0000) >> 16) != 0x132) &&
		(((regECC & 0x03FF0000) >> 16) != 0x36))
		result |= 0x02;
	else
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path B Rx IQK fail!!\n"));

#endif
	return result;
}



u1Byte          //bit0 = 1 => Tx OK, bit1 = 1 => Rx OK
phy_PathB_RxIQK8188F(
	IN PADAPTER pAdapter,
	IN BOOLEAN configPathB
)
{
	u4Byte regEAC, regEB4, regEBC, regECC, regEC4, u4tmp;
	u1Byte result = 0x00;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);
	PDM_ODM_T pDM_Odm = &pHalData->odmpriv;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path B Rx IQK!\n"));

	//1 Get TXIMR setting
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Get RXIQK TXIMR!\n"));
	//modify RXIQK mode table
//	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path-A Rx IQK modify RXIQK mode table!\n"));
//	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x000000);
//	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_WE_LUT, bRFRegOffsetMask, 0x800a0 );
//	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_RCK_OS, bRFRegOffsetMask, 0x30000 );
//	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_TXPA_G1, bRFRegOffsetMask, 0x0000f );
//	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_TXPA_G2, bRFRegOffsetMask, 0xf117B );
//	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x808000);

	//IQK setting
	ODM_SetBBReg(pDM_Odm, rTx_IQK, bMaskDWord, 0x01007c00);
	ODM_SetBBReg(pDM_Odm, rRx_IQK, bMaskDWord, 0x81004800);

	//path-B IQK setting
	ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_B, bMaskDWord, 0x10008c1c);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_B, bMaskDWord, 0x30008c1c);
	ODM_SetBBReg(pDM_Odm, rTx_IQK_PI_B, bMaskDWord, 0x82130804);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_PI_B, bMaskDWord, 0x68130000);

	//LO calibration setting
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("LO calibration setting!\n"));
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Rsp, bMaskDWord, 0x0046a911);

	//One shot, path B LOK & IQK
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("One shot, path B LOK & IQK!\n"));
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Cont, bMaskDWord, 0x00000002);
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Cont, bMaskDWord, 0x00000000);

	// delay x ms
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Delay %d ms for One shot, path A LOK & IQK.\n", IQK_DELAY_TIME_8188F));
	//PlatformStallExecution(IQK_DELAY_TIME_8188F*1000);
	ODM_delay_ms(IQK_DELAY_TIME_8188F);


	// Check failed
	regEAC = ODM_GetBBReg(pDM_Odm, rRx_Power_After_IQK_A_2, bMaskDWord);
	regEB4 = ODM_GetBBReg(pDM_Odm, rTx_Power_Before_IQK_B, bMaskDWord);
	regEBC = ODM_GetBBReg(pDM_Odm, rTx_Power_After_IQK_B, bMaskDWord);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xeac = 0x%x\n", regEAC));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xeb4 = 0x%x, 0xebc = 0x%x\n", regEB4, regEBC));
	//monitor image power before & after IQK
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xeb0(before IQK)= 0x%x, 0xeb8(afer IQK) = 0x%x\n",
				 ODM_GetBBReg(pDM_Odm, 0xeb0, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xeb8, bMaskDWord)));


	if (!(regEAC & BIT31) &&
		(((regEB4 & 0x03FF0000) >> 16) != 0x142) &&
		(((regEBC & 0x03FF0000) >> 16) != 0x42))
		result |= 0x01;
	else                            //if Tx not OK, ignore Rx
		return result;

	u4tmp = 0x80007C00 | (regEB4 & 0x3FF0000) | ((regEBC & 0x3FF0000) >> 16);
	ODM_SetBBReg(pDM_Odm, rTx_IQK, bMaskDWord, u4tmp);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xe40 = 0x%x u4tmp = 0x%x\n", ODM_GetBBReg(pDM_Odm, rTx_IQK, bMaskDWord), u4tmp));


	//1 RX IQK
	//modify RXIQK mode table
//	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path-A Rx IQK modify RXIQK mode table 2!\n"));
//	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x000000);
//	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_WE_LUT, bRFRegOffsetMask, 0x800a0 );

	//<20121009, Kordan> RF Mode = 3
//	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_WE_LUT, 0x80000, 0x1);	               // 0xEF[19]   = 0x1
//	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_RCK_OS, bRFRegOffsetMask, 0x18000 );  // 0x30[19:0] = 0x18000
//	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_TXPA_G1, bRFRegOffsetMask, 0x0000f ); // 0x31[19:0] = 0x0000f
//	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_TXPA_G2, bRFRegOffsetMask, 0xf7ffa ); // 0x32[19:0] = 0xf7ffa
//	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_WE_LUT, 0x80000, 0x0);	               // 0xEF[19]   = 0x0
//	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x808000);

	//IQK setting
	ODM_SetBBReg(pDM_Odm, rRx_IQK, bMaskDWord, 0x01004800);

	//path-B IQK setting
	ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_B, bMaskDWord, 0x30008c1c);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_B, bMaskDWord, 0x10008c1c);
	ODM_SetBBReg(pDM_Odm, rTx_IQK_PI_B, bMaskDWord, 0x82130c05);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_PI_B, bMaskDWord, 0x68130c05);

	//LO calibration setting
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("LO calibration setting!\n"));
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Rsp, bMaskDWord, 0x0046a911);

	//One shot, path B LOK & IQK
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("One shot, path B LOK & IQK!\n"));
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Cont, bMaskDWord, 0x00000002);
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Cont, bMaskDWord, 0x00000000);

	// delay x ms
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Delay %d ms for One shot, path A LOK & IQK.\n", IQK_DELAY_TIME_8188F));
	//PlatformStallExecution(IQK_DELAY_TIME_8188F*1000);
	ODM_delay_ms(IQK_DELAY_TIME_8188F);

	// Check failed
	regEAC = ODM_GetBBReg(pDM_Odm, rRx_Power_After_IQK_A_2, bMaskDWord);
	regEC4 = ODM_GetBBReg(pDM_Odm, rRx_Power_Before_IQK_B_2, bMaskDWord);;
	regECC = ODM_GetBBReg(pDM_Odm, rRx_Power_After_IQK_B_2, bMaskDWord);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xeac = 0x%x\n", regEAC));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xec4 = 0x%x, 0xecc = 0x%x\n", regEC4, regECC));
	//monitor image power before & after IQK
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xec0(before IQK)= 0x%x, 0xec8(afer IQK) = 0x%x\n",
				 ODM_GetBBReg(pDM_Odm, 0xec0, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xec8, bMaskDWord)));

	//	PA/PAD controlled by 0x0
	//leave IQK mode
//	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x000000);
//	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xdf, bRFRegOffsetMask, 0x180 );



#if 0
	if (!(regEAC & BIT31) &&
		(((regEB4 & 0x03FF0000) >> 16) != 0x142) &&
		(((regEBC & 0x03FF0000) >> 16) != 0x42))
		result |= 0x01;
	else                            //if Tx not OK, ignore Rx
		return result;
#endif

	if (!(regEAC & BIT30) &&     //if Tx is OK, check whether Rx is OK
		(((regEC4 & 0x03FF0000) >> 16) != 0x132) &&
		(((regECC & 0x03FF0000) >> 16) != 0x36))
		result |= 0x02;
	else
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path B Rx IQK fail!!\n"));

	return result;


}


VOID
_PHY_PathAFillIQKMatrix8188F(
	IN PADAPTER pAdapter,
	IN BOOLEAN bIQKOK,
	IN s4Byte result[][8],
	IN u1Byte final_candidate,
	IN BOOLEAN bTxOnly
)
{
	u4Byte Oldval_0, X, TX0_A, reg;
	s4Byte Y, TX0_C;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);
	PDM_ODM_T pDM_Odm = &pHalData->odmpriv;
	PODM_RF_CAL_T pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path A IQ Calibration %s !\n", (bIQKOK) ? "Success" : "Failed"));

	if (final_candidate == 0xFF)
		return;

	else if (bIQKOK) {
		Oldval_0 = (ODM_GetBBReg(pDM_Odm, rOFDM0_XATxIQImbalance, bMaskDWord) >> 22) & 0x3FF;

		X = result[final_candidate][0];
		if ((X & 0x00000200) != 0)
			X = X | 0xFFFFFC00;
		TX0_A = (X * Oldval_0) >> 8;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("X = 0x%x, TX0_A = 0x%x, Oldval_0 0x%x\n", X, TX0_A, Oldval_0));
		ODM_SetBBReg(pDM_Odm, rOFDM0_XATxIQImbalance, 0x3FF, TX0_A);

		ODM_SetBBReg(pDM_Odm, rOFDM0_ECCAThreshold, BIT(31), ((X * Oldval_0 >> 7) & 0x1));

		Y = result[final_candidate][1];
		if ((Y & 0x00000200) != 0)
			Y = Y | 0xFFFFFC00;

		//2 Tx IQC
		TX0_C = (Y * Oldval_0) >> 8;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Y = 0x%x, TX = 0x%x\n", Y, TX0_C));
		ODM_SetBBReg(pDM_Odm, rOFDM0_XCTxAFE, 0xF0000000, ((TX0_C & 0x3C0) >> 6));
		pRFCalibrateInfo->TxIQC_8723B[PATH_S1][IDX_0xC94][KEY] = rOFDM0_XCTxAFE;
		pRFCalibrateInfo->TxIQC_8723B[PATH_S1][IDX_0xC94][VAL] = ODM_GetBBReg(pDM_Odm, rOFDM0_XCTxAFE, bMaskDWord);

		ODM_SetBBReg(pDM_Odm, rOFDM0_XATxIQImbalance, 0x003F0000, (TX0_C & 0x3F));
		pRFCalibrateInfo->TxIQC_8723B[PATH_S1][IDX_0xC80][KEY] = rOFDM0_XATxIQImbalance;
		pRFCalibrateInfo->TxIQC_8723B[PATH_S1][IDX_0xC80][VAL] = ODM_GetBBReg(pDM_Odm, rOFDM0_XATxIQImbalance, bMaskDWord);

		ODM_SetBBReg(pDM_Odm, rOFDM0_ECCAThreshold, BIT(29), ((Y * Oldval_0 >> 7) & 0x1));
		pRFCalibrateInfo->TxIQC_8723B[PATH_S1][IDX_0xC4C][KEY] = rOFDM0_ECCAThreshold;
		pRFCalibrateInfo->TxIQC_8723B[PATH_S1][IDX_0xC4C][VAL] = ODM_GetBBReg(pDM_Odm, rOFDM0_ECCAThreshold, bMaskDWord);

		if (bTxOnly) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("_PHY_PathAFillIQKMatrix8188F only Tx OK\n"));

			// <20130226, Kordan> Saving RxIQC, otherwise not initialized.
			pRFCalibrateInfo->RxIQC_8723B[PATH_S1][IDX_0xCA0][KEY] = rOFDM0_RxIQExtAnta;
			pRFCalibrateInfo->RxIQC_8723B[PATH_S1][IDX_0xCA0][VAL] = ODM_GetBBReg(pDM_Odm, rOFDM0_RxIQExtAnta, bMaskDWord);
			pRFCalibrateInfo->RxIQC_8723B[PATH_S1][IDX_0xC14][KEY] = rOFDM0_XARxIQImbalance;
			pRFCalibrateInfo->RxIQC_8723B[PATH_S1][IDX_0xC14][VAL] = ODM_GetBBReg(pDM_Odm, rOFDM0_XARxIQImbalance, bMaskDWord);
			return;
		}

		reg = result[final_candidate][2];

		//2 Rx IQC
		ODM_SetBBReg(pDM_Odm, rOFDM0_XARxIQImbalance, 0x3FF, reg);
		reg = result[final_candidate][3] & 0x3F;
		ODM_SetBBReg(pDM_Odm, rOFDM0_XARxIQImbalance, 0xFC00, reg);
		pRFCalibrateInfo->RxIQC_8723B[PATH_S1][IDX_0xC14][KEY] = rOFDM0_XARxIQImbalance;
		pRFCalibrateInfo->RxIQC_8723B[PATH_S1][IDX_0xC14][VAL] = ODM_GetBBReg(pDM_Odm, rOFDM0_XARxIQImbalance, bMaskDWord);

		reg = (result[final_candidate][3] >> 6) & 0xF;
		ODM_SetBBReg(pDM_Odm, rOFDM0_RxIQExtAnta, 0xF0000000, reg);
		pRFCalibrateInfo->RxIQC_8723B[PATH_S1][IDX_0xCA0][KEY] = rOFDM0_RxIQExtAnta;
		pRFCalibrateInfo->RxIQC_8723B[PATH_S1][IDX_0xCA0][VAL] = ODM_GetBBReg(pDM_Odm, rOFDM0_RxIQExtAnta, bMaskDWord);

	}
}

VOID
_PHY_PathBFillIQKMatrix8188F(
	IN PADAPTER pAdapter,
	IN BOOLEAN bIQKOK,
	IN s4Byte result[][8],
	IN u1Byte final_candidate,
	IN BOOLEAN bTxOnly         //do Tx only
)
{
	u4Byte Oldval_1, X, TX1_A, reg;
	s4Byte Y, TX1_C;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);
	PDM_ODM_T pDM_Odm = &pHalData->odmpriv;
	PODM_RF_CAL_T pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path B IQ Calibration %s !\n", (bIQKOK) ? "Success" : "Failed"));

	if (final_candidate == 0xFF)
		return;

	else if (bIQKOK) {
		Oldval_1 = (ODM_GetBBReg(pDM_Odm, rOFDM0_XBTxIQImbalance, bMaskDWord) >> 22) & 0x3FF;

		X = result[final_candidate][4];
		if ((X & 0x00000200) != 0)
			X = X | 0xFFFFFC00;
		TX1_A = (X * Oldval_1) >> 8;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("X = 0x%x, TX1_A = 0x%x\n", X, TX1_A));

		ODM_SetBBReg(pDM_Odm, rOFDM0_XBTxIQImbalance, 0x3FF, TX1_A);

		ODM_SetBBReg(pDM_Odm, rOFDM0_ECCAThreshold, BIT(27), ((X * Oldval_1 >> 7) & 0x1));

		Y = result[final_candidate][5];
		if ((Y & 0x00000200) != 0)
			Y = Y | 0xFFFFFC00;

		TX1_C = (Y * Oldval_1) >> 8;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Y = 0x%x, TX1_C = 0x%x\n", Y, TX1_C));

		//2 Tx IQC
		ODM_SetBBReg(pDM_Odm, rOFDM0_XDTxAFE, 0xF0000000, ((TX1_C & 0x3C0) >> 6));
		pRFCalibrateInfo->TxIQC_8723B[PATH_S0][IDX_0xC9C][KEY] = rOFDM0_XDTxAFE;
		pRFCalibrateInfo->TxIQC_8723B[PATH_S0][IDX_0xC9C][VAL] = ODM_GetBBReg(pDM_Odm, rOFDM0_XDTxAFE, bMaskDWord);

		ODM_SetBBReg(pDM_Odm, rOFDM0_XBTxIQImbalance, 0x003F0000, (TX1_C & 0x3F));
		pRFCalibrateInfo->TxIQC_8723B[PATH_S0][IDX_0xC88][KEY] = rOFDM0_XBTxIQImbalance;
		pRFCalibrateInfo->TxIQC_8723B[PATH_S0][IDX_0xC88][VAL] = ODM_GetBBReg(pDM_Odm, rOFDM0_XBTxIQImbalance, bMaskDWord);

		ODM_SetBBReg(pDM_Odm, rOFDM0_ECCAThreshold, BIT(25), ((Y * Oldval_1 >> 7) & 0x1));
		pRFCalibrateInfo->TxIQC_8723B[PATH_S0][IDX_0xC4C][KEY] = rOFDM0_ECCAThreshold;
		pRFCalibrateInfo->TxIQC_8723B[PATH_S0][IDX_0xC4C][VAL] = ODM_GetBBReg(pDM_Odm, rOFDM0_ECCAThreshold, bMaskDWord);

		if (bTxOnly) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("_PHY_PathBFillIQKMatrix8188F only Tx OK\n"));

			pRFCalibrateInfo->RxIQC_8723B[PATH_S0][IDX_0xC1C][KEY] = rOFDM0_XBRxIQImbalance;
			pRFCalibrateInfo->RxIQC_8723B[PATH_S0][IDX_0xC1C][VAL] = ODM_GetBBReg(pDM_Odm, rOFDM0_XBRxIQImbalance, bMaskDWord);
			pRFCalibrateInfo->RxIQC_8723B[PATH_S0][IDX_0xC78][KEY] = rOFDM0_AGCRSSITable;
			pRFCalibrateInfo->RxIQC_8723B[PATH_S0][IDX_0xC78][VAL] = ODM_GetBBReg(pDM_Odm, rOFDM0_AGCRSSITable, bMaskDWord);
			return;
		}

		//2 Rx IQC
		reg = result[final_candidate][6];
		ODM_SetBBReg(pDM_Odm, rOFDM0_XBRxIQImbalance, 0x3FF, reg);
		reg = result[final_candidate][7] & 0x3F;
		ODM_SetBBReg(pDM_Odm, rOFDM0_XBRxIQImbalance, 0xFC00, reg);
		pRFCalibrateInfo->RxIQC_8723B[PATH_S0][IDX_0xC1C][KEY] = rOFDM0_XBRxIQImbalance;
		pRFCalibrateInfo->RxIQC_8723B[PATH_S0][IDX_0xC1C][VAL] = ODM_GetBBReg(pDM_Odm, rOFDM0_XBRxIQImbalance, bMaskDWord);

		reg = (result[final_candidate][7] >> 6) & 0xF;
		ODM_SetBBReg(pDM_Odm, rOFDM0_AGCRSSITable, 0x0000F000, reg);
		pRFCalibrateInfo->RxIQC_8723B[PATH_S0][IDX_0xC78][KEY] = rOFDM0_AGCRSSITable;
		pRFCalibrateInfo->RxIQC_8723B[PATH_S0][IDX_0xC78][VAL] = ODM_GetBBReg(pDM_Odm, rOFDM0_AGCRSSITable, bMaskDWord);
	}
}

//
// 2011/07/26 MH Add an API for testing IQK fail case.
//
// MP Already declare in odm.c
BOOLEAN
ODM_CheckPowerStatus(
	IN PADAPTER Adapter)
{
	/*
		HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
		PDM_ODM_T			pDM_Odm = &pHalData->DM_OutSrc;
		RT_RF_POWER_STATE 	rtState;
		PMGNT_INFO			pMgntInfo	= &(Adapter->MgntInfo);

		// 2011/07/27 MH We are not testing ready~~!! We may fail to get correct value when init sequence.
		if (pMgntInfo->init_adpt_in_progress == TRUE)
		{
			ODM_RT_TRACE(pDM_Odm,COMP_INIT, DBG_LOUD, ("ODM_CheckPowerStatus Return TRUE, due to initadapter"));
			return	TRUE;
		}

		//
		//	2011/07/19 MH We can not execute tx pwoer tracking/ LLC calibrate or IQK.
		//
		Adapter->HalFunc.GetHwRegHandler(Adapter, HW_VAR_RF_STATE, (pu1Byte)(&rtState));
		if(Adapter->bDriverStopped || Adapter->bDriverIsGoingToPnpSetPowerSleep || rtState == eRfOff)
		{
			ODM_RT_TRACE(pDM_Odm,COMP_INIT, DBG_LOUD, ("ODM_CheckPowerStatus Return FALSE, due to %d/%d/%d\n",
			Adapter->bDriverStopped, Adapter->bDriverIsGoingToPnpSetPowerSleep, rtState));
			return	FALSE;
		}
	*/
	return TRUE;
}

VOID
_PHY_SaveADDARegisters8188F(
	IN PADAPTER pAdapter,
	IN pu4Byte ADDAReg,
	IN pu4Byte ADDABackup,
	IN u4Byte RegisterNum
)
{
	u4Byte i;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);
	PDM_ODM_T pDM_Odm = &pHalData->odmpriv;

	if (ODM_CheckPowerStatus(pAdapter) == FALSE)
		return;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Save ADDA parameters.\n"));
	for (i = 0; i < RegisterNum; i++)
		ADDABackup[i] = ODM_GetBBReg(pDM_Odm, ADDAReg[i], bMaskDWord);
}


VOID
_PHY_SaveMACRegisters8188F(
	IN PADAPTER pAdapter,
	IN pu4Byte MACReg,
	IN pu4Byte MACBackup
)
{
	u4Byte i;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);
	PDM_ODM_T pDM_Odm = &pHalData->odmpriv;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Save MAC parameters.\n"));
	for (i = 0; i < (IQK_MAC_REG_NUM - 1); i++)
		MACBackup[i] = ODM_Read1Byte(pDM_Odm, MACReg[i]);
	MACBackup[i] = ODM_Read4Byte(pDM_Odm, MACReg[i]);

}


VOID
_PHY_ReloadADDARegisters8188F(
	IN PADAPTER pAdapter,
	IN pu4Byte ADDAReg,
	IN pu4Byte ADDABackup,
	IN u4Byte RegiesterNum
)
{
	u4Byte i;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);
	PDM_ODM_T pDM_Odm = &pHalData->odmpriv;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Reload ADDA power saving parameters !\n"));
	for (i = 0; i < RegiesterNum; i++)
		ODM_SetBBReg(pDM_Odm, ADDAReg[i], bMaskDWord, ADDABackup[i]);
}

VOID
_PHY_ReloadMACRegisters8188F(
	IN PADAPTER pAdapter,
	IN pu4Byte MACReg,
	IN pu4Byte MACBackup
)
{
	u4Byte i;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);
	PDM_ODM_T pDM_Odm = &pHalData->odmpriv;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Reload MAC parameters !\n"));
	for (i = 0; i < (IQK_MAC_REG_NUM - 1); i++)
		ODM_Write1Byte(pDM_Odm, MACReg[i], (u1Byte)MACBackup[i]);
	ODM_Write4Byte(pDM_Odm, MACReg[i], MACBackup[i]);
}


VOID
_PHY_PathADDAOn8188F(
	IN PADAPTER pAdapter,
	IN pu4Byte ADDAReg,
	IN BOOLEAN isPathAOn,
	IN BOOLEAN is2T
)
{
	u4Byte pathOn;
	u4Byte i;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);
	PDM_ODM_T pDM_Odm = &pHalData->odmpriv;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("ADDA ON.\n"));

	pathOn = isPathAOn ? 0x03c00014 : 0x03c00014;
	if (FALSE == is2T) {
		pathOn = 0x03c00014;
		ODM_SetBBReg(pDM_Odm, ADDAReg[0], bMaskDWord, 0x03c00014);
	} else
		ODM_SetBBReg(pDM_Odm, ADDAReg[0], bMaskDWord, pathOn);

	for (i = 1; i < IQK_ADDA_REG_NUM; i++)
		ODM_SetBBReg(pDM_Odm, ADDAReg[i], bMaskDWord, pathOn);

}

VOID
_PHY_MACSettingCalibration8188F(
	IN PADAPTER pAdapter,
	IN pu4Byte MACReg,
	IN pu4Byte MACBackup
)
{
	u4Byte i = 0;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);
	PDM_ODM_T pDM_Odm = &pHalData->odmpriv;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("MAC settings for Calibration.\n"));

#if 0
	ODM_Write1Byte(pDM_Odm, MACReg[i], 0x3F);

	for (i = 1; i < (IQK_MAC_REG_NUM - 1); i++)
		ODM_Write1Byte(pDM_Odm, MACReg[i], (u1Byte)(MACBackup[i] & (~BIT3)));
	ODM_Write1Byte(pDM_Odm, MACReg[i], (u1Byte)(MACBackup[i] & (~BIT5)));
#else

	ODM_SetBBReg(pDM_Odm, 0x520, 0x00ff0000, 0xff);
#endif
}

VOID
_PHY_PathAStandBy8188F(
	IN PADAPTER pAdapter
)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);
	PDM_ODM_T pDM_Odm = &pHalData->odmpriv;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path-A standby mode!\n"));

	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x000000);
//Allen
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_AC, bMaskDWord, 0x10000);
	//ODM_SetBBReg(pDM_Odm, 0x840, bMaskDWord, 0x00010000);
//
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x808000);
}

VOID
_PHY_PIModeSwitch8188F(
	IN PADAPTER pAdapter,
	IN BOOLEAN PIMode
)
{
	u4Byte mode;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);
	PDM_ODM_T pDM_Odm = &pHalData->odmpriv;
	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BB Switch to %s mode!\n", (PIMode ? "PI" : "SI")));

	mode = PIMode ? 0x01000100 : 0x01000000;
	ODM_SetBBReg(pDM_Odm, rFPGA0_XA_HSSIParameter1, bMaskDWord, mode);
	ODM_SetBBReg(pDM_Odm, rFPGA0_XB_HSSIParameter1, bMaskDWord, mode);
}

BOOLEAN
phy_SimularityCompare_8188F(
	IN PADAPTER pAdapter,
	IN s4Byte result[][8],
	IN u1Byte c1,
	IN u1Byte c2
)
{
	u4Byte i, j, diff, SimularityBitMap, bound = 0;
#if DBG
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);
	PDM_ODM_T pDM_Odm = &pHalData->odmpriv;
#endif
	u1Byte final_candidate[2] = { 0xFF, 0xFF };  //for path A and path B
	BOOLEAN bResult = TRUE;
	BOOLEAN is2T = TRUE;

	s4Byte tmp1 = 0, tmp2 = 0;

	if (is2T)
		bound = 8;
	else
		bound = 4;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("===> IQK:phy_SimularityCompare_8192E c1 %d c2 %d!!!\n", c1, c2));


	SimularityBitMap = 0;

	for (i = 0; i < bound; i++) {

		if ((i == 1) || (i == 3) || (i == 5) || (i == 7)) {
			if ((result[c1][i] & 0x00000200) != 0)
				tmp1 = result[c1][i] | 0xFFFFFC00;
			else
				tmp1 = result[c1][i];

			if ((result[c2][i] & 0x00000200) != 0)
				tmp2 = result[c2][i] | 0xFFFFFC00;
			else
				tmp2 = result[c2][i];
		} else {
			tmp1 = result[c1][i];
			tmp2 = result[c2][i];
		}

		diff = (tmp1 > tmp2) ? (tmp1 - tmp2) : (tmp2 - tmp1);

		if (diff > MAX_TOLERANCE) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK:differnece overflow %d index %d compare1 0x%x compare2 0x%x!!!\n", diff, i, result[c1][i], result[c2][i]));

			if ((i == 2 || i == 6) && !SimularityBitMap) {
				if (result[c1][i] + result[c1][i + 1] == 0)
					final_candidate[(i / 4)] = c2;
				else if (result[c2][i] + result[c2][i + 1] == 0)
					final_candidate[(i / 4)] = c1;
				else
					SimularityBitMap = SimularityBitMap | (1 << i);
			} else
				SimularityBitMap = SimularityBitMap | (1 << i);
		}
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK:phy_SimularityCompare_8192E SimularityBitMap   %x !!!\n", SimularityBitMap));

	if (SimularityBitMap == 0) {
		for (i = 0; i < (bound / 4); i++) {
			if (final_candidate[i] != 0xFF) {
				for (j = i * 4; j < (i + 1) * 4 - 2; j++) result[3][j] = result[final_candidate[i]][j];
				bResult = FALSE;
			}
		}
		return bResult;
	} else {

		if (!(SimularityBitMap & 0x03)) {       //path A TX OK
			for (i = 0; i < 2; i++) result[3][i] = result[c1][i];
		}

		if (!(SimularityBitMap & 0x0c)) {       //path A RX OK
			for (i = 2; i < 4; i++) result[3][i] = result[c1][i];
		}

		if (!(SimularityBitMap & 0x30)) { //path B TX OK
			for (i = 4; i < 6; i++) result[3][i] = result[c1][i];

		}

		if (!(SimularityBitMap & 0xc0)) { //path B RX OK
			for (i = 6; i < 8; i++) result[3][i] = result[c1][i];
		}
		return FALSE;
	}
}



VOID
phy_IQCalibrate_8188F(
	IN PADAPTER pAdapter,
	IN s4Byte result[][8],
	IN u1Byte t,
	IN BOOLEAN is2T
)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);
	PDM_ODM_T pDM_Odm = &pHalData->odmpriv;

	u4Byte i;
	u1Byte PathAOK, PathBOK;
	u1Byte tmp0xc50 = (u1Byte)ODM_GetBBReg(pDM_Odm, 0xC50, bMaskByte0);
	u1Byte tmp0xc58 = (u1Byte)ODM_GetBBReg(pDM_Odm, 0xC58, bMaskByte0);
	u4Byte ADDA_REG[IQK_ADDA_REG_NUM] = {
		rFPGA0_XCD_SwitchControl, rBlue_Tooth,
		rRx_Wait_CCA, rTx_CCK_RFON,
		rTx_CCK_BBON, rTx_OFDM_RFON,
		rTx_OFDM_BBON, rTx_To_Rx,
		rTx_To_Tx, rRx_CCK,
		rRx_OFDM, rRx_Wait_RIFS,
		rRx_TO_Rx, rStandby,
		rSleep, rPMPD_ANAEN
	};
	u4Byte IQK_MAC_REG[IQK_MAC_REG_NUM] = {
		REG_TXPAUSE, REG_BCN_CTRL,
		REG_BCN_CTRL_1, REG_GPIO_MUXCFG
	};

	//since 92C & 92D have the different define in IQK_BB_REG
	u4Byte IQK_BB_REG_92C[IQK_BB_REG_NUM] = {
		rOFDM0_TRxPathEnable, rOFDM0_TRMuxPar,
		rFPGA0_XCD_RFInterfaceSW, rConfig_AntA, rConfig_AntB,
		rFPGA0_XAB_RFInterfaceSW, rFPGA0_XA_RFInterfaceOE,
		rFPGA0_XB_RFInterfaceOE, rFPGA0_RFMOD
	};

	u4Byte Path_SEL_BB, Path_SEL_RF;

	const u4Byte retryCount = 2;

	// Note: IQ calibration must be performed after loading
	// 		PHY_REG.txt , and radio_a, radio_b.txt

	//u4Byte bbvalue;


	if (t == 0) {
//	 	 bbvalue = ODM_GetBBReg(pDM_Odm, rFPGA0_RFMOD, bMaskDWord);
//			RT_DISP(FINIT, INIT_IQK, ("phy_IQCalibrate_8188F()==>0x%08x\n",bbvalue));

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQ Calibration for %s for %d times\n", (is2T ? "2T2R" : "1T1R"), t));

		// Save ADDA parameters, turn Path A ADDA on
		_PHY_SaveADDARegisters8188F(pAdapter, ADDA_REG, pDM_Odm->RFCalibrateInfo.ADDA_backup, IQK_ADDA_REG_NUM);
		_PHY_SaveMACRegisters8188F(pAdapter, IQK_MAC_REG, pDM_Odm->RFCalibrateInfo.IQK_MAC_backup);
		_PHY_SaveADDARegisters8188F(pAdapter, IQK_BB_REG_92C, pDM_Odm->RFCalibrateInfo.IQK_BB_backup, IQK_BB_REG_NUM);
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQ Calibration for %s for %d times\n", (is2T ? "2T2R" : "1T1R"), t));

	_PHY_PathADDAOn8188F(pAdapter, ADDA_REG, TRUE, is2T);


	if (t == 0)
		pDM_Odm->RFCalibrateInfo.bRfPiEnable = (u1Byte)ODM_GetBBReg(pDM_Odm, rFPGA0_XA_HSSIParameter1, BIT(8));

#if 0
	if (!pDM_Odm->RFCalibrateInfo.bRfPiEnable) {
		// Switch BB to PI mode to do IQ Calibration.
		_PHY_PIModeSwitch8188F(pAdapter, TRUE);
	}
#endif

	//save RF path
	Path_SEL_BB = ODM_GetBBReg(pDM_Odm, 0x948, bMaskDWord);
	Path_SEL_RF = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xb0, 0xfffff);


	//BB setting
	/*ODM_SetBBReg(pDM_Odm, rFPGA0_RFMOD, BIT24, 0x00);*/
	ODM_SetBBReg(pDM_Odm, rOFDM0_TRxPathEnable, bMaskDWord, 0x03a05600);
	ODM_SetBBReg(pDM_Odm, rOFDM0_TRMuxPar, bMaskDWord, 0x000800e4);
	ODM_SetBBReg(pDM_Odm, rFPGA0_XCD_RFInterfaceSW, bMaskDWord, 0x25204000);

	//external switch control
//	ODM_SetBBReg(pDM_Odm, rFPGA0_XAB_RFInterfaceSW, BIT10, 0x01);
//	ODM_SetBBReg(pDM_Odm, rFPGA0_XAB_RFInterfaceSW, BIT26, 0x01);
//	ODM_SetBBReg(pDM_Odm, rFPGA0_XA_RFInterfaceOE, BIT10, 0x00);
//	ODM_SetBBReg(pDM_Odm, rFPGA0_XB_RFInterfaceOE, BIT10, 0x00);


	if (is2T) {
		//Allen
		//	ODM_SetBBReg(pDM_Odm, rFPGA0_XA_LSSIParameter, bMaskDWord, 0x00010000);
		//	ODM_SetBBReg(pDM_Odm, rFPGA0_XB_LSSIParameter, bMaskDWord, 0x00010000);
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, RF_AC, bMaskDWord, 0x10000);
	}

	//MAC settings
	_PHY_MACSettingCalibration8188F(pAdapter, IQK_MAC_REG, pDM_Odm->RFCalibrateInfo.IQK_MAC_backup);


	//Page B init
	//AP or IQK
//	ODM_SetBBReg(pDM_Odm, rConfig_AntA, bMaskDWord, 0x0f600000);

	if (is2T) {
//		ODM_SetBBReg(pDM_Odm, rConfig_AntB, bMaskDWord, 0x0f600000);
	}

	// IQ calibration setting
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK setting!\n"));
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x808000);
	ODM_SetBBReg(pDM_Odm, rTx_IQK, bMaskDWord, 0x01007c00);
	ODM_SetBBReg(pDM_Odm, rRx_IQK, bMaskDWord, 0x01004800);

	for (i = 0; i < retryCount; i++) {
		PathAOK = phy_PathA_IQK_8188F(pAdapter, is2T);
//		if(PathAOK == 0x03){
		if (PathAOK == 0x01) { //Path A Tx IQK Success
			ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x000000);
			pDM_Odm->RFCalibrateInfo.TxLOK[ODM_RF_PATH_A] = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x8, bRFRegOffsetMask);

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path A Tx IQK Success!!\n"));
			result[t][0] = (ODM_GetBBReg(pDM_Odm, rTx_Power_Before_IQK_A, bMaskDWord) & 0x3FF0000) >> 16;
			result[t][1] = (ODM_GetBBReg(pDM_Odm, rTx_Power_After_IQK_A, bMaskDWord) & 0x3FF0000) >> 16;
			break;
		}
#if 0
		else if (i == (retryCount - 1) && PathAOK == 0x01) { //Tx IQK OK
			RT_DISP(FINIT, INIT_IQK, ("Path A IQK Only  Tx Success!!\n"));

			result[t][0] = (ODM_GetBBReg(pDM_Odm, rTx_Power_Before_IQK_A, bMaskDWord) & 0x3FF0000) >> 16;
			result[t][1] = (ODM_GetBBReg(pDM_Odm, rTx_Power_After_IQK_A, bMaskDWord) & 0x3FF0000) >> 16;
		}
#endif
	}

//bypass RXQIK
#if 1

	for (i = 0; i < retryCount; i++) {
		PathAOK = phy_PathA_RxIQK8188F(pAdapter, is2T);
		if (PathAOK == 0x03) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path A Rx IQK Success!!\n"));
//				result[t][0] = (ODM_GetBBReg(pDM_Odm, rTx_Power_Before_IQK_A, bMaskDWord)&0x3FF0000)>>16;
//				result[t][1] = (ODM_GetBBReg(pDM_Odm, rTx_Power_After_IQK_A, bMaskDWord)&0x3FF0000)>>16;
			result[t][2] = (ODM_GetBBReg(pDM_Odm, rRx_Power_Before_IQK_A_2, bMaskDWord) & 0x3FF0000) >> 16;
			result[t][3] = (ODM_GetBBReg(pDM_Odm, rRx_Power_After_IQK_A_2, bMaskDWord) & 0x3FF0000) >> 16;
			break;
		} else
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path A Rx IQK Fail!!\n"));
	}
#endif


	if (0x00 == PathAOK)
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path A IQK failed!!\n"));

	if (is2T) {
		_PHY_PathAStandBy8188F(pAdapter);

		// Turn Path B ADDA on
		_PHY_PathADDAOn8188F(pAdapter, ADDA_REG, FALSE, is2T);
//Allen
		for (i = 0; i < retryCount; i++) {
			PathBOK = phy_PathB_IQK_8188F(pAdapter);
//		if(PathBOK == 0x03){
			if (PathBOK == 0x01) { //Path B Tx IQK Success
				ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x000000);
				pDM_Odm->RFCalibrateInfo.TxLOK[ODM_RF_PATH_B] = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x8, bRFRegOffsetMask);

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path B Tx IQK Success!!\n"));
				result[t][4] = (ODM_GetBBReg(pDM_Odm, rTx_Power_Before_IQK_A, bMaskDWord) & 0x3FF0000) >> 16;
				result[t][5] = (ODM_GetBBReg(pDM_Odm, rTx_Power_After_IQK_A, bMaskDWord) & 0x3FF0000) >> 16;
				break;
			}
#if 0
			else if (i == (retryCount - 1) && PathAOK == 0x01) { //Tx IQK OK
				RT_DISP(FINIT, INIT_IQK, ("Path B IQK Only  Tx Success!!\n"));

				result[t][0] = (ODM_GetBBReg(pDM_Odm, rTx_Power_Before_IQK_B, bMaskDWord) & 0x3FF0000) >> 16;
				result[t][1] = (ODM_GetBBReg(pDM_Odm, rTx_Power_After_IQK_B, bMaskDWord) & 0x3FF0000) >> 16;
			}
#endif
		}

//bypass RXQIK
#if 0

		for (i = 0; i < retryCount; i++) {
			PathBOK = phy_PathB_RxIQK8188F(pAdapter, is2T);
			if (PathBOK == 0x03) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path B Rx IQK Success!!\n"));
//				result[t][0] = (ODM_GetBBReg(pDM_Odm, rTx_Power_Before_IQK_A, bMaskDWord)&0x3FF0000)>>16;
//				result[t][1] = (ODM_GetBBReg(pDM_Odm, rTx_Power_After_IQK_A, bMaskDWord)&0x3FF0000)>>16;
				result[t][6] = (ODM_GetBBReg(pDM_Odm, rRx_Power_Before_IQK_B_2, bMaskDWord) & 0x3FF0000) >> 16;
				result[t][7] = (ODM_GetBBReg(pDM_Odm, rRx_Power_After_IQK_B_2, bMaskDWord) & 0x3FF0000) >> 16;
				break;
			} else
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path B Rx IQK Fail!!\n"));
		}

#endif

////////Allen end /////////
		if (0x00 == PathBOK)
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path B IQK failed!!\n"));
	}

	//Back to BB mode, load original value
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK:Back to BB mode, load original value!\n"));
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0);

	if (t != 0) {
		if (!pDM_Odm->RFCalibrateInfo.bRfPiEnable) {
			// Switch back BB to SI mode after finish IQ Calibration.
			_PHY_PIModeSwitch8188F(pAdapter, FALSE);
		}

		// Reload ADDA power saving parameters
		_PHY_ReloadADDARegisters8188F(pAdapter, ADDA_REG, pDM_Odm->RFCalibrateInfo.ADDA_backup, IQK_ADDA_REG_NUM);

		// Reload MAC parameters
		_PHY_ReloadMACRegisters8188F(pAdapter, IQK_MAC_REG, pDM_Odm->RFCalibrateInfo.IQK_MAC_backup);

		_PHY_ReloadADDARegisters8188F(pAdapter, IQK_BB_REG_92C, pDM_Odm->RFCalibrateInfo.IQK_BB_backup, IQK_BB_REG_NUM);


		//Reload RF path
		ODM_SetBBReg(pDM_Odm, 0x948, bMaskDWord, Path_SEL_BB);
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xb0, 0xfffff, Path_SEL_RF);

		//Allen initial gain 0xc50
		// Restore RX initial gain
		ODM_SetBBReg(pDM_Odm, 0xc50, bMaskByte0, 0x50);
		ODM_SetBBReg(pDM_Odm, 0xc50, bMaskByte0, tmp0xc50);
		if (is2T) {
			ODM_SetBBReg(pDM_Odm, 0xc58, bMaskByte0, 0x50);
			ODM_SetBBReg(pDM_Odm, 0xc58, bMaskByte0, tmp0xc58);
		}

		//load 0xe30 IQC default value
		ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_A, bMaskDWord, 0x01008c00);
		ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_A, bMaskDWord, 0x01008c00);

	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("phy_IQCalibrate_8188F() <==\n"));

}


VOID
_rtl8188fu_phy_phy_lc_calibrate(
	IN PDM_ODM_T pDM_Odm,
	IN BOOLEAN is2T
)
{
	u1Byte tmpReg;
	u4Byte RF_Amode = 0, RF_Bmode = 0, LC_Cal, cnt;
	PADAPTER pAdapter = pDM_Odm->Adapter;

	/*Check continuous TX and Packet TX*/
	tmpReg = ODM_Read1Byte(pDM_Odm, 0xd03);

	if ((tmpReg & 0x70) != 0)			/*Deal with contisuous TX case*/
		ODM_Write1Byte(pDM_Odm, 0xd03, tmpReg & 0x8F);	/*disable all continuous TX*/
	else							/* Deal with Packet TX case*/
		ODM_Write1Byte(pDM_Odm, REG_TXPAUSE, 0xFF);			/* block all queues*/


	/*backup RF0x18*/
	LC_Cal = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask);

	/*Start LCK*/
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, LC_Cal|0x08000);

	for(cnt=0;cnt<100;cnt++)
	{
		if(ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, 0x8000) != 0x1)
		break;
		ODM_delay_ms(10);
	}

	/*Recover channel number*/
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, LC_Cal);


	/*Restore original situation*/
	if ((tmpReg&0x70) != 0) {
		/*Deal with contisuous TX case*/
		ODM_Write1Byte(pDM_Odm, 0xd03, tmpReg);
	} else {
		/* Deal with Packet TX case*/
		ODM_Write1Byte(pDM_Odm, REG_TXPAUSE, 0x00);
	}

}

//Analog Pre-distortion calibration
#define		APK_BB_REG_NUM	8
#define		APK_CURVE_REG_NUM 4
#define		PATH_NUM		2

VOID
phy_APCalibrate_8188F(
	IN PADAPTER pAdapter,
	IN s1Byte delta,
	IN BOOLEAN is2T
)
{
}



#define		DP_BB_REG_NUM		7
#define		DP_RF_REG_NUM		1
#define		DP_RETRY_LIMIT		10
#define		DP_PATH_NUM		2
#define		DP_DPK_NUM			3
#define		DP_DPK_VALUE_NUM	2




VOID
PHY_IQCalibrate_8188F(
	IN PADAPTER pAdapter,
	IN BOOLEAN bReCovery,
	IN BOOLEAN bRestore
)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);

	PDM_ODM_T pDM_Odm = &pHalData->odmpriv;


	s4Byte result[4][8];   //last is final result
	u1Byte i, final_candidate, Indexforchannel;
	BOOLEAN bPathAOK, bPathBOK;
#if DBG
	s4Byte RegE94, RegE9C, RegEA4, RegEAC, RegEB4, RegEBC, RegEC4, RegECC, RegTmp = 0;
#else
	s4Byte RegE94, RegEA4, RegEB4, RegEC4, RegTmp = 0;
#endif
	BOOLEAN is12simular, is13simular, is23simular;
	BOOLEAN bSingleTone = FALSE, bCarrierSuppression = FALSE;
	u4Byte IQK_BB_REG_92C[IQK_BB_REG_NUM] = {
		rOFDM0_XARxIQImbalance, rOFDM0_XBRxIQImbalance,
		rOFDM0_ECCAThreshold, rOFDM0_AGCRSSITable,
		rOFDM0_XATxIQImbalance, rOFDM0_XBTxIQImbalance,
		rOFDM0_XCTxAFE, rOFDM0_XDTxAFE,
		rOFDM0_RxIQExtAnta
	};
	u4Byte Path_SEL_BB = 0, Path_SEL_RF = 0;

	if (ODM_CheckPowerStatus(pAdapter) == FALSE)
		return;

	if (!(pDM_Odm->SupportAbility & ODM_RF_CALIBRATION))
		return;

	// 20120213<Kordan> Turn on when continuous Tx to pass lab testing. (required by Edlu)
	if (bSingleTone || bCarrierSuppression)
		return;

	if (bRestore) {
		u4Byte offset, data;
		u1Byte path, bResult = SUCCESS;
		PODM_RF_CAL_T pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);

		//#define 	PATH_S0                         1 // RF_PATH_B
		//#define 	PATH_S1                         0 // RF_PATH_A

		path = (ODM_GetBBReg(pDM_Odm, rS0S1_PathSwitch, bMaskByte0) == 0x00) ? ODM_RF_PATH_A : ODM_RF_PATH_B;
		//Restore TX IQK
		for (i = 0; i < 3; ++i) {
			offset = pRFCalibrateInfo->TxIQC_8723B[path][i][0];
			data = pRFCalibrateInfo->TxIQC_8723B[path][i][1];
			if ((offset == 0) || (data == 0)) {
				//DBG_871X("%s =>path:%s Restore TX IQK result failed\n",__FUNCTION__,(path==ODM_RF_PATH_A)?"A":"B");
				bResult = FAIL;
				break;
			}
			RT_TRACE(COMP_MP, DBG_TRACE, ("Switch to S1 TxIQC(offset, data) = (0x%X, 0x%X)\n", offset, data));
			ODM_SetBBReg(pDM_Odm, offset, bMaskDWord, data);
		}
		//Restore RX IQK
		for (i = 0; i < 2; ++i) {
			offset = pRFCalibrateInfo->RxIQC_8723B[path][i][0];
			data = pRFCalibrateInfo->RxIQC_8723B[path][i][1];
			if ((offset == 0) || (data == 0)) {
				//DBG_871X("%s =>path:%s  Restore RX IQK result failed\n",__FUNCTION__,(path==ODM_RF_PATH_A)?"A":"B");
				bResult = FAIL;
				break;
			}
			RT_TRACE(COMP_MP, DBG_TRACE, ("Switch to S1 RxIQC (offset, data) = (0x%X, 0x%X)\n", offset, data));
			ODM_SetBBReg(pDM_Odm, offset, bMaskDWord, data);
		}

		if (pDM_Odm->RFCalibrateInfo.TxLOK[ODM_RF_PATH_A] == 0) {
			//DBG_871X("%s => Restore Path-A TxLOK result failed\n",__FUNCTION__);
			bResult = FAIL;
		} else {
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_TXM_IDAC, bRFRegOffsetMask, pDM_Odm->RFCalibrateInfo.TxLOK[ODM_RF_PATH_A]);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, RF_TXM_IDAC, bRFRegOffsetMask, pDM_Odm->RFCalibrateInfo.TxLOK[ODM_RF_PATH_B]);
		}

		if (bResult == SUCCESS)
			return;

	}

	if (bReCovery)
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD, ("PHY_IQCalibrate_8188F: Return due to bReCovery!\n"));
		_PHY_ReloadADDARegisters8188F(pAdapter, IQK_BB_REG_92C, pDM_Odm->RFCalibrateInfo.IQK_BB_backup_recover, 9);
		return;
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK:Start!!!\n"));


	// Save RF Path
	Path_SEL_BB = ODM_GetBBReg(pDM_Odm, 0x948, bMaskDWord);
	Path_SEL_RF = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xb0, 0xfffff);


	for (i = 0; i < 8; i++) {
		result[0][i] = 0;
		result[1][i] = 0;
		result[2][i] = 0;
		result[3][i] = 0;
	}
	final_candidate = 0xff;
	bPathAOK = FALSE;
	bPathBOK = FALSE;
	is12simular = FALSE;
	is23simular = FALSE;
	is13simular = FALSE;


	for (i = 0; i < 3; i++) {


		phy_IQCalibrate_8188F(pAdapter, result, i, FALSE);



		if (i == 1) {
			is12simular = phy_SimularityCompare_8188F(pAdapter, result, 0, 1);
			if (is12simular) {
				final_candidate = 0;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK: is12simular final_candidate is %x\n", final_candidate));
				break;
			}
		}

		if (i == 2) {
			is13simular = phy_SimularityCompare_8188F(pAdapter, result, 0, 2);
			if (is13simular) {
				final_candidate = 0;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK: is13simular final_candidate is %x\n", final_candidate));

				break;
			}
			is23simular = phy_SimularityCompare_8188F(pAdapter, result, 1, 2);
			if (is23simular) {
				final_candidate = 1;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK: is23simular final_candidate is %x\n", final_candidate));
			} else {
				for (i = 0; i < 8; i++) RegTmp += result[3][i];

				if (RegTmp != 0)
					final_candidate = 3;
				else
					final_candidate = 0xFF;
			}
		}
	}
//	RT_TRACE(COMP_INIT,DBG_LOUD,("Release Mutex in IQCalibrate\n"));

	for (i = 0; i < 4; i++) {
		RegE94 = result[i][0];
		RegEA4 = result[i][2];
		RegEB4 = result[i][4];
		RegEC4 = result[i][6];
#if DBG
		RegE9C = result[i][1];
		RegEAC = result[i][3];
		RegEBC = result[i][5];
		RegECC = result[i][7];
#endif
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK: RegE94=%04x RegE9C=%04x RegEA4=%04x RegEAC=%04x RegEB4=%04x RegEBC=%04x RegEC4=%04x RegECC=%04x\n",
					 RegE94, RegE9C, RegEA4, RegEAC, RegEB4, RegEBC, RegEC4, RegECC));
	}

	if (final_candidate != 0xff) {
		pDM_Odm->RFCalibrateInfo.RegE94 = result[final_candidate][0];
		pDM_Odm->RFCalibrateInfo.RegE9C = result[final_candidate][1];
		pDM_Odm->RFCalibrateInfo.RegEB4 = result[final_candidate][4];
		pDM_Odm->RFCalibrateInfo.RegEBC = result[final_candidate][5];

		RegE94 = result[final_candidate][0];
		RegEA4 = result[final_candidate][2];
		RegEB4 = result[final_candidate][4];
		RegEC4 = result[final_candidate][6];
#if DBG
		RegE9C = result[final_candidate][1];
		RegEAC = result[final_candidate][3];
		RegEBC = result[final_candidate][5];
		RegECC = result[final_candidate][7];
#endif
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK: final_candidate is %x\n", final_candidate));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK: RegE94=%04x RegE9C=%04x RegEA4=%04x RegEAC=%04x RegEB4=%04x RegEBC=%04x RegEC4=%04x RegECC=%04x\n",
					 RegE94, RegE9C, RegEA4, RegEAC, RegEB4, RegEBC, RegEC4, RegECC));
		bPathAOK = bPathBOK = TRUE;
	} else {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK: FAIL use default value\n"));

		pDM_Odm->RFCalibrateInfo.RegE94 = pDM_Odm->RFCalibrateInfo.RegEB4 = 0x100;  //X default value
		pDM_Odm->RFCalibrateInfo.RegE9C = pDM_Odm->RFCalibrateInfo.RegEBC = 0x0;        //Y default value
	}

	{
		if (RegE94 != 0) {
			_PHY_PathAFillIQKMatrix8188F(pAdapter, bPathAOK, result, final_candidate, (RegEA4 == 0));
		}
	}

	{
		if (RegEB4 != 0)
			_PHY_PathBFillIQKMatrix8188F(pAdapter, bPathBOK, result, final_candidate, (RegEC4 == 0));
	}

	Indexforchannel = ODM_GetRightChnlPlaceforIQK(pHalData->CurrentChannel);

//To Fix BSOD when final_candidate is 0xff
//by sherry 20120321
	if (final_candidate < 4) {
		for (i = 0; i < IQK_Matrix_REG_NUM; i++) pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting[Indexforchannel].Value[0][i] = result[final_candidate][i];
		pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting[Indexforchannel].bIQKDone = TRUE;
	}
	//RT_DISP(FINIT, INIT_IQK, ("\nIQK OK Indexforchannel %d.\n", Indexforchannel));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("\nIQK OK Indexforchannel %d.\n", Indexforchannel));

	_PHY_SaveADDARegisters8188F(pAdapter, IQK_BB_REG_92C, pDM_Odm->RFCalibrateInfo.IQK_BB_backup_recover, 9);

	// Restore RF Path
	ODM_SetBBReg(pDM_Odm, 0x948, bMaskDWord, Path_SEL_BB);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xb0, 0xfffff, Path_SEL_RF);


	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK finished 8188F\n"));


}


VOID
PHY_LCCalibrate_8188F(
	PVOID pDM_VOID
)
{
	BOOLEAN bSingleTone = FALSE, bCarrierSuppression = FALSE;
	u4Byte timeout = 2000, timecount = 0;

	PDM_ODM_T pDM_Odm = (PDM_ODM_T)pDM_VOID;

	if (!(pDM_Odm->SupportAbility & ODM_RF_CALIBRATION))
		return;

	// 20120213<Kordan> Turn on when continuous Tx to pass lab testing. (required by Edlu)
	if (bSingleTone || bCarrierSuppression)
		return;

	while (*(pDM_Odm->pbScanInProcess) && timecount < timeout) {
		ODM_delay_ms(50);
		timecount += 50;
	}

	pDM_Odm->RFCalibrateInfo.bLCKInProgress = TRUE;


	_rtl8188fu_phy_phy_lc_calibrate(pDM_Odm, FALSE);


	pDM_Odm->RFCalibrateInfo.bLCKInProgress = FALSE;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("LCK:Finish!!!interface %d 8188F\n", pDM_Odm->InterfaceIndex));

}
