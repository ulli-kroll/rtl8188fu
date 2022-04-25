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
#ifndef __HAL_INTF_H__
#define __HAL_INTF_H__


enum RTL871X_HCI_TYPE {
	RTW_USB 	= BIT1,
};

enum _CHIP_TYPE {

	NULL_CHIP_TYPE,
	RTL8188E,
	RTL8192E,
	RTL8812,
	RTL8821, //RTL8811
	RTL8723B,
	RTL8814A,
	RTL8703B,
	RTL8188F,
	MAX_CHIP_TYPE
};

extern const u32 _chip_type_to_odm_ic_type[];
#define chip_type_to_odm_ic_type(chip_type) (((chip_type) >= MAX_CHIP_TYPE) ? _chip_type_to_odm_ic_type[MAX_CHIP_TYPE] : _chip_type_to_odm_ic_type[(chip_type)])

typedef enum _HAL_HW_TIMER_TYPE {
	HAL_TIMER_NONE = 0,
	HAL_TIMER_TXBF = 1,
	HAL_TIMER_EARLYMODE = 2,
} HAL_HW_TIMER_TYPE, *PHAL_HW_TIMER_TYPE;


typedef enum _HW_VARIABLES{
	HW_VAR_MEDIA_STATUS,
	HW_VAR_MEDIA_STATUS1,
	HW_VAR_SET_OPMODE,
	HW_VAR_MAC_ADDR,
	HW_VAR_BSSID,
	HW_VAR_INIT_RTS_RATE,
	HW_VAR_BASIC_RATE,
	HW_VAR_TXPAUSE,
	HW_VAR_BCN_FUNC,
	HW_VAR_CORRECT_TSF,
	HW_VAR_CHECK_BSSID,
	HW_VAR_MLME_DISCONNECT,
	HW_VAR_MLME_SITESURVEY,
	HW_VAR_MLME_JOIN,
	HW_VAR_ON_RCR_AM,
       HW_VAR_OFF_RCR_AM,
	HW_VAR_BEACON_INTERVAL,
	HW_VAR_SLOT_TIME,
	HW_VAR_RESP_SIFS,
	HW_VAR_ACK_PREAMBLE,
	HW_VAR_SEC_CFG,
	HW_VAR_SEC_DK_CFG,
	HW_VAR_BCN_VALID,
	HW_VAR_RF_TYPE,
	/* PHYDM odm->SupportAbility */
	HW_VAR_CAM_EMPTY_ENTRY,
	HW_VAR_CAM_INVALID_ALL,
	HW_VAR_AC_PARAM_VO,
	HW_VAR_AC_PARAM_VI,
	HW_VAR_AC_PARAM_BE,
	HW_VAR_AC_PARAM_BK,
	HW_VAR_ACM_CTRL,
	HW_VAR_AMPDU_MIN_SPACE,
	HW_VAR_AMPDU_FACTOR,
	HW_VAR_RXDMA_AGG_PG_TH,
	HW_VAR_SET_RPWM,
	HW_VAR_CPWM,
	HW_VAR_H2C_FW_PWRMODE,
	HW_VAR_H2C_PS_TUNE_PARAM,
	HW_VAR_H2C_FW_JOINBSSRPT,
	HW_VAR_FWLPS_RF_ON,
	HW_VAR_H2C_FW_P2P_PS_OFFLOAD,
	HW_VAR_TRIGGER_GPIO_0,
	HW_VAR_BT_SET_COEXIST,
	HW_VAR_BT_ISSUE_DELBA,	
	HW_VAR_SWITCH_EPHY_WoWLAN,
	HW_VAR_EFUSE_USAGE,
	HW_VAR_EFUSE_BYTES,
	HW_VAR_FIFO_CLEARN_UP,
	HW_VAR_RESTORE_HW_SEQ,
	HW_VAR_CHECK_TXBUF,
	HW_VAR_PCIE_STOP_TX_DMA,
	HW_VAR_APFM_ON_MAC, //Auto FSM to Turn On, include clock, isolation, power control for MAC only
	HW_VAR_HCI_SUS_STATE,
	// The valid upper nav range for the HW updating, if the true value is larger than the upper range, the HW won't update it.
	// Unit in microsecond. 0 means disable this function.
	HW_VAR_SYS_CLKR,
	HW_VAR_NAV_UPPER,
	HW_VAR_C2H_HANDLE,
	HW_VAR_RPT_TIMER_SETTING,
	HW_VAR_TX_RPT_MAX_MACID,	
	HW_VAR_CHK_HI_QUEUE_EMPTY,
	HW_VAR_DL_BCN_SEL,
	HW_VAR_AMPDU_MAX_TIME,
	HW_VAR_WIRELESS_MODE,
	HW_VAR_USB_MODE,
	HW_VAR_PORT_SWITCH,
	HW_VAR_DO_IQK,
	HW_VAR_DM_IN_LPS,
	HW_VAR_SET_REQ_FW_PS,
	HW_VAR_FW_PS_STATE,
	HW_VAR_SOUNDING_ENTER,
	HW_VAR_SOUNDING_LEAVE,
	HW_VAR_SOUNDING_RATE,
	HW_VAR_SOUNDING_STATUS,
	HW_VAR_SOUNDING_FW_NDPA,
	HW_VAR_SOUNDING_CLK,
/*Add by YuChen for TXBF HW timer*/
	HW_VAR_HW_REG_TIMER_INIT,
	HW_VAR_HW_REG_TIMER_RESTART,
	HW_VAR_HW_REG_TIMER_START,
	HW_VAR_HW_REG_TIMER_STOP,
/*Add by YuChen for TXBF HW timer*/
	HW_VAR_DL_RSVD_PAGE,
	HW_VAR_MACID_LINK,
	HW_VAR_MACID_NOLINK,
	HW_VAR_MACID_SLEEP,
	HW_VAR_MACID_WAKEUP,
	HW_VAR_DUMP_MAC_QUEUE_INFO,
	HW_VAR_ASIX_IOT,
	HW_VAR_EN_HW_UPDATE_TSF,
	HW_VAR_CH_SW_NEED_TO_TAKE_CARE_IQK_INFO,
	HW_VAR_CH_SW_IQK_INFO_BACKUP,
	HW_VAR_CH_SW_IQK_INFO_RESTORE,
}HW_VARIABLES;

