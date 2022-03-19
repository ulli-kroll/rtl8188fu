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
#define _HAL_COM_PHYCFG_C_

#include <drv_types.h>
#include <hal_data.h>

/*
* rtw_regsty_get_target_tx_power -
*
* Return dBm or -1 for undefined
*/
s8 rtw_regsty_get_target_tx_power(
	IN	PADAPTER		Adapter,
	IN	u8				Band,
	IN	u8				RfPath,
	IN	RATE_SECTION	RateSection
	)
{
	struct registry_priv *regsty = adapter_to_regsty(Adapter);
	s8 value = 0;

	if (RfPath > RF_PATH_D) {
		DBG_871X_LEVEL(_drv_always_, "%s invalid RfPath:%d\n", __func__, RfPath);
		return -1;
	}

	if (Band != BAND_ON_2_4G
		#ifdef CONFIG_IEEE80211_BAND_5GHZ
		&& Band != BAND_ON_5G
		#endif
	) {
		DBG_871X_LEVEL(_drv_always_, "%s invalid Band:%d\n", __func__, Band);
		return -1;
	}

	if (RateSection >= RATE_SECTION_NUM
		#ifdef CONFIG_IEEE80211_BAND_5GHZ
		|| (Band == BAND_ON_5G && RateSection == CCK)
		#endif
	) {
		DBG_871X_LEVEL(_drv_always_, "%s invalid RateSection:%d in %sG, RfPath:%d\n", __func__
			, RateSection, (Band == BAND_ON_2_4G) ? "2.4" : "5", RfPath);
		return -1;
	}

	if (Band == BAND_ON_2_4G)
		value = regsty->target_tx_pwr_2g[RfPath][RateSection];
#ifdef CONFIG_IEEE80211_BAND_5GHZ
	else /* BAND_ON_5G */
		value = regsty->target_tx_pwr_5g[RfPath][RateSection - 1];
#endif

	return value;
}

bool rtw_regsty_chk_target_tx_power_valid(_adapter *adapter)
{
	struct hal_spec_t *hal_spec = GET_HAL_SPEC(adapter);
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(adapter);
	int path, tx_num, band, rs;
	s8 target;

	for (band = BAND_ON_2_4G; band <= BAND_ON_5G; band++) {
		if (!hal_is_band_support(adapter, band))
			continue;

		for (path = 0; path < RF_PATH_MAX; path++) {
			if (path >= hal_data->NumTotalRFPath)
				break;

			for (rs = 0; rs < RATE_SECTION_NUM; rs++) {
				tx_num = rate_section_to_tx_num(rs);
				if (tx_num >= hal_spec->nss_num)
					continue;

				if (band == BAND_ON_5G && IS_CCK_RATE_SECTION(rs))
					continue;

				if (IS_VHT_RATE_SECTION(rs))
					continue;

				target = rtw_regsty_get_target_tx_power(adapter, band, path, rs);
				if (target == -1)
					return _FALSE;
			}
		}
	}

	return _TRUE;
}

/*
* rtl8188fu_get_tx_power_by_rateBase -
*
* Return 2 times of dBm
*/
u8
rtl8188fu_get_tx_power_by_rateBase(
	IN	PADAPTER		Adapter,
	IN	u8				Band,
	IN	u8				RfPath,
	IN	u8				TxNum,
	IN	RATE_SECTION	RateSection
	)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(Adapter);
	u8 value = 0;

	if (RfPath > RF_PATH_D) {
		DBG_871X_LEVEL(_drv_always_, "%s invalid RfPath:%d\n", __func__, RfPath);
		return 0;
	}

	if (Band != BAND_ON_2_4G && Band != BAND_ON_5G) {
		DBG_871X_LEVEL(_drv_always_, "%s invalid Band:%d\n", __func__, Band);
		return 0;
	}

	if (RateSection >= RATE_SECTION_NUM
		|| (Band == BAND_ON_5G && RateSection == CCK)
	) {
		DBG_871X_LEVEL(_drv_always_, "%s invalid RateSection:%d in %sG, RfPath:%d, TxNum:%d\n", __func__
			, RateSection, (Band == BAND_ON_2_4G) ? "2.4" : "5", RfPath, TxNum);
		return 0;
	}

	if (Band == BAND_ON_2_4G)
		value = pHalData->txpwr_by_rate_base_24g[RfPath][TxNum][RateSection];
	else /* BAND_ON_5G */
		value = pHalData->txpwr_by_rate_base_5g[RfPath][TxNum][RateSection - 1];

	return value;
}

static void rtl8188fu_phy_set_txpower_by_rate_base(
	IN	PADAPTER		Adapter,
	IN	u8				Band,
	IN	u8				RfPath,
	IN	RATE_SECTION	RateSection,
	IN	u8				TxNum,
	IN	u8				Value
	)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(Adapter);
	
	if (RfPath > RF_PATH_D) {
		DBG_871X_LEVEL(_drv_always_, "%s invalid RfPath:%d\n", __func__, RfPath);
		return;
	}

	if (Band == BAND_ON_2_4G) {
		/* ULLI : yes we are stupid for 0=CCK 1=OFDM 2=HT_MCS0_MCS7 3=HT_MCS8_MCS15 */
		switch (RateSection) {
		case 0:
			pHalData->txpwr_by_rate_base_24g[RfPath][TxNum][0] = Value;
			break;
		case 1:
			pHalData->txpwr_by_rate_base_24g[RfPath][TxNum][1] = Value;
			break;
		case 2:
			pHalData->txpwr_by_rate_base_24g[RfPath][TxNum][2] = Value;
			break;
		case 3:
			pHalData->txpwr_by_rate_base_24g[RfPath][TxNum][3] = Value;
			break;
		default:
			DBG_871X_LEVEL(_drv_always_, "%s invalid RateSection:%d in %sG, RfPath:%d, TxNum:%d\n", __func__
				, RateSection, (Band == BAND_ON_2_4G) ? "2.4" : "5", RfPath, TxNum);
			break;
		}
	} else {
		DBG_871X_LEVEL(_drv_always_, "%s invalid Band:%d\n", __func__, Band);
	}

	if (Band != BAND_ON_2_4G && Band != BAND_ON_5G) {
		DBG_871X_LEVEL(_drv_always_, "%s invalid Band:%d\n", __func__, Band);
		return;
	}
}

/*
* phy_get_target_tx_power -
*
* Return 2 times of dBm
*/
u8 phy_get_target_tx_power(
	IN	PADAPTER		Adapter,
	IN	u8				Band,
	IN	u8				RfPath,
	IN	RATE_SECTION	RateSection
	)
{
	struct registry_priv *regsty = adapter_to_regsty(Adapter);
	s16 target_power;

	if (phy_is_tx_power_by_rate_needed(Adapter) == _FALSE && regsty->target_tx_pwr_valid == _TRUE)
		target_power = 2 * rtw_regsty_get_target_tx_power(Adapter, Band, RfPath, RateSection);
	else
		target_power = rtl8188fu_get_tx_power_by_rateBase(Adapter, Band, RfPath, rate_section_to_tx_num(RateSection), RateSection);

	return target_power;
}

static void rtl8188fu_phy_store_txpower_by_rate_base(
	IN	PADAPTER	pAdapter
	)
{
	struct hal_spec_t *hal_spec = GET_HAL_SPEC(pAdapter);

	u8 rate_sec_base[RATE_SECTION_NUM] = {
		MGN_11M,
		MGN_54M,
		MGN_MCS7,
		MGN_MCS15,
		MGN_MCS23,
		MGN_MCS31,
		MGN_VHT1SS_MCS7,
		MGN_VHT2SS_MCS7,
		MGN_VHT3SS_MCS7,
		MGN_VHT4SS_MCS7,
	};

	u8 band, path, rs, tx_num, base, index;

	for (band = BAND_ON_2_4G; band <= BAND_ON_5G; band++) {

		for (path = RF_PATH_A; path < RF_PATH_MAX; path++) {
			/* TODO: 8814A's NumTotalRFPath differs at probe(3) and up(4), need fixed
			if (path >= hal_data->NumTotalRFPath)
				break;
			*/

			for (rs = 0; rs < RATE_SECTION_NUM; rs++) {
				tx_num = rate_section_to_tx_num(rs);
				if (tx_num >= hal_spec->nss_num)
					continue;

				if (band == BAND_ON_5G && IS_CCK_RATE_SECTION(rs))
					continue;

				base = _rtl8188fu_get_tx_power_by_rate(pAdapter, band, path, tx_num, rate_sec_base[rs]);
				rtl8188fu_phy_set_txpower_by_rate_base(pAdapter, band, path, rs, tx_num, base);
			}
		}
	}
}