typedef enum _HAL_DEF_VARIABLE{
	HAL_DEF_UNDERCORATEDSMOOTHEDPWDB,
	HAL_DEF_IS_SUPPORT_ANT_DIV,
	HAL_DEF_DRVINFO_SZ,
	HAL_DEF_MAX_RECVBUF_SZ,
	HAL_DEF_RX_PACKET_OFFSET,
	HAL_DEF_RX_DMA_SZ_WOW,
	HAL_DEF_RX_DMA_SZ,
	HAL_DEF_RX_PAGE_SIZE,
	HAL_DEF_DBG_DUMP_RXPKT,//for dbg
	HAL_DEF_RA_DECISION_RATE,
	HAL_DEF_RA_SGI,
	HAL_DEF_PT_PWR_STATUS,
	HAL_DEF_TX_LDPC, 				// LDPC support
	HAL_DEF_RX_LDPC, 				// LDPC support
	HAL_DEF_TX_STBC, 				// TX STBC support
	HAL_DEF_RX_STBC, 				// RX STBC support
	HAL_DEF_EXPLICIT_BEAMFORMER,// Explicit  Compressed Steering Capable
	HAL_DEF_EXPLICIT_BEAMFORMEE,// Explicit Compressed Beamforming Feedback Capable
	HAL_DEF_BEAMFORMER_CAP,
	HAL_DEF_BEAMFORMEE_CAP,
	HW_VAR_MAX_RX_AMPDU_FACTOR,
	HW_DEF_RA_INFO_DUMP,
	HAL_DEF_DBG_DUMP_TXPKT,

	HAL_DEF_TX_PAGE_SIZE,
	HAL_DEF_TX_PAGE_BOUNDARY,
	HAL_DEF_TX_PAGE_BOUNDARY_WOWLAN,
	HAL_DEF_ANT_DETECT,//to do for 8723a
	HAL_DEF_PCI_SUUPORT_L1_BACKDOOR, // Determine if the L1 Backdoor setting is turned on.
	HAL_DEF_PCI_AMD_L1_SUPPORT,
	HAL_DEF_PCI_ASPM_OSC, // Support for ASPM OSC, added by Roger, 2013.03.27.
	HAL_DEF_MACID_SLEEP, // Support for MACID sleep
	HAL_DEF_DBG_DIS_PWT, //disable Tx power training or not.
	HAL_DEF_EFUSE_USAGE,	/* Get current EFUSE utilization. 2008.12.19. Added by Roger. */
	HAL_DEF_EFUSE_BYTES,
	HW_VAR_BEST_AMPDU_DENSITY,
}HAL_DEF_VARIABLE;

typedef enum _HAL_ODM_VARIABLE{
	HAL_ODM_STA_INFO,	
	HAL_ODM_P2P_STATE,
	HAL_ODM_WIFI_DISPLAY_STATE,
	HAL_ODM_NOISE_MONITOR,
	HAL_ODM_REGULATION,
	HAL_ODM_INITIAL_GAIN,
	HAL_ODM_FA_CNT_DUMP,
	HAL_ODM_RX_INFO_DUMP,
#ifdef CONFIG_AUTO_CHNL_SEL_NHM
	HAL_ODM_AUTO_CHNL_SEL,
#endif
}HAL_ODM_VARIABLE;

typedef enum _HAL_INTF_PS_FUNC{
	HAL_USB_SELECT_SUSPEND,
	HAL_MAX_ID,
}HAL_INTF_PS_FUNC;

typedef s32 (*c2h_id_filter)(u8 *c2h_evt);

struct hal_ops {
	/*** initialize section ***/
	void	(*read_chip_version)(_adapter *padapter);
	void	(*init_default_value)(_adapter *padapter);
	void	(*intf_chip_configure)(_adapter *padapter);
	void	(*read_adapter_info)(_adapter *padapter);
	void	(*hal_power_off)(_adapter *padapter);	
	u32	(*hal_init)(_adapter *padapter);
	u32	(*hal_deinit)(_adapter *padapter);
	void	(*dm_init)(_adapter *padapter);
	void	(*dm_deinit)(_adapter *padapter);
	
	/*** xmit section ***/
	s32	(*init_xmit_priv)(_adapter *padapter);
	void	(*free_xmit_priv)(_adapter *padapter);
	s32	(*hal_xmit)(_adapter *padapter, struct xmit_frame *pxmitframe);
	/*
	 * mgnt_xmit should be implemented to run in interrupt context
	 */
	s32 (*mgnt_xmit)(_adapter *padapter, struct xmit_frame *pmgntframe);
	s32	(*hal_xmitframe_enqueue)(_adapter *padapter, struct xmit_frame *pxmitframe);
#ifdef CONFIG_XMIT_THREAD_MODE
	s32 (*xmit_thread_handler)(_adapter *padapter);
#endif
	void	(*run_thread)(_adapter *padapter);
	void	(*cancel_thread)(_adapter *padapter);

	/*** recv section ***/
	s32	(*init_recv_priv)(_adapter *padapter);
	void	(*free_recv_priv)(_adapter *padapter);	
#if defined(CONFIG_USB_HCI)
	u32	(*inirp_init)(_adapter *padapter);
	u32	(*inirp_deinit)(_adapter *padapter);
#endif
	/*** interrupt hdl section ***/
	void	(*enable_interrupt)(_adapter *padapter);
	void	(*disable_interrupt)(_adapter *padapter);
	u8	(*check_ips_status)(_adapter *padapter);

	/*** DM section ***/
	