VOID
PHY_InitTxPowerByRate(
	IN	PADAPTER	pAdapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	u8	band = 0, rfPath = 0, TxNum = 0, rate = 0, i = 0, j = 0;

	if ( IS_HARDWARE_TYPE_8188E( pAdapter ) )
	{
		for ( i = 0; i < MAX_PG_GROUP; ++i )
			for ( j = 0; j < 16; ++j )
				pHalData->MCSTxPowerLevelOriginalOffset[i][j] = 0;
	}
	else
	{
		for ( band = BAND_ON_2_4G; band <= BAND_ON_5G; ++band )
				for ( rfPath = 0; rfPath < TX_PWR_BY_RATE_NUM_RF; ++rfPath )
					for ( TxNum = 0; TxNum < TX_PWR_BY_RATE_NUM_RF; ++TxNum )
						for ( rate = 0; rate < TX_PWR_BY_RATE_NUM_RATE; ++rate )
							pHalData->tx_power_by_rate_offset[band][rfPath][TxNum][rate] = 0;
	}
}

static void _rtl8188fu_phy_set_tx_power_by_rate(
	IN	PADAPTER	pAdapter, 
	IN	u8			Band, 
	IN	u8			RFPath, 
	IN	u8			TxNum, 
	IN	u8			Rate,
	IN	s8			Value
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA( pAdapter );
	u8	rateIndex = PHY_GetRateIndexOfTxPowerByRate( Rate );
	
	if ( Band != BAND_ON_2_4G && Band != BAND_ON_5G )
	{
		DBG_871X("Invalid band %d in %s\n", Band, __FUNCTION__ );
		return;
	}
	if ( RFPath > ODM_RF_PATH_D )
	{
		DBG_871X("Invalid RfPath %d in %s\n", RFPath, __FUNCTION__ );
		return;
	}
	if ( TxNum >= RF_MAX_TX_NUM )
	{
		DBG_871X( "Invalid TxNum %d in %s\n", TxNum, __FUNCTION__ );
		return;
	}
	if ( rateIndex >= TX_PWR_BY_RATE_NUM_RATE )
	{
		DBG_871X("Invalid RateIndex %d in %s\n", rateIndex, __FUNCTION__ );
		return;
	}

	pHalData->tx_power_by_rate_offset[Band][RFPath][TxNum][rateIndex] = Value;
}

static void rtl8188fu_phy_convert_txpower_dbm_to_relative_value(
	IN	PADAPTER	pAdapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA( pAdapter );
	u8 			base = 0, i = 0, value = 0,
				band = 0, path = 0, txNum = 0, index = 0, 
				startIndex = 0, endIndex = 0;
	u8			cckRates[4] = {MGN_1M, MGN_2M, MGN_5_5M, MGN_11M},
				ofdmRates[8] = {MGN_6M, MGN_9M, MGN_12M, MGN_18M, MGN_24M, MGN_36M, MGN_48M, MGN_54M},
				mcs0_7Rates[8] = {MGN_MCS0, MGN_MCS1, MGN_MCS2, MGN_MCS3, MGN_MCS4, MGN_MCS5, MGN_MCS6, MGN_MCS7},
				mcs8_15Rates[8] = {MGN_MCS8, MGN_MCS9, MGN_MCS10, MGN_MCS11, MGN_MCS12, MGN_MCS13, MGN_MCS14, MGN_MCS15},
				mcs16_23Rates[8] = {MGN_MCS16, MGN_MCS17, MGN_MCS18, MGN_MCS19, MGN_MCS20, MGN_MCS21, MGN_MCS22, MGN_MCS23},
				vht1ssRates[10] = {MGN_VHT1SS_MCS0, MGN_VHT1SS_MCS1, MGN_VHT1SS_MCS2, MGN_VHT1SS_MCS3, MGN_VHT1SS_MCS4, 
							   MGN_VHT1SS_MCS5, MGN_VHT1SS_MCS6, MGN_VHT1SS_MCS7, MGN_VHT1SS_MCS8, MGN_VHT1SS_MCS9},
				vht2ssRates[10] = {MGN_VHT2SS_MCS0, MGN_VHT2SS_MCS1, MGN_VHT2SS_MCS2, MGN_VHT2SS_MCS3, MGN_VHT2SS_MCS4, 
							   MGN_VHT2SS_MCS5, MGN_VHT2SS_MCS6, MGN_VHT2SS_MCS7, MGN_VHT2SS_MCS8, MGN_VHT2SS_MCS9},
				vht3ssRates[10] = {MGN_VHT3SS_MCS0, MGN_VHT3SS_MCS1, MGN_VHT3SS_MCS2, MGN_VHT3SS_MCS3, MGN_VHT3SS_MCS4, 
								   MGN_VHT3SS_MCS5, MGN_VHT3SS_MCS6, MGN_VHT3SS_MCS7, MGN_VHT3SS_MCS8, MGN_VHT3SS_MCS9};

	//DBG_871X("===>PHY_ConvertTxPowerByRateInDbmToRelativeValues()\n" );

	for ( band = BAND_ON_2_4G; band <= BAND_ON_5G; ++band )
	{
		for ( path = ODM_RF_PATH_A; path <= ODM_RF_PATH_D; ++path )
		{
			for ( txNum = RF_1TX; txNum < RF_MAX_TX_NUM; ++txNum )
			{
				// CCK
				base = rtl8188fu_get_tx_power_by_rate( pAdapter, band, path, txNum, MGN_11M );
				for ( i = 0; i < sizeof( cckRates ); ++i )
				{
					value = rtl8188fu_get_tx_power_by_rate( pAdapter, band, path, txNum, cckRates[i] );
					_rtl8188fu_phy_set_tx_power_by_rate( pAdapter, band, path, txNum, cckRates[i], value - base );
				}

				// OFDM
				base = rtl8188fu_get_tx_power_by_rate( pAdapter, band, path, txNum, MGN_54M );
				for ( i = 0; i < sizeof( ofdmRates ); ++i )
				{
					value = rtl8188fu_get_tx_power_by_rate( pAdapter, band, path, txNum, ofdmRates[i] );
					_rtl8188fu_phy_set_tx_power_by_rate( pAdapter, band, path, txNum, ofdmRates[i], value - base );
				}
				
				// HT MCS0~7
				base = rtl8188fu_get_tx_power_by_rate( pAdapter, band, path, txNum, MGN_MCS7 );
				for ( i = 0; i < sizeof( mcs0_7Rates ); ++i )
				{
					value = rtl8188fu_get_tx_power_by_rate( pAdapter, band, path, txNum, mcs0_7Rates[i] );
					_rtl8188fu_phy_set_tx_power_by_rate( pAdapter, band, path, txNum, mcs0_7Rates[i], value - base );
				}

				// HT MCS8~15
				base = rtl8188fu_get_tx_power_by_rate( pAdapter, band, path, txNum, MGN_MCS15 );
				for ( i = 0; i < sizeof( mcs8_15Rates ); ++i )
				{
					value = rtl8188fu_get_tx_power_by_rate( pAdapter, band, path, txNum, mcs8_15Rates[i] );
					_rtl8188fu_phy_set_tx_power_by_rate( pAdapter, band, path, txNum, mcs8_15Rates[i], value - base );
				}

				// HT MCS16~23
				base = rtl8188fu_get_tx_power_by_rate( pAdapter, band, path, txNum, MGN_MCS23 );
				for ( i = 0; i < sizeof( mcs16_23Rates ); ++i )
				{
					value = rtl8188fu_get_tx_power_by_rate( pAdapter, band, path, txNum, mcs16_23Rates[i] );
					_rtl8188fu_phy_set_tx_power_by_rate( pAdapter, band, path, txNum, mcs16_23Rates[i], value - base );
				}

				// VHT 1SS
				base = rtl8188fu_get_tx_power_by_rate( pAdapter, band, path, txNum, MGN_VHT1SS_MCS7 );
				for ( i = 0; i < sizeof( vht1ssRates ); ++i )
				{
					value = rtl8188fu_get_tx_power_by_rate( pAdapter, band, path, txNum, vht1ssRates[i] );
					_rtl8188fu_phy_set_tx_power_by_rate( pAdapter, band, path, txNum, vht1ssRates[i], value - base );
				}

				// VHT 2SS
				base = rtl8188fu_get_tx_power_by_rate( pAdapter, band, path, txNum, MGN_VHT2SS_MCS7 );
				for ( i = 0; i < sizeof( vht2ssRates ); ++i )
				{
					value = rtl8188fu_get_tx_power_by_rate( pAdapter, band, path, txNum, vht2ssRates[i] );
					_rtl8188fu_phy_set_tx_power_by_rate( pAdapter, band, path, txNum, vht2ssRates[i], value - base );
				}

				// VHT 3SS
				base = rtl8188fu_get_tx_power_by_rate( pAdapter, band, path, txNum, MGN_VHT3SS_MCS7 );
				for ( i = 0; i < sizeof( vht3ssRates ); ++i )
				{
					value = rtl8188fu_get_tx_power_by_rate( pAdapter, band, path, txNum, vht3ssRates[i] );
					_rtl8188fu_phy_set_tx_power_by_rate( pAdapter, band, path, txNum, vht3ssRates[i], value - base );
				}
			}
		}
	}

	//DBG_871X("<===PHY_ConvertTxPowerByRateInDbmToRelativeValues()\n" );
}