	void	(*InitSwLeds)(_adapter *padapter);
	void	(*DeInitSwLeds)(_adapter *padapter);
		

	void	(*set_bwmode_handler)(_adapter *padapter, CHANNEL_WIDTH Bandwidth, u8 Offset);
	void	(*set_channel_handler)(_adapter *padapter, u8 channel);
	void	(*set_chnl_bw_handler)(_adapter *padapter, u8 channel, CHANNEL_WIDTH Bandwidth, u8 Offset40, u8 Offset80);

	void	(*hal_dm_watchdog)(_adapter *padapter);

	void	(*SetHwRegHandler)(_adapter *padapter, u8	variable,u8* val);
	void	(*GetHwRegHandler)(_adapter *padapter, u8	variable,u8* val);

#ifdef CONFIG_C2H_PACKET_EN
	void	(*SetHwRegHandlerWithBuf)(_adapter *padapter, u8 variable, u8 * pbuf, int len);
#endif

	u8	(*GetHalDefVarHandler)(_adapter *padapter, HAL_DEF_VARIABLE eVariable, PVOID pValue);
	u8	(*SetHalDefVarHandler)(_adapter *padapter, HAL_DEF_VARIABLE eVariable, PVOID pValue);

	void	(*GetHalODMVarHandler)(_adapter *padapter, HAL_ODM_VARIABLE eVariable, PVOID pValue1,PVOID pValue2);
	void	(*SetHalODMVarHandler)(_adapter *padapter, HAL_ODM_VARIABLE eVariable, PVOID pValue1,BOOLEAN bSet);

	void	(*UpdateRAMaskHandler)(_adapter *padapter, u32 mac_id, u8 rssi_level);
	void	(*SetBeaconRelatedRegistersHandler)(_adapter *padapter);

	void	(*Add_RateATid)(_adapter *padapter, u64 bitmap, u8 *arg, u8 rssi_level);
	u8	(*interface_ps_func)(_adapter *padapter,HAL_INTF_PS_FUNC efunc_id, u8* val);

	u32	(*read_bbreg)(_adapter *padapter, u32 RegAddr, u32 BitMask);
	void	(*write_bbreg)(_adapter *padapter, u32 RegAddr, u32 BitMask, u32 Data);
	u32	(*read_rfreg)(_adapter *padapter, u8 eRFPath, u32 RegAddr, u32 BitMask);
	void	(*write_rfreg)(_adapter *padapter, u8 eRFPath, u32 RegAddr, u32 BitMask, u32 Data);

#ifdef CONFIG_HOSTAPD_MLME
	s32	(*hostap_mgnt_xmit_entry)(_adapter *padapter, _pkt *pkt);
#endif

	void (*EfusePowerSwitch)(_adapter *padapter, u8 bWrite, u8 PwrState);
	void (*BTEfusePowerSwitch)(_adapter *padapter, u8 bWrite, u8 PwrState);
	void (*ReadEFuse)(_adapter *padapter, u16 _offset, u16 _size_byte, u8 *pbuf);
	void (*EFUSEGetEfuseDefinition)(_adapter *padapter, u8 type, void *pOut);
	u16	(*EfuseGetCurrentSize)(_adapter *padapter);
	int 	(*Efuse_PgPacketRead)(_adapter *padapter, u8 offset, u8 *data);
	
#ifdef CONFIG_IOL
	int (*IOL_exec_cmds_sync)(_adapter *padapter, struct xmit_frame *xmit_frame, u32 max_wating_ms, u32 bndy_cnt);
#endif

	s32 (*c2h_handler)(_adapter *padapter, u8 *c2h_evt);
	c2h_id_filter c2h_id_filter_ccx;
	s32 (*fill_h2c_cmd)(PADAPTER, u8 ElementID, u32 CmdLen, u8 *pCmdBuffer);
	void (*fill_fake_txdesc)(PADAPTER, u8 *pDesc, u32 BufferLen,
			u8 IsPsPoll, u8 IsBTQosNull, u8 bDataFrame);
	s32 (*fw_dl)(_adapter *adapter);

	u8 (*hal_get_tx_buff_rsvd_page_num)(_adapter *adapter, bool wowlan);
#ifdef CONFIG_GPIO_API
	void (*update_hisr_hsisr_ind)(PADAPTER padapter, u32 flag);
#endif
	void (*fw_correct_bcn)(PADAPTER padapter);
};

typedef	enum _RT_EEPROM_TYPE{
	EEPROM_93C46,
	EEPROM_93C56,
	EEPROM_BOOT_EFUSE,
}RT_EEPROM_TYPE,*PRT_EEPROM_TYPE;



#define RF_CHANGE_BY_INIT	0
#define RF_CHANGE_BY_IPS 	BIT28
#define RF_CHANGE_BY_PS 	BIT29
#define RF_CHANGE_BY_HW 	BIT30
#define RF_CHANGE_BY_SW 	BIT31

typedef enum _HARDWARE_TYPE{
	HARDWARE_TYPE_RTL8188EE,
	HARDWARE_TYPE_RTL8188EU,
	HARDWARE_TYPE_RTL8188ES,
//	NEW_GENERATION_IC
	HARDWARE_TYPE_RTL8192EE,
	HARDWARE_TYPE_RTL8192EU,
	HARDWARE_TYPE_RTL8192ES,
	HARDWARE_TYPE_RTL8812E,
	HARDWARE_TYPE_RTL8812AU,
	HARDWARE_TYPE_RTL8811AU,
	HARDWARE_TYPE_RTL8821E,
	HARDWARE_TYPE_RTL8821U,
	HARDWARE_TYPE_RTL8821S,
	HARDWARE_TYPE_RTL8723BE,
	HARDWARE_TYPE_RTL8723BU,
	HARDWARE_TYPE_RTL8723BS,
	HARDWARE_TYPE_RTL8814AE,
	HARDWARE_TYPE_RTL8814AU,
	HARDWARE_TYPE_RTL8814AS,
	HARDWARE_TYPE_RTL8821BE,
	HARDWARE_TYPE_RTL8821BU,
	HARDWARE_TYPE_RTL8821BS,
	HARDWARE_TYPE_RTL8822BE,
	HARDWARE_TYPE_RTL8822BU,
	HARDWARE_TYPE_RTL8822BS,
	HARDWARE_TYPE_RTL8703BE,
	HARDWARE_TYPE_RTL8703BU,
	HARDWARE_TYPE_RTL8703BS,
	HARDWARE_TYPE_RTL8188FE,
	HARDWARE_TYPE_RTL8188FU,
	HARDWARE_TYPE_RTL8188FS,
	HARDWARE_TYPE_MAX,
}HARDWARE_TYPE;

#define IS_NEW_GENERATION_IC(_Adapter)	(rtw_get_hw_type(_Adapter) >= HARDWARE_TYPE_RTL8192EE)

/* RTL8188F Series */
#define IS_HARDWARE_TYPE_8188FE(_Adapter)		(rtw_get_hw_type(_Adapter) == HARDWARE_TYPE_RTL8188FE)
#define IS_HARDWARE_TYPE_8188FS(_Adapter)		(rtw_get_hw_type(_Adapter) == HARDWARE_TYPE_RTL8188FS)
#define IS_HARDWARE_TYPE_8188FU(_Adapter)		(rtw_get_hw_type(_Adapter) == HARDWARE_TYPE_RTL8188FU)
#define	IS_HARDWARE_TYPE_8188F(_Adapter)			\
(IS_HARDWARE_TYPE_8188FE(_Adapter) || IS_HARDWARE_TYPE_8188FU(_Adapter) || IS_HARDWARE_TYPE_8188FS(_Adapter))

typedef enum _wowlan_subcode {
	WOWLAN_ENABLE			= 0,
	WOWLAN_DISABLE			= 1,
	WOWLAN_AP_ENABLE		= 2,
	WOWLAN_AP_DISABLE		= 3,
	WOWLAN_PATTERN_CLEAN		= 4
} wowlan_subcode;