/*
  * This function must be called if the value in the PHY_REG_PG.txt(or header)
  * is exact dBm values
  */
VOID
PHY_TxPowerByRateConfiguration(
	IN  PADAPTER			pAdapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA( pAdapter);

	rtl8188fu_phy_store_txpower_by_rate_base( pAdapter );
	rtl8188fu_phy_convert_txpower_dbm_to_relative_value( pAdapter );
}

VOID 
rtl8188fu_phy_set_txpower_index_by_rate_section(PADAPTER pAdapter,
						u8 RFPath,
						u8 Channel,
						u8 RateSection)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(pAdapter);

	if ( RateSection == CCK )
	{
		u8	cckRates[]   = {MGN_1M, MGN_2M, MGN_5_5M, MGN_11M};
		if ( pHalData->CurrentBandType == BAND_ON_2_4G )
			rtl8188fu_phy_set_txpower_index_by_rate_array( pAdapter, RFPath, pHalData->CurrentChannelBW, Channel,
									  cckRates, sizeof(cckRates)/sizeof(u8) );
			
	}
	else if ( RateSection == OFDM )
	{
		u8	ofdmRates[]  = {MGN_6M, MGN_9M, MGN_12M, MGN_18M, MGN_24M, MGN_36M, MGN_48M, MGN_54M};
		rtl8188fu_phy_set_txpower_index_by_rate_array( pAdapter, RFPath, pHalData->CurrentChannelBW, Channel,
									 ofdmRates, sizeof(ofdmRates)/sizeof(u8));
		
	}
	else if ( RateSection == HT_MCS0_MCS7 )
	{
		u8	htRates1T[]  = {MGN_MCS0, MGN_MCS1, MGN_MCS2, MGN_MCS3, MGN_MCS4, MGN_MCS5, MGN_MCS6, MGN_MCS7};
		rtl8188fu_phy_set_txpower_index_by_rate_array( pAdapter, RFPath, pHalData->CurrentChannelBW, Channel,
									 htRates1T, sizeof(htRates1T)/sizeof(u8));

	}
	else if ( RateSection == HT_MCS8_MCS15 )
	{
		u8	htRates2T[]  = {MGN_MCS8, MGN_MCS9, MGN_MCS10, MGN_MCS11, MGN_MCS12, MGN_MCS13, MGN_MCS14, MGN_MCS15};
		rtl8188fu_phy_set_txpower_index_by_rate_array( pAdapter, RFPath, pHalData->CurrentChannelBW, Channel,
								  	 htRates2T, sizeof(htRates2T)/sizeof(u8));
		
	}
	else
	{
		DBG_871X("Invalid RateSection %d in %s", RateSection, __FUNCTION__ );
	}
}

BOOLEAN 
phy_GetChnlIndex(
	IN	u8 	Channel,
	OUT u8*	ChannelIdx
	)
{
	u8  i = 0;
	BOOLEAN bIn24G=_TRUE;

	if (Channel <= 14) {
		bIn24G = _TRUE;
		*ChannelIdx = Channel - 1;
	} else {
		bIn24G = _FALSE;	

		for (i = 0; i < CENTER_CH_5G_ALL_NUM; ++i) {
			if (center_ch_5g_all[i] == Channel) {
				*ChannelIdx = i;
				return bIn24G;
			}
		}
	}

	return bIn24G;
}