struct wowlan_ioctl_param{
	unsigned int subcode;
	unsigned int subcode_value;
	unsigned int wakeup_reason;
};

#define Rx_Pairwisekey			0x01
#define Rx_GTK					0x02
#define Rx_DisAssoc				0x04
#define Rx_DeAuth				0x08
#define Rx_ARPReq				0x09
#define FWDecisionDisconnect	0x10
#define Rx_MagicPkt				0x21
#define Rx_UnicastPkt			0x22
#define Rx_PatternPkt			0x23
#define	RX_PNOWakeUp			0x55
#define	AP_WakeUp			0x66

u8 rtw_hal_data_init(_adapter *padapter);
void rtw_hal_data_deinit(_adapter *padapter);

void rtw_hal_def_value_init(_adapter *padapter);

void	rtw_hal_free_data(_adapter *padapter);

void rtw_hal_dm_init(_adapter *padapter);
void rtw_hal_dm_deinit(_adapter *padapter);
void rtw_hal_sw_led_init(_adapter *padapter);
void rtw_hal_sw_led_deinit(_adapter *padapter);

u32 rtw_hal_power_on(_adapter *padapter);
void rtw_hal_power_off(_adapter *padapter);

uint rtw_hal_init(_adapter *padapter);
uint rtw_hal_deinit(_adapter *padapter);
void rtw_hal_stop(_adapter *padapter);
void rtw_hal_set_hwreg(PADAPTER padapter, u8 variable, u8 *val);
void rtw_hal_get_hwreg(PADAPTER padapter, u8 variable, u8 *val);

#ifdef CONFIG_C2H_PACKET_EN
void rtw_hal_set_hwreg_with_buf(_adapter *padapter, u8 variable, u8 *pbuf, int len);
#endif

void rtw_hal_chip_configure(_adapter *padapter);
void rtw_hal_read_chip_info(_adapter *padapter);
void rtw_hal_read_chip_version(_adapter *padapter);

u8 rtw_hal_set_def_var(_adapter *padapter, HAL_DEF_VARIABLE eVariable, PVOID pValue);
u8 rtw_hal_get_def_var(_adapter *padapter, HAL_DEF_VARIABLE eVariable, PVOID pValue);

void rtw_hal_set_odm_var(_adapter *padapter, HAL_ODM_VARIABLE eVariable, PVOID pValue1,BOOLEAN bSet);
void	rtw_hal_get_odm_var(_adapter *padapter, HAL_ODM_VARIABLE eVariable, PVOID pValue1,PVOID pValue2);

void rtw_hal_enable_interrupt(_adapter *padapter);
void rtw_hal_disable_interrupt(_adapter *padapter);

u8 rtw_hal_check_ips_status(_adapter *padapter);

#if defined(CONFIG_USB_HCI)
u32	rtw_hal_inirp_init(_adapter *padapter);
u32	rtw_hal_inirp_deinit(_adapter *padapter);
#endif

u8	rtw_hal_intf_ps_func(_adapter *padapter,HAL_INTF_PS_FUNC efunc_id, u8* val);

s32	rtw_hal_xmitframe_enqueue(_adapter *padapter, struct xmit_frame *pxmitframe);
s32	rtw_hal_xmit(_adapter *padapter, struct xmit_frame *pxmitframe);
s32	rtw_hal_mgnt_xmit(_adapter *padapter, struct xmit_frame *pmgntframe);

s32	rtw_hal_init_xmit_priv(_adapter *padapter);
void	rtw_hal_free_xmit_priv(_adapter *padapter);

s32	rtw_hal_init_recv_priv(_adapter *padapter);
void	rtw_hal_free_recv_priv(_adapter *padapter);

void rtw_hal_update_ra_mask(struct sta_info *psta, u8 rssi_level);
void	rtw_hal_add_ra_tid(_adapter *padapter, u64 bitmap, u8 *arg, u8 rssi_level);

void	rtw_hal_start_thread(_adapter *padapter);
void	rtw_hal_stop_thread(_adapter *padapter);

void rtw_hal_bcn_related_reg_setting(_adapter *padapter);

u32	rtw_hal_read_bbreg(_adapter *padapter, u32 RegAddr, u32 BitMask);
void	rtw_hal_write_bbreg(_adapter *padapter, u32 RegAddr, u32 BitMask, u32 Data);
u32	rtw_hal_read_rfreg(_adapter *padapter, u32 eRFPath, u32 RegAddr, u32 BitMask);
void	rtw_hal_write_rfreg(_adapter *padapter, u32 eRFPath, u32 RegAddr, u32 BitMask, u32 Data);

#define PHY_QueryBBReg(Adapter, RegAddr, BitMask) rtw_hal_read_bbreg((Adapter), (RegAddr), (BitMask))
#define PHY_SetBBReg(Adapter, RegAddr, BitMask, Data) rtw_hal_write_bbreg((Adapter), (RegAddr), (BitMask), (Data))
#define PHY_QueryRFReg(Adapter, eRFPath, RegAddr, BitMask) rtw_hal_read_rfreg((Adapter), (eRFPath), (RegAddr), (BitMask))
#define PHY_SetRFReg(Adapter, eRFPath, RegAddr, BitMask, Data) rtw_hal_write_rfreg((Adapter), (eRFPath), (RegAddr), (BitMask), (Data))

void	rtw_hal_set_bwmode(_adapter *padapter, CHANNEL_WIDTH Bandwidth, u8 Offset);
void	rtw_hal_set_chan(_adapter *padapter, u8 channel);
void	rtw_hal_set_chnl_bw(_adapter *padapter, u8 channel, CHANNEL_WIDTH Bandwidth, u8 Offset40, u8 Offset80);
void	rtw_hal_dm_watchdog(_adapter *padapter);
void	rtw_hal_dm_watchdog_in_lps(_adapter *padapter);

#ifdef CONFIG_HOSTAPD_MLME
s32	rtw_hal_hostap_mgnt_xmit_entry(_adapter *padapter, _pkt *pkt);
#endif

#ifdef CONFIG_IOL
int rtw_hal_iol_cmd(ADAPTER *adapter, struct xmit_frame *xmit_frame, u32 max_wating_ms, u32 bndy_cnt);
#endif

#ifdef CONFIG_XMIT_THREAD_MODE
s32 rtw_hal_xmit_thread_handler(_adapter *padapter);
#endif

bool rtw_hal_c2h_valid(_adapter *adapter, u8 *buf);
s32 rtw_hal_c2h_evt_read(_adapter *adapter, u8 *buf);
s32 rtw_hal_c2h_handler(_adapter *adapter, u8 *c2h_evt);
c2h_id_filter rtw_hal_c2h_id_filter_ccx(_adapter *adapter);

s32 rtw_hal_is_disable_sw_channel_plan(PADAPTER padapter);

s32 rtw_hal_macid_sleep(PADAPTER padapter, u8 macid);
s32 rtw_hal_macid_wakeup(PADAPTER padapter, u8 macid);

s32 rtw_hal_fill_h2c_cmd(PADAPTER padapter, u8 ElementID, u32 CmdLen, u8 *pCmdBuffer);
void rtw_hal_fill_fake_txdesc(_adapter *padapter, u8 *pDesc, u32 BufferLen,
		u8 IsPsPoll, u8 IsBTQosNull, u8 bDataFrame);
u8 rtw_hal_get_txbuff_rsvd_page_num(_adapter *adapter, bool wowlan);

#ifdef CONFIG_GPIO_API
void rtw_hal_update_hisr_hsisr_ind(_adapter *padapter, u32 flag);
#endif

void rtw_hal_fw_correct_bcn(_adapter *padapter);
s32 rtw_hal_fw_dl(_adapter *padapter);

u8 rtw_hal_ops_check(_adapter *padapter);

#endif //__HAL_INTF_H__