u8 rtl8188fu_phy_get_txpower_index_base(
	IN	PADAPTER		pAdapter,
	IN	u8				RFPath,
	IN	u8				Rate,	
	IN	CHANNEL_WIDTH	BandWidth,	
	IN	u8				Channel,
	OUT PBOOLEAN		bIn24G
	)
{
	PHAL_DATA_TYPE		pHalData = GET_HAL_DATA(pAdapter);
	PDM_ODM_T			pDM_Odm = &pHalData->odmpriv;
	u8					i = 0;	//default set to 1S
	u8					txPower = 0;
	u8					chnlIdx = (Channel-1);
	
	if (HAL_IsLegalChannel(pAdapter, Channel) == _FALSE)
	{
		chnlIdx = 0;
		DBG_871X("Illegal channel!!\n");
	}

	*bIn24G = phy_GetChnlIndex(Channel, &chnlIdx);

	//DBG_871X("[%s] Channel Index: %d\n", (*bIn24G?"2.4G":"5G"), chnlIdx);

	if (*bIn24G) //3 ============================== 2.4 G ==============================
	{
		if ( IS_CCK_RATE(Rate) )
		{
			txPower = pHalData->Index24G_CCK_Base[RFPath][chnlIdx];	
		}
		else if ( MGN_6M <= Rate )
		{				
			txPower = pHalData->Index24G_BW40_Base[RFPath][chnlIdx];
		}
		else
		{
			DBG_871X("PHY_GetTxPowerIndexBase: INVALID Rate.\n");
		}

		//DBG_871X("Base Tx power(RF-%c, Rate #%d, Channel Index %d) = 0x%X\n", 
		//		((RFPath==0)?'A':'B'), Rate, chnlIdx, txPower);
		
		// OFDM-1T
		if ( (MGN_6M <= Rate && Rate <= MGN_54M) && ! IS_CCK_RATE(Rate) )
		{
			txPower += pHalData->OFDM_24G_Diff[RFPath][TX_1S];
			//DBG_871X("+PowerDiff 2.4G (RF-%c): (OFDM-1T) = (%d)\n", ((RFPath==0)?'A':'B'), pHalData->OFDM_24G_Diff[RFPath][TX_1S]);
		}
		// BW20-1S, BW20-2S
		if (BandWidth == CHANNEL_WIDTH_20)
		{
			if ( (MGN_MCS0 <= Rate && Rate <= MGN_MCS31) || (MGN_VHT1SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW20_24G_Diff[RFPath][TX_1S];
			if ( (MGN_MCS8 <= Rate && Rate <= MGN_MCS31) || (MGN_VHT2SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW20_24G_Diff[RFPath][TX_2S];
			if ( (MGN_MCS16 <= Rate && Rate <= MGN_MCS31) || (MGN_VHT3SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW20_24G_Diff[RFPath][TX_3S];
			if ( (MGN_MCS24 <= Rate && Rate <= MGN_MCS31) || (MGN_VHT4SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW20_24G_Diff[RFPath][TX_4S];

			//DBG_871X("+PowerDiff 2.4G (RF-%c): (BW20-1S, BW20-2S, BW20-3S, BW20-4S) = (%d, %d, %d, %d)\n", ((RFPath==0)?'A':(RFPath==1)?'B':(RFPath==2)?'C':'D'), 
			//	pHalData->BW20_24G_Diff[RFPath][TX_1S], pHalData->BW20_24G_Diff[RFPath][TX_2S], 
			//	pHalData->BW20_24G_Diff[RFPath][TX_3S], pHalData->BW20_24G_Diff[RFPath][TX_4S]);
		}
		// BW40-1S, BW40-2S
		else if (BandWidth == CHANNEL_WIDTH_40)
		{
			if ( (MGN_MCS0 <= Rate && Rate <= MGN_MCS31) || (MGN_VHT1SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW40_24G_Diff[RFPath][TX_1S];
			if ( (MGN_MCS8 <= Rate && Rate <= MGN_MCS31) || (MGN_VHT2SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW40_24G_Diff[RFPath][TX_2S];
			if ( (MGN_MCS16 <= Rate && Rate <= MGN_MCS31) || (MGN_VHT3SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW40_24G_Diff[RFPath][TX_3S];
			if ( (MGN_MCS24 <= Rate && Rate <= MGN_MCS31) || (MGN_VHT4SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW40_24G_Diff[RFPath][TX_4S];			 

			//DBG_871X("+PowerDiff 2.4G (RF-%c): (BW40-1S, BW40-2S, BW40-3S, BW40-4S) = (%d, %d, %d, %d)\n", ((RFPath==0)?'A':(RFPath==1)?'B':(RFPath==2)?'C':'D'), 
			//	pHalData->BW40_24G_Diff[RFPath][TX_1S], pHalData->BW40_24G_Diff[RFPath][TX_2S],
			//	pHalData->BW40_24G_Diff[RFPath][TX_3S], pHalData->BW40_24G_Diff[RFPath][TX_4S]);
		}
		// Willis suggest adopt BW 40M power index while in BW 80 mode
		else if ( BandWidth == CHANNEL_WIDTH_80 )
		{
			if ( (MGN_MCS0 <= Rate && Rate <= MGN_MCS31) || (MGN_VHT1SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW40_24G_Diff[RFPath][TX_1S];
			if ( (MGN_MCS8 <= Rate && Rate <= MGN_MCS31) || (MGN_VHT2SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW40_24G_Diff[RFPath][TX_2S];
			if ( (MGN_MCS16 <= Rate && Rate <= MGN_MCS31) || (MGN_VHT3SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW40_24G_Diff[RFPath][TX_3S];
			if ( (MGN_MCS24 <= Rate && Rate <= MGN_MCS31) || (MGN_VHT4SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW40_24G_Diff[RFPath][TX_4S];

			//DBG_871X("+PowerDiff 2.4G (RF-%c): (BW40-1S, BW40-2S, BW40-3S, BW40-4T) = (%d, %d, %d, %d) P.S. Current is in BW 80MHz\n", ((RFPath==0)?'A':(RFPath==1)?'B':(RFPath==2)?'C':'D'), 
			//	pHalData->BW40_24G_Diff[RFPath][TX_1S], pHalData->BW40_24G_Diff[RFPath][TX_2S],
			//	pHalData->BW40_24G_Diff[RFPath][TX_3S], pHalData->BW40_24G_Diff[RFPath][TX_4S]);
		}
	}
	else //3 ============================== 5 G ==============================
	{
		if ( MGN_6M <= Rate )
		{				
			txPower = pHalData->Index5G_BW40_Base[RFPath][chnlIdx];
		}
		else
		{
			DBG_871X("===> mpt_ProQueryCalTxPower_Jaguar: INVALID Rate.\n");
		}

		//DBG_871X("Base Tx power(RF-%c, Rate #%d, Channel Index %d) = 0x%X\n", 
		//	((RFPath==0)?'A':'B'), Rate, chnlIdx, txPower);

		// OFDM-1T
		if ( (MGN_6M <= Rate && Rate <= MGN_54M) && ! IS_CCK_RATE(Rate))
		{
			txPower += pHalData->OFDM_5G_Diff[RFPath][TX_1S];
			//DBG_871X("+PowerDiff 5G (RF-%c): (OFDM-1T) = (%d)\n", ((RFPath==0)?'A':'B'), pHalData->OFDM_5G_Diff[RFPath][TX_1S]);
		}
		
		// BW20-1S, BW20-2S
		if (BandWidth == CHANNEL_WIDTH_20)
		{
			if ( (MGN_MCS0 <= Rate && Rate <= MGN_MCS31)  || (MGN_VHT1SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW20_5G_Diff[RFPath][TX_1S];
			if ( (MGN_MCS8 <= Rate && Rate <= MGN_MCS31) || (MGN_VHT2SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW20_5G_Diff[RFPath][TX_2S];
			if ( (MGN_MCS16 <= Rate && Rate <= MGN_MCS31) || (MGN_VHT3SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW20_5G_Diff[RFPath][TX_3S];
			if ( (MGN_MCS24 <= Rate && Rate <= MGN_MCS31) || (MGN_VHT4SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW20_5G_Diff[RFPath][TX_4S];

			//DBG_871X("+PowerDiff 5G (RF-%c): (BW20-1S, BW20-2S, BW20-3S, BW20-4S) = (%d, %d, %d, %d)\n", ((RFPath==0)?'A':(RFPath==1)?'B':(RFPath==2)?'C':'D'), 
			//	pHalData->BW20_5G_Diff[RFPath][TX_1S], pHalData->BW20_5G_Diff[RFPath][TX_2S],
			//	pHalData->BW20_5G_Diff[RFPath][TX_3S], pHalData->BW20_5G_Diff[RFPath][TX_4S]);
		}
		// BW40-1S, BW40-2S
		else if (BandWidth == CHANNEL_WIDTH_40)
		{
			if ( (MGN_MCS0 <= Rate && Rate <= MGN_MCS31)  || (MGN_VHT1SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW40_5G_Diff[RFPath][TX_1S];
			if ( (MGN_MCS8 <= Rate && Rate <= MGN_MCS31) || (MGN_VHT2SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW40_5G_Diff[RFPath][TX_2S];
			if ( (MGN_MCS16 <= Rate && Rate <= MGN_MCS31) || (MGN_VHT3SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW40_5G_Diff[RFPath][TX_3S];
			if ( (MGN_MCS24 <= Rate && Rate <= MGN_MCS31) || (MGN_VHT4SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW40_5G_Diff[RFPath][TX_4S];

			//DBG_871X("+PowerDiff 5G(RF-%c): (BW40-1S, BW40-2S) = (%d, %d, %d, %d)\n", ((RFPath==0)?'A':(RFPath==1)?'B':(RFPath==2)?'C':'D'), 
			//	pHalData->BW40_5G_Diff[RFPath][TX_1S], pHalData->BW40_5G_Diff[RFPath][TX_2S],
			//	pHalData->BW40_5G_Diff[RFPath][TX_3S], pHalData->BW40_5G_Diff[RFPath][TX_4S]);
		}
		// BW80-1S, BW80-2S
		else if (BandWidth== CHANNEL_WIDTH_80)
		{
			// <20121220, Kordan> Get the index of array "Index5G_BW80_Base".
			for (i = 0; i < CENTER_CH_5G_80M_NUM; ++i)
				if (center_ch_5g_80m[i] == Channel)
					chnlIdx = i;

			txPower = pHalData->Index5G_BW80_Base[RFPath][chnlIdx];

			if ( (MGN_MCS0 <= Rate && Rate <= MGN_MCS31)  || (MGN_VHT1SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += + pHalData->BW80_5G_Diff[RFPath][TX_1S];
			if ( (MGN_MCS8 <= Rate && Rate <= MGN_MCS31) || (MGN_VHT2SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW80_5G_Diff[RFPath][TX_2S];
			if ( (MGN_MCS16 <= Rate && Rate <= MGN_MCS31) || (MGN_VHT3SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW80_5G_Diff[RFPath][TX_3S];
			if ( (MGN_MCS23 <= Rate && Rate <= MGN_MCS31) || (MGN_VHT4SS_MCS0 <= Rate && Rate <= MGN_VHT4SS_MCS9))
				txPower += pHalData->BW80_5G_Diff[RFPath][TX_4S];

			//DBG_871X("+PowerDiff 5G(RF-%c): (BW80-1S, BW80-2S, BW80-3S, BW80-4S) = (%d, %d, %d, %d)\n",((RFPath==0)?'A':(RFPath==1)?'B':(RFPath==2)?'C':'D'), 
			//	pHalData->BW80_5G_Diff[RFPath][TX_1S], pHalData->BW80_5G_Diff[RFPath][TX_2S],
			//	pHalData->BW80_5G_Diff[RFPath][TX_3S], pHalData->BW80_5G_Diff[RFPath][TX_4S]);
		}
	}

	return txPower;	
}

s8
rtl8188fu_phy_get_tx_power_tracking_offset(
	PADAPTER	pAdapter,
	u8			RFPath,
	u8			Rate
	)
{
	PHAL_DATA_TYPE		pHalData = GET_HAL_DATA(pAdapter);
	PDM_ODM_T			pDM_Odm = &pHalData->odmpriv;	
	s8	offset = 0;
	
	if( pDM_Odm->RFCalibrateInfo.TxPowerTrackControl  == _FALSE)
		return offset;
	
	if ((Rate == MGN_1M) ||(Rate == MGN_2M)||(Rate == MGN_5_5M)||(Rate == MGN_11M))
	{ 
		offset = pDM_Odm->RFCalibrateInfo.remnant_cck_idx;
		/*DBG_871X("+Remnant_CCKSwingIdx = 0x%x\n", RFPath, Rate, pRFCalibrateInfo->Remnant_CCKSwingIdx);*/
	}
	else
	{
		offset = pDM_Odm->RFCalibrateInfo.remnant_ofdm_swing_idx[RFPath];
		/*DBG_871X("+Remanant_OFDMSwingIdx[RFPath %u][Rate 0x%x] = 0x%x\n", RFPath, Rate, pRFCalibrateInfo->Remnant_OFDMSwingIdx[RFPath]);	*/	
		
	}

	return offset;
}

u8
PHY_GetRateIndexOfTxPowerByRate(
	IN	u8		Rate
	)
{
	u8	index = 0;
	switch ( Rate )
	{
		case MGN_1M: index = 0; break;
		case MGN_2M: index = 1; break;
		case MGN_5_5M: index = 2; break;
		case MGN_11M: index = 3; break;
		case MGN_6M: index = 4; break;
		case MGN_9M: index = 5; break;
		case MGN_12M: index = 6; break;
		case MGN_18M: index = 7; break;
		case MGN_24M: index = 8; break;
		case MGN_36M: index = 9; break;
		case MGN_48M: index = 10; break;
		case MGN_54M: index = 11; break;
		case MGN_MCS0: index = 12; break;
		case MGN_MCS1: index = 13; break;
		case MGN_MCS2: index = 14; break;
		case MGN_MCS3: index = 15; break;
		case MGN_MCS4: index = 16; break;
		case MGN_MCS5: index = 17; break;
		case MGN_MCS6: index = 18; break;
		case MGN_MCS7: index = 19; break;
		case MGN_MCS8: index = 20; break;
		case MGN_MCS9: index = 21; break;
		case MGN_MCS10: index = 22; break;
		case MGN_MCS11: index = 23; break;
		case MGN_MCS12: index = 24; break;
		case MGN_MCS13: index = 25; break;
		case MGN_MCS14: index = 26; break;
		case MGN_MCS15: index = 27; break;
		case MGN_MCS16: index = 28; break;
		case MGN_MCS17: index = 29; break;
		case MGN_MCS18: index = 30; break;
		case MGN_MCS19: index = 31; break;
		case MGN_MCS20: index = 32; break;
		case MGN_MCS21: index = 33; break;
		case MGN_MCS22: index = 34; break;
		case MGN_MCS23: index = 35; break;
		case MGN_MCS24: index = 36; break;
		case MGN_MCS25: index = 37; break;
		case MGN_MCS26: index = 38; break;
		case MGN_MCS27: index = 39; break;
		case MGN_MCS28: index = 40; break;
		case MGN_MCS29: index = 41; break;
		case MGN_MCS30: index = 42; break;
		case MGN_MCS31: index = 43; break;
		case MGN_VHT1SS_MCS0: index = 44; break;
		case MGN_VHT1SS_MCS1: index = 45; break;
		case MGN_VHT1SS_MCS2: index = 46; break;
		case MGN_VHT1SS_MCS3: index = 47; break;
		case MGN_VHT1SS_MCS4: index = 48; break;
		case MGN_VHT1SS_MCS5: index = 49; break;
		case MGN_VHT1SS_MCS6: index = 50; break;
		case MGN_VHT1SS_MCS7: index = 51; break;
		case MGN_VHT1SS_MCS8: index = 52; break;
		case MGN_VHT1SS_MCS9: index = 53; break;
		case MGN_VHT2SS_MCS0: index = 54; break;
		case MGN_VHT2SS_MCS1: index = 55; break;
		case MGN_VHT2SS_MCS2: index = 56; break;
		case MGN_VHT2SS_MCS3: index = 57; break;
		case MGN_VHT2SS_MCS4: index = 58; break;
		case MGN_VHT2SS_MCS5: index = 59; break;
		case MGN_VHT2SS_MCS6: index = 60; break;
		case MGN_VHT2SS_MCS7: index = 61; break;
		case MGN_VHT2SS_MCS8: index = 62; break;
		case MGN_VHT2SS_MCS9: index = 63; break;
		case MGN_VHT3SS_MCS0: index = 64; break;
		case MGN_VHT3SS_MCS1: index = 65; break;
		case MGN_VHT3SS_MCS2: index = 66; break;
		case MGN_VHT3SS_MCS3: index = 67; break;
		case MGN_VHT3SS_MCS4: index = 68; break;
		case MGN_VHT3SS_MCS5: index = 69; break;
		case MGN_VHT3SS_MCS6: index = 70; break;
		case MGN_VHT3SS_MCS7: index = 71; break;
		case MGN_VHT3SS_MCS8: index = 72; break;
		case MGN_VHT3SS_MCS9: index = 73; break;
		case MGN_VHT4SS_MCS0: index = 74; break;
		case MGN_VHT4SS_MCS1: index = 75; break;
		case MGN_VHT4SS_MCS2: index = 76; break;
		case MGN_VHT4SS_MCS3: index = 77; break;
		case MGN_VHT4SS_MCS4: index = 78; break;
		case MGN_VHT4SS_MCS5: index = 79; break;
		case MGN_VHT4SS_MCS6: index = 80; break;
		case MGN_VHT4SS_MCS7: index = 81; break;
		case MGN_VHT4SS_MCS8: index = 82; break;
		case MGN_VHT4SS_MCS9: index = 83; break;
		default:
			DBG_871X("Invalid rate 0x%x in %s\n", Rate, __FUNCTION__ );
			break;
	};

	return index;
}

s8
_rtl8188fu_get_tx_power_by_rate(
	IN	PADAPTER	pAdapter, 
	IN	u8			Band, 
	IN	u8			RFPath, 
	IN	u8			TxNum, 
	IN	u8			Rate
	)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);
	s8 value = 0;
	u8 rateIndex = PHY_GetRateIndexOfTxPowerByRate(Rate);

	if (Band != BAND_ON_2_4G && Band != BAND_ON_5G) {
		DBG_871X("Invalid band %d in %s\n", Band, __func__);
		goto exit;
	}
	if (RFPath > ODM_RF_PATH_D) {
		DBG_871X("Invalid RfPath %d in %s\n", RFPath, __func__);
		goto exit;
	}
	if (TxNum >= RF_MAX_TX_NUM) {
		DBG_871X("Invalid TxNum %d in %s\n", TxNum, __func__);
		goto exit;
	}
	if (rateIndex >= TX_PWR_BY_RATE_NUM_RATE) {
		DBG_871X("Invalid RateIndex %d in %s\n", rateIndex, __func__);
		goto exit;
	}

	value = pHalData->tx_power_by_rate_offset[Band][RFPath][TxNum][rateIndex];

exit:
	return value;
}


s8
rtl8188fu_get_tx_power_by_rate(
	IN	PADAPTER	pAdapter,
	IN	u8			Band,
	IN	u8			RFPath,
	IN	u8			TxNum,
	IN	u8			Rate
	)
{
	if (!phy_is_tx_power_by_rate_needed(pAdapter))
		return 0;

	return _rtl8188fu_get_tx_power_by_rate(pAdapter, Band, RFPath, TxNum, Rate);
}


void rtl8188fu_phy_set_txpower_level(
	IN	PADAPTER	Adapter,
	IN	u8			channel
	)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	BOOLEAN bIsIn24G = (pHalData->CurrentBandType == BAND_ON_2_4G );
	u8 path = ODM_RF_PATH_A;

	//if ( pMgntInfo->RegNByteAccess == 0 )
	{
		if ( bIsIn24G )
			rtl8188fu_phy_set_txpower_index_by_rate_section( Adapter, path, channel, CCK );
		
		rtl8188fu_phy_set_txpower_index_by_rate_section( Adapter, path, channel, OFDM );
		rtl8188fu_phy_set_txpower_index_by_rate_section( Adapter, path, channel, HT_MCS0_MCS7 );

		if (pHalData->NumTotalRFPath >= 2) {
			rtl8188fu_phy_set_txpower_index_by_rate_section( Adapter, path, channel, HT_MCS8_MCS15 );
		}
	}
}

VOID
rtl8188fu_phy_set_txpower_index_by_rate_array(
	IN	PADAPTER			pAdapter,
	IN 	u8					RFPath,
	IN	CHANNEL_WIDTH		BandWidth,	
	IN	u8					Channel,
	IN	u8*					Rates,
	IN	u8					RateArraySize
	)
{
	u32	powerIndex = 0;
	int	i = 0;

	for (i = 0; i < RateArraySize; ++i) {
		powerIndex = rtl8188fu_phy_get_txpower_index(pAdapter, RFPath, Rates[i], BandWidth, Channel);
		rtl8188fu_phy_set_txpower_index(pAdapter, powerIndex, RFPath, Rates[i]);
	}
}

s8
phy_GetWorldWideLimit(
	s8* LimitTable
)
{
	s8	min = LimitTable[0];
	u8	i = 0;
	
	for (i = 0; i < MAX_REGULATION_NUM; ++i) {
		if (LimitTable[i] < min)
			min = LimitTable[i];
	}

	return min;
}

s8
phy_GetChannelIndexOfTxPowerLimit(
	IN	u8			Band,
	IN	u8			Channel
	)
{
	s8	channelIndex = -1;
	u8	i = 0;

	if (Band == BAND_ON_2_4G) {
		channelIndex = Channel - 1;
	} else if (Band == BAND_ON_5G) {
		for (i = 0; i < CENTER_CH_5G_ALL_NUM; ++i) {
			if (center_ch_5g_all[i] == Channel)
				channelIndex = i;
		}
	} else {
		DBG_871X_LEVEL(_drv_always_, "Invalid Band %d in %s\n", Band, __func__);
	}

	if (channelIndex == -1)
		DBG_871X_LEVEL(_drv_always_, "Invalid Channel %d of Band %d in %s\n", Channel, Band, __func__);

	return channelIndex;
}

s8
rtl8188fu_phy_get_txpower_limit(
	IN	PADAPTER			Adapter,
	IN	u32					RegPwrTblSel,
	IN	BAND_TYPE			Band,
	IN	CHANNEL_WIDTH		Bandwidth,
	IN	u8					RfPath,
	IN	u8					DataRate,
	IN	u8					Channel
	)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	s16				band = -1, regulation = -1, bandwidth = -1,
					rateSection = -1, channel = -1;
	s8				powerLimit = MAX_POWER_INDEX;

	if ( ( Adapter->registrypriv.RegEnableTxPowerLimit == 2 && pHalData->EEPROMRegulatory != 1 ) || 
		   Adapter->registrypriv.RegEnableTxPowerLimit == 0 )
		return MAX_POWER_INDEX;

	switch (RegPwrTblSel) {
	case 1:
		regulation = TXPWR_LMT_ETSI;
		break;
	case 2:
		regulation = TXPWR_LMT_MKK;
		break;
	case 3:
		regulation = TXPWR_LMT_FCC;
		break;
	case 4:
		regulation = TXPWR_LMT_WW;
		break;
	default:
		regulation = (Band == BAND_ON_2_4G) ? pHalData->Regulation2_4G : pHalData->Regulation5G;
		break;
	}

	//DBG_871X("pMgntInfo->RegPwrTblSel %d, final regulation %d\n", Adapter->registrypriv.RegPwrTblSel, regulation );

	
	if ( Band == BAND_ON_2_4G ) band = 0; 
	else if ( Band == BAND_ON_5G ) band = 1; 

	if ( Bandwidth == CHANNEL_WIDTH_20 ) bandwidth = 0;
	else if ( Bandwidth == CHANNEL_WIDTH_40 ) bandwidth = 1;
	else if ( Bandwidth == CHANNEL_WIDTH_80 ) bandwidth = 2;
	else if ( Bandwidth == CHANNEL_WIDTH_160 ) bandwidth = 3;

	switch ( DataRate )
	{
		case MGN_1M: case MGN_2M: case MGN_5_5M: case MGN_11M:
			rateSection = 0;
			break;

		case MGN_6M: case MGN_9M: case MGN_12M: case MGN_18M:
		case MGN_24M: case MGN_36M: case MGN_48M: case MGN_54M:
			rateSection = 1;
			break;

		case MGN_MCS0: case MGN_MCS1: case MGN_MCS2: case MGN_MCS3: 
		case MGN_MCS4: case MGN_MCS5: case MGN_MCS6: case MGN_MCS7:
			rateSection = 2;
			break;
			
		case MGN_MCS8: case MGN_MCS9: case MGN_MCS10: case MGN_MCS11: 
		case MGN_MCS12: case MGN_MCS13: case MGN_MCS14: case MGN_MCS15:
			rateSection = 3;
			break;

		case MGN_MCS16: case MGN_MCS17: case MGN_MCS18: case MGN_MCS19: 
		case MGN_MCS20: case MGN_MCS21: case MGN_MCS22: case MGN_MCS23:
			rateSection = 4;
			break;

		case MGN_MCS24: case MGN_MCS25: case MGN_MCS26: case MGN_MCS27: 
		case MGN_MCS28: case MGN_MCS29: case MGN_MCS30: case MGN_MCS31:
			rateSection = 5;
			break;

		case MGN_VHT1SS_MCS0: case MGN_VHT1SS_MCS1: case MGN_VHT1SS_MCS2:
		case MGN_VHT1SS_MCS3: case MGN_VHT1SS_MCS4: case MGN_VHT1SS_MCS5:
		case MGN_VHT1SS_MCS6: case MGN_VHT1SS_MCS7: case MGN_VHT1SS_MCS8:
		case MGN_VHT1SS_MCS9:
			rateSection = 6;
			break;
			
		case MGN_VHT2SS_MCS0: case MGN_VHT2SS_MCS1: case MGN_VHT2SS_MCS2:
		case MGN_VHT2SS_MCS3: case MGN_VHT2SS_MCS4: case MGN_VHT2SS_MCS5:
		case MGN_VHT2SS_MCS6: case MGN_VHT2SS_MCS7: case MGN_VHT2SS_MCS8:
		case MGN_VHT2SS_MCS9:
			rateSection = 7;
			break;

		case MGN_VHT3SS_MCS0: case MGN_VHT3SS_MCS1: case MGN_VHT3SS_MCS2:
		case MGN_VHT3SS_MCS3: case MGN_VHT3SS_MCS4: case MGN_VHT3SS_MCS5:
		case MGN_VHT3SS_MCS6: case MGN_VHT3SS_MCS7: case MGN_VHT3SS_MCS8:
		case MGN_VHT3SS_MCS9:
			rateSection = 8;
			break;

		case MGN_VHT4SS_MCS0: case MGN_VHT4SS_MCS1: case MGN_VHT4SS_MCS2:
		case MGN_VHT4SS_MCS3: case MGN_VHT4SS_MCS4: case MGN_VHT4SS_MCS5:
		case MGN_VHT4SS_MCS6: case MGN_VHT4SS_MCS7: case MGN_VHT4SS_MCS8:
		case MGN_VHT4SS_MCS9:
			rateSection = 9;
			break;

		default:
			DBG_871X("Wrong rate 0x%x\n", DataRate );
			break;
	}

	if ( Band == BAND_ON_5G  && rateSection == 0 )
			DBG_871X("Wrong rate 0x%x: No CCK in 5G Band\n", DataRate );

	// workaround for wrong index combination to obtain tx power limit, 
	// OFDM only exists in BW 20M
	if ( rateSection == 1 )
		bandwidth = 0;

	// workaround for wrong index combination to obtain tx power limit, 
	// CCK table will only be given in BW 20M
	if ( rateSection == 0 )
		bandwidth = 0;

	// workaround for wrong indxe combination to obtain tx power limit, 
	// HT on 80M will reference to HT on 40M
	if ( ( rateSection == 2 || rateSection == 3 ) && Band == BAND_ON_5G && bandwidth == 2 ) {
		bandwidth = 1;
	}
	
	if ( Band == BAND_ON_2_4G )
		channel = phy_GetChannelIndexOfTxPowerLimit( BAND_ON_2_4G, Channel );
	else if ( Band == BAND_ON_5G )
		channel = phy_GetChannelIndexOfTxPowerLimit( BAND_ON_5G, Channel );
	else if ( Band == BAND_ON_BOTH )
	{
		// BAND_ON_BOTH don't care temporarily 
	}
	
	if ( band == -1 || regulation == -1 || bandwidth == -1 || 
	     rateSection == -1 || channel == -1 )
	{
		//DBG_871X("Wrong index value to access power limit table [band %d][regulation %d][bandwidth %d][rf_path %d][rate_section %d][chnlGroup %d]\n",
		//	  band, regulation, bandwidth, RfPath, rateSection, channelGroup );

		return MAX_POWER_INDEX;
	}

	if ( Band == BAND_ON_2_4G ) {
		s8 limits[10] = {0}; u8 i = 0;
		if (bandwidth >= MAX_2_4G_BANDWIDTH_NUM)
			bandwidth = MAX_2_4G_BANDWIDTH_NUM - 1;
		for (i = 0; i < MAX_REGULATION_NUM; ++i)
			limits[i] = pHalData->TxPwrLimit_2_4G[i][bandwidth][rateSection][channel][RfPath]; 

		powerLimit = (regulation == TXPWR_LMT_WW) ? phy_GetWorldWideLimit(limits) :
			          pHalData->TxPwrLimit_2_4G[regulation][bandwidth][rateSection][channel][RfPath];

	} else if ( Band == BAND_ON_5G ) {
		s8 limits[10] = {0}; u8 i = 0;
		for (i = 0; i < MAX_REGULATION_NUM; ++i)
			limits[i] = pHalData->TxPwrLimit_5G[i][bandwidth][rateSection][channel][RfPath];
		
		powerLimit = (regulation == TXPWR_LMT_WW) ? phy_GetWorldWideLimit(limits) : 
					  pHalData->TxPwrLimit_5G[regulation][bandwidth][rateSection][channel][RfPath];
	} else 
		DBG_871X("No power limit table of the specified band\n" );

	// combine 5G VHT & HT rate
	// 5G 20M and 40M HT and VHT can cross reference
	/*
	if ( Band == BAND_ON_5G && powerLimit == MAX_POWER_INDEX ) {
		if ( bandwidth == 0 || bandwidth == 1 ) { 
			RT_TRACE( COMP_INIT, DBG_LOUD, ( "No power limit table of the specified band %d, bandwidth %d, ratesection %d, rf path %d\n", 
					  band, bandwidth, rateSection, RfPath ) );
			if ( rateSection == 2 )
				powerLimit = pHalData->TxPwrLimit_5G[regulation]
										[bandwidth][4][channelGroup][RfPath];
			else if ( rateSection == 4 )
				powerLimit = pHalData->TxPwrLimit_5G[regulation]
										[bandwidth][2][channelGroup][RfPath];
			else if ( rateSection == 3 )
				powerLimit = pHalData->TxPwrLimit_5G[regulation]
										[bandwidth][5][channelGroup][RfPath];
			else if ( rateSection == 5 )
				powerLimit = pHalData->TxPwrLimit_5G[regulation]
										[bandwidth][3][channelGroup][RfPath];
		}
	}
	*/
	//DBG_871X("TxPwrLmt[Regulation %d][Band %d][BW %d][RFPath %d][Rate 0x%x][Chnl %d] = %d\n", 
	//		regulation, pHalData->CurrentBandType, Bandwidth, RfPath, DataRate, Channel, powerLimit);
	return powerLimit;
}

VOID
phy_CrossReferenceHTAndVHTTxPowerLimit(
	IN	PADAPTER			pAdapter
	)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);
	u8 regulation, bw, channel, rs, ref_rs;
	int ht_ref_vht_5g_20_40 = 0;
	int vht_ref_ht_5g_20_40 = 0;
	int ht_has_ref_5g_20_40 = 0;
	int vht_has_ref_5g_20_40 = 0;

	pHalData->tx_pwr_lmt_5g_20_40_ref = 0;

	for (regulation = 0; regulation < MAX_REGULATION_NUM; ++regulation) {

		for (bw = 0; bw < MAX_5G_BANDWIDTH_NUM; ++bw) {

			for (channel = 0; channel < CENTER_CH_5G_ALL_NUM; ++channel) {

				for (rs = 0; rs < MAX_RATE_SECTION_NUM; ++rs) {

					/* 5G 20M 40M VHT and HT can cross reference */
					if (bw == CHANNEL_WIDTH_20 || bw == CHANNEL_WIDTH_40) {
						if (rs == HT_1SS)
							ref_rs = VHT_1SS;
						else if (rs == HT_2SS)
							ref_rs = VHT_2SS;
						else if (rs == HT_3SS)
							ref_rs = VHT_3SS;
						else if (rs == HT_4SS)
							ref_rs = VHT_4SS;
						else if (rs == VHT_1SS)
							ref_rs = HT_1SS;
						else if (rs == VHT_2SS)
							ref_rs = HT_2SS;
						else if (rs == VHT_3SS)
							ref_rs = HT_3SS;
						else if (rs == VHT_4SS)
							ref_rs = HT_4SS;
						else
							continue;

						if (pHalData->TxPwrLimit_5G[regulation][bw][ref_rs][channel][RF_PATH_A] == MAX_POWER_INDEX)
							continue;

						if (IS_HT_RATE_SECTION(rs))
							ht_has_ref_5g_20_40++;
						else if (IS_VHT_RATE_SECTION(rs))
							vht_has_ref_5g_20_40++;
						else
							continue;

						if (pHalData->TxPwrLimit_5G[regulation][bw][rs][channel][RF_PATH_A] != MAX_POWER_INDEX)
							continue;

						if (IS_HT_RATE_SECTION(rs) && IS_VHT_RATE_SECTION(ref_rs))
							ht_ref_vht_5g_20_40++;
						else if (IS_VHT_RATE_SECTION(rs) && IS_HT_RATE_SECTION(ref_rs))
							vht_ref_ht_5g_20_40++;

						if (0)
							DBG_871X("reg:%u, bw:%u, ch:%u, %s ref %s\n"
								, regulation, bw, channel
								, rate_section_str(rs), rate_section_str(ref_rs));

						pHalData->TxPwrLimit_5G[regulation][bw][rs][channel][RF_PATH_A] =
							pHalData->TxPwrLimit_5G[regulation][bw][ref_rs][channel][RF_PATH_A];
					}

				}
			}
		}
	}

	if (0) {
		DBG_871X("ht_ref_vht_5g_20_40:%d, ht_has_ref_5g_20_40:%d\n", ht_ref_vht_5g_20_40, ht_has_ref_5g_20_40);
		DBG_871X("vht_ref_hht_5g_20_40:%d, vht_has_ref_5g_20_40:%d\n", vht_ref_ht_5g_20_40, vht_has_ref_5g_20_40);
	}

	/* 5G 20M&40M HT all come from VHT*/
	if (ht_ref_vht_5g_20_40 && ht_has_ref_5g_20_40 == ht_ref_vht_5g_20_40)
		pHalData->tx_pwr_lmt_5g_20_40_ref |= TX_PWR_LMT_REF_HT_FROM_VHT;

	/* 5G 20M&40M VHT all come from HT*/
	if (vht_ref_ht_5g_20_40 && vht_has_ref_5g_20_40 == vht_ref_ht_5g_20_40)
		pHalData->tx_pwr_lmt_5g_20_40_ref |= TX_PWR_LMT_REF_VHT_FROM_HT;
}

VOID 
PHY_ConvertTxPowerLimitToPowerIndex(
	IN	PADAPTER			Adapter
	)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(Adapter);
	u8 base;
	u8 regulation, bw, channel, rateSection;
	s8 tempValue = 0, tempPwrLmt = 0;
	u8 rfPath = 0;

	if (pHalData->odmpriv.PhyRegPgValueType != PHY_REG_PG_EXACT_VALUE) {
		rtw_warn_on(1);
		return;
	}

	phy_CrossReferenceHTAndVHTTxPowerLimit(Adapter);

	for (regulation = 0; regulation < MAX_REGULATION_NUM; ++regulation) {

		for (bw = 0; bw < MAX_2_4G_BANDWIDTH_NUM; ++bw) {

			for (channel = 0; channel < CENTER_CH_2G_NUM; ++channel) {

				for (rateSection = CCK; rateSection <= HT_4SS; ++rateSection) {
					tempPwrLmt = pHalData->TxPwrLimit_2_4G[regulation][bw][rateSection][channel][RF_PATH_A];

					if (tempPwrLmt != MAX_POWER_INDEX) {

						for (rfPath = RF_PATH_A; rfPath < MAX_RF_PATH; ++rfPath) {
							base = phy_get_target_tx_power(Adapter, BAND_ON_2_4G, rfPath, rateSection);
							tempValue = tempPwrLmt - base;
							pHalData->TxPwrLimit_2_4G[regulation][bw][rateSection][channel][rfPath] = tempValue;
						}
					}
				}
			}
		}
	}
	
}

/*
* PHY_InitTxPowerLimit - Set all hal_data.TxPwrLimit_2_4G, TxPwrLimit_5G array to MAX_POWER_INDEX
*/
VOID
PHY_InitTxPowerLimit(
	IN	PADAPTER		Adapter
	)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(Adapter);
	u8 i, j, k, l, m;

	for (i = 0; i < MAX_REGULATION_NUM; ++i)
		for (j = 0; j < MAX_2_4G_BANDWIDTH_NUM; ++j)
			for (k = 0; k < MAX_RATE_SECTION_NUM; ++k)
				for (m = 0; m < CENTER_CH_2G_NUM; ++m)
					for (l = 0; l < MAX_RF_PATH; ++l)
						pHalData->TxPwrLimit_2_4G[i][j][k][m][l] = MAX_POWER_INDEX;

	for (i = 0; i < MAX_REGULATION_NUM; ++i)
		for (j = 0; j < MAX_5G_BANDWIDTH_NUM; ++j)
			for (k = 0; k < MAX_RATE_SECTION_NUM; ++k)
				for (m = 0; m < CENTER_CH_5G_ALL_NUM; ++m)
					for (l = 0; l < MAX_RF_PATH; ++l)
						pHalData->TxPwrLimit_5G[i][j][k][m][l] = MAX_POWER_INDEX;
}

/*
* PHY_SetTxPowerLimit - Parsing TX power limit from phydm array, called by odm_ConfigBB_TXPWR_LMT_XXX in phydm
*/
VOID
PHY_SetTxPowerLimit(
	IN	PDM_ODM_T		pDM_Odm,
	IN	u8				*Regulation,
	IN	u8				*Band,
	IN	u8				*Bandwidth,
	IN	u8				*RateSection,
	IN	u8				*RfPath,
	IN	u8				*Channel,
	IN	u8				*PowerLimit
	)
{
	PADAPTER Adapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(Adapter);
	u8 regulation = 0, bandwidth = 0, rateSection = 0, channel;
	s8 powerLimit = 0, prevPowerLimit, channelIndex;

	if (0)
		DBG_871X("Index of power limit table [band %s][regulation %s][bw %s][rate section %s][rf path %s][chnl %s][val %s]\n"
			, Band, Regulation, Bandwidth, RateSection, RfPath, Channel, PowerLimit);

	if (GetU1ByteIntegerFromStringInDecimal((s8 *)Channel, &channel) == _FALSE
		|| GetU1ByteIntegerFromStringInDecimal((s8 *)PowerLimit, &powerLimit) == _FALSE
	){
		DBG_871X_LEVEL(_drv_always_, "Illegal index of power limit table [ch %s][val %s]\n", Channel, PowerLimit);
		return;
	}

	powerLimit = powerLimit > MAX_POWER_INDEX ? MAX_POWER_INDEX : powerLimit;

	if (eqNByte(Regulation, (u8 *)("FCC"), 3))
		regulation = TXPWR_LMT_FCC;
	else if (eqNByte(Regulation, (u8 *)("MKK"), 3))
		regulation = TXPWR_LMT_MKK;
	else if (eqNByte(Regulation, (u8 *)("ETSI"), 4))
		regulation = TXPWR_LMT_ETSI;
	else if (eqNByte(Regulation, (u8 *)("WW13"), 4))
		regulation = TXPWR_LMT_WW;
	else {
		DBG_871X_LEVEL(_drv_always_, "unknown regulation:%s", Regulation);
		return;
	}

	if (eqNByte(RateSection, (u8 *)("CCK"), 3) && eqNByte(RfPath, (u8 *)("1T"), 2))
		rateSection = CCK;
	else if (eqNByte(RateSection, (u8 *)("OFDM"), 4) && eqNByte(RfPath, (u8 *)("1T"), 2))
		rateSection = OFDM;
	else if (eqNByte(RateSection, (u8 *)("HT"), 2) && eqNByte(RfPath, (u8 *)("1T"), 2))
		rateSection = HT_1SS;
	else if (eqNByte(RateSection, (u8 *)("HT"), 2) && eqNByte(RfPath, (u8 *)("2T"), 2))
		rateSection = HT_2SS;
	else if (eqNByte(RateSection, (u8 *)("HT"), 2) && eqNByte(RfPath, (u8 *)("3T"), 2))
		rateSection = HT_3SS;
	else if (eqNByte(RateSection, (u8 *)("HT"), 2) && eqNByte(RfPath, (u8 *)("4T"), 2))
		rateSection = HT_4SS;
	else if (eqNByte(RateSection, (u8 *)("VHT"), 3) && eqNByte(RfPath, (u8 *)("1T"), 2))
		rateSection = VHT_1SS;
	else if (eqNByte(RateSection, (u8 *)("VHT"), 3) && eqNByte(RfPath, (u8 *)("2T"), 2))
		rateSection = VHT_2SS;
	else if (eqNByte(RateSection, (u8 *)("VHT"), 3) && eqNByte(RfPath, (u8 *)("3T"), 2))
		rateSection = VHT_3SS;
	else if (eqNByte(RateSection, (u8 *)("VHT"), 3) && eqNByte(RfPath, (u8 *)("4T"), 2))
		rateSection = VHT_4SS;
	else {
		DBG_871X_LEVEL(_drv_always_, "Wrong rate section: (%s,%s)\n", RateSection, RfPath);
		return;
	}

	if (eqNByte(Bandwidth, (u8 *)("20M"), 3))
		bandwidth = CHANNEL_WIDTH_20;
	else if (eqNByte(Bandwidth, (u8 *)("40M"), 3))
		bandwidth = CHANNEL_WIDTH_40;
	else if (eqNByte(Bandwidth, (u8 *)("80M"), 3))
		bandwidth = CHANNEL_WIDTH_80;
	else {
		DBG_871X_LEVEL(_drv_always_, "unknown bandwidth: %s\n", Bandwidth);
		return;
	}

	if (eqNByte(Band, (u8 *)("2.4G"), 4)) {
		channelIndex = phy_GetChannelIndexOfTxPowerLimit(BAND_ON_2_4G, channel);

		if (channelIndex == -1) {
			DBG_871X_LEVEL(_drv_always_, "unsupported channel: %d at 2.4G\n", channel);
			return;
		}

		if (bandwidth >= MAX_2_4G_BANDWIDTH_NUM) {
			DBG_871X_LEVEL(_drv_always_, "unsupported bandwidth: %s at 2.4G\n", Bandwidth);
			return;
		}

		prevPowerLimit = pHalData->TxPwrLimit_2_4G[regulation][bandwidth][rateSection][channelIndex][RF_PATH_A];

		if (prevPowerLimit != MAX_POWER_INDEX)
			DBG_871X_LEVEL(_drv_always_, "duplicate tx power limit combination [band %s][regulation %s][bw %s][rate section %s][rf path %s][chnl %s]\n"
				, Band, Regulation, Bandwidth, RateSection, RfPath, Channel);

		if (powerLimit < prevPowerLimit)
			pHalData->TxPwrLimit_2_4G[regulation][bandwidth][rateSection][channelIndex][RF_PATH_A] = powerLimit;

		if (0)
			DBG_871X("2.4G Band value : [regulation %d][bw %d][rate_section %d][chnl %d][val %d]\n"
				, regulation, bandwidth, rateSection, channelIndex, pHalData->TxPwrLimit_2_4G[regulation][bandwidth][rateSection][channelIndex][ODM_RF_PATH_A]);
	} else if (eqNByte(Band, (u8 *)("5G"), 2)) {

		channelIndex = phy_GetChannelIndexOfTxPowerLimit(BAND_ON_5G, channel);

		if (channelIndex == -1) {
			DBG_871X_LEVEL(_drv_always_, "unsupported channel: %d at 5G\n", channel);
			return;
		}

		prevPowerLimit = pHalData->TxPwrLimit_5G[regulation][bandwidth][rateSection][channelIndex][RF_PATH_A];

		if (prevPowerLimit != MAX_POWER_INDEX)
			DBG_871X_LEVEL(_drv_always_, "duplicate tx power limit combination [band %s][regulation %s][bw %s][rate section %s][rf path %s][chnl %s]\n"
				, Band, Regulation, Bandwidth, RateSection, RfPath, Channel);

		if (powerLimit < prevPowerLimit)
			pHalData->TxPwrLimit_5G[regulation][bandwidth][rateSection][channelIndex][RF_PATH_A] = powerLimit;

		if (0)
			DBG_871X("5G Band value : [regulation %d][bw %d][rate_section %d][chnl %d][val %d]\n"
				, regulation, bandwidth, rateSection, channel, pHalData->TxPwrLimit_5G[regulation][bandwidth][rateSection][channelIndex][RF_PATH_A]);
	} else {
		DBG_871X_LEVEL(_drv_always_, "Cannot recognize the band info in %s\n", Band);
		return;
	}
}

bool phy_is_tx_power_limit_needed(_adapter *adapter)
{
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(adapter);
	struct registry_priv *regsty = dvobj_to_regsty(adapter_to_dvobj(adapter));

	if (regsty->RegEnableTxPowerLimit == 1
		|| (regsty->RegEnableTxPowerLimit == 2 && hal_data->EEPROMRegulatory == 1))
		return _TRUE;
	return _FALSE;
}

bool phy_is_tx_power_by_rate_needed(_adapter *adapter)
{
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(adapter);
	struct registry_priv *regsty = dvobj_to_regsty(adapter_to_dvobj(adapter));

	if (regsty->RegEnableTxPowerByRate == 1
		|| (regsty->RegEnableTxPowerByRate == 2 && hal_data->EEPROMRegulatory != 2))
		return _TRUE;
	return _FALSE;
}

int phy_load_tx_power_by_rate(_adapter *adapter)
{
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(adapter);
	struct registry_priv *regsty = dvobj_to_regsty(adapter_to_dvobj(adapter));
	int ret = _FAIL;

	hal_data->txpwr_by_rate_loaded = 0;
	PHY_InitTxPowerByRate(adapter);

	/* tx power limit is based on tx power by rate */
	hal_data->txpwr_limit_loaded = 0;

	if (HAL_STATUS_SUCCESS == ODM_ConfigBBWithHeaderFile(&hal_data->odmpriv, CONFIG_BB_PHY_REG_PG)) {
		DBG_871X("default power by rate loaded\n");
		hal_data->txpwr_by_rate_from_file = 0;
		goto post_hdl;
	}

	DBG_871X_LEVEL(_drv_err_, "%s():Read Tx power by rate fail\n", __func__);
	goto exit;

post_hdl:
	if (hal_data->odmpriv.PhyRegPgValueType != PHY_REG_PG_EXACT_VALUE) {
		rtw_warn_on(1);
		goto exit;
	}

	PHY_TxPowerByRateConfiguration(adapter);
	hal_data->txpwr_by_rate_loaded = 1;

	ret = _SUCCESS;

exit:
	return ret;
}

int phy_load_tx_power_limit(_adapter *adapter)
{
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(adapter);
	struct registry_priv *regsty = dvobj_to_regsty(adapter_to_dvobj(adapter));
	int ret = _FAIL;

	hal_data->txpwr_limit_loaded = 0;
	PHY_InitTxPowerLimit(adapter);

	if (!hal_data->txpwr_by_rate_loaded && regsty->target_tx_pwr_valid != _TRUE) {
		DBG_871X_LEVEL(_drv_err_, "%s():Read Tx power limit before target tx power is specify\n", __func__);
		goto exit;
	}

	if (HAL_STATUS_SUCCESS == ODM_ConfigRFWithHeaderFile(&hal_data->odmpriv, CONFIG_RF_TXPWR_LMT, (ODM_RF_RADIO_PATH_E)0)) {
		DBG_871X("default power limit loaded\n");
		hal_data->txpwr_limit_from_file = 0;
		goto post_hdl;
	}

	DBG_871X_LEVEL(_drv_err_, "%s():Read Tx power limit fail\n", __func__);
	goto exit;

post_hdl:
	PHY_ConvertTxPowerLimitToPowerIndex(adapter);
	hal_data->txpwr_limit_loaded = 1;
	ret = _SUCCESS;

exit:
	return ret;
}

void phy_load_tx_power_ext_info(_adapter *adapter)
{
	struct registry_priv *regsty = adapter_to_regsty(adapter);

	/* check registy target tx power */
	regsty->target_tx_pwr_valid = rtw_regsty_chk_target_tx_power_valid(adapter);

	/* power by rate and limit */
	if (phy_is_tx_power_by_rate_needed(adapter)
		|| (phy_is_tx_power_limit_needed(adapter) && regsty->target_tx_pwr_valid != _TRUE))
		phy_load_tx_power_by_rate(adapter);

	if (phy_is_tx_power_limit_needed(adapter))
		phy_load_tx_power_limit(adapter);
}
