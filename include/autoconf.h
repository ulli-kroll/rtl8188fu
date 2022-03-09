/******************************************************************************
 *
 * Copyright(c) 2007 - 2014 Realtek Corporation. All rights reserved.
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
/*
 * Public General Configure
 */
#define AUTOCONF_INCLUDED

#define RTL871X_MODULE_NAME "8188FU"
#define DRV_NAME "rtl8188fu"

#ifndef CONFIG_RTL8188F
#define CONFIG_RTL8188F
#endif
#define CONFIG_USB_HCI

#define PLATFORM_LINUX 1

#define RTL8188F_USB_MAC_LOOPBACK 0

/*
 * Wi-Fi Functions Configure
 */
#define CONFIG_80211N_HT
#define CONFIG_RECV_REORDERING_CTRL

/* #define CONFIG_IOCTL_CFG80211 */		/* Set from Makefile */
#ifdef CONFIG_IOCTL_CFG80211
	/*
	 * Indicate new sta asoc through cfg80211_new_sta
	 * If kernel version >= 3.2 or
	 * version < 3.2 but already apply cfg80211 patch,
	 * RTW_USE_CFG80211_STA_EVENT must be defined!
	 */
	/* #define RTW_USE_CFG80211_STA_EVENT */ /* Indicate new sta asoc through cfg80211_new_sta */
	#define CONFIG_CFG80211_FORCE_COMPATIBLE_2_6_37_UNDER

	/* #define CONFIG_DEBUG_CFG80211 */
	/* #define CONFIG_DRV_ISSUE_PROV_REQ */ /* IOT FOR S2 */

	#define CONFIG_SET_SCAN_DENY_TIMER
	/*#define SUPPLICANT_RTK_VERSION_LOWER_THAN_JB42*/ /* wpa_supplicant realtek version <= jb42 will be defined this */
#endif

#define CONFIG_AP_MODE
#ifdef CONFIG_AP_MODE
	#define CONFIG_NATIVEAP_MLME
	#ifndef CONFIG_NATIVEAP_MLME
		#define CONFIG_HOSTAPD_MLME
	#endif
	/* #define CONFIG_FIND_BEST_CHANNEL */

#endif

/*
 * Added by Kurt 20110511
 */
#ifdef CONFIG_TDLS
	#define CONFIG_TDLS_DRIVER_SETUP
/*
	#define CONFIG_TDLS_AUTOSETUP
*/
	#define CONFIG_TDLS_AUTOCHECKALIVE
	#define CONFIG_TDLS_CH_SW	/* Enable this flag only when we confirm that TDLS CH SW is supported in FW */
#endif

#define CONFIG_80211D


/*
 * Hareware/Firmware Related Configure
 */
/* #define SUPPORT_HW_RFOFF_DETECTED */

#define CONFIG_LED
#ifdef CONFIG_LED
	#define CONFIG_SW_LED
	#ifdef CONFIG_SW_LED
		/* define CONFIG_LED_HANDLED_BY_CMD_THREAD */
	#endif
#endif /* CONFIG_LED */

#define CONFIG_XMIT_ACK
#ifdef CONFIG_XMIT_ACK
	#define CONFIG_ACTIVE_KEEP_ALIVE_CHECK
#endif

#define CONFIG_C2H_PACKET_EN

#define CONFIG_RF_POWER_TRIM

#define RTW_NOTCH_FILTER 0 /* 0:Disable, 1:Enable, */

/*
 * Interface Related Configure
 */
#define CONFIG_USB_TX_AGGREGATION
#define CONFIG_USB_RX_AGGREGATION

#define USB_INTERFERENCE_ISSUE /* this should be checked in all usb interface */

#define CONFIG_GLOBAL_UI_PID

#define CONFIG_OUT_EP_WIFI_MODE

#define ENABLE_USB_DROP_INCORRECT_OUT

/* #define CONFIG_REDUCE_USB_TX_INT	*/ /* Trade-off: Improve performance, but may cause TX URBs blocked by USB Host/Bus driver on few platforms. */

/*
 * CONFIG_USE_USB_BUFFER_ALLOC_XX uses Linux USB Buffer alloc API and is for Linux platform only now!
 */
/* #define CONFIG_USE_USB_BUFFER_ALLOC_TX	*/ /* Trade-off: For TX path, improve stability on some platforms, but may cause performance degrade on other platforms. */
/* #define CONFIG_USE_USB_BUFFER_ALLOC_RX	*/ /* For RX path */

/*
 * USB VENDOR REQ BUFFER ALLOCATION METHOD
 * if not set we'll use function local variable (stack memory)
 */
/* #define CONFIG_USB_VENDOR_REQ_BUFFER_DYNAMIC_ALLOCATE */
#define CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC
#define CONFIG_USB_VENDOR_REQ_MUTEX
#define CONFIG_VENDOR_REQ_RETRY
/* #define CONFIG_USB_SUPPORT_ASYNC_VDN_REQ */


/*
 * Others
 */
#define CONFIG_SKB_COPY	/* for amsdu */

/* #define CONFIG_EASY_REPLACEMENT */

/* #define CONFIG_ADAPTOR_INFO_CACHING_FILE */ /* now just applied on 8192cu only, should make it general... */

/* #define CONFIG_SET_SCAN_DENY_TIMER */

#define CONFIG_NEW_SIGNAL_STAT_PROCESS

/* #define CONFIG_SIGNAL_DISPLAY_DBM */ /* display RX signal with dbm */
#ifdef CONFIG_SIGNAL_DISPLAY_DBM
/* #define CONFIG_BACKGROUND_NOISE_MONITOR */
#endif

/* #define CONFIG_CHECK_AC_LIFETIME	*/ /* Check packet lifetime of 4 ACs. */

#define CONFIG_LONG_DELAY_ISSUE
#define CONFIG_ATTEMPT_TO_FIX_AP_BEACON_ERROR


/*
 * Auto Configure Section
 */
#ifdef CONFIG_MINIMAL_MEMORY_USAGE
	#undef CONFIG_USB_TX_AGGREGATION
	#undef CONFIG_USB_RX_AGGREGATION
#endif

#ifdef CONFIG_POWER_SAVING
	#define CONFIG_IPS
	#define CONFIG_LPS

	#ifdef CONFIG_IPS
	/* #define CONFIG_IPS_LEVEL_2	1  */ /*enable this to set default IPS mode to IPS_LEVEL_2 */
	#endif

	#ifdef CONFIG_LPS_LCLK
		#define CONFIG_XMIT_THREAD_MODE
	#endif
#endif /* CONFIG_POWER_SAVING */

	#define BT_30_SUPPORT 0

#ifdef CONFIG_GPIO_WAKEUP
	#ifndef WAKEUP_GPIO_IDX
		#define WAKEUP_GPIO_IDX	0	/* WIFI Chip Side */
	#endif /* !WAKEUP_GPIO_IDX */
#endif /* CONFIG_GPIO_WAKEUP */

#ifdef CONFIG_AP_MODE
	/* #define CONFIG_INTERRUPT_BASED_TXBCN */ /* Tx Beacon when driver BCN_OK ,BCN_ERR interrupt occurs */
	#ifdef CONFIG_INTERRUPT_BASED_TXBCN
		/* #define CONFIG_INTERRUPT_BASED_TXBCN_EARLY_INT */
		#define CONFIG_INTERRUPT_BASED_TXBCN_BCN_OK_ERR
	#endif
#endif /* CONFIG_AP_MODE */

#ifdef CONFIG_USE_USB_BUFFER_ALLOC_RX

#else
	#define CONFIG_PREALLOC_RECV_SKB
	#ifdef CONFIG_PREALLOC_RECV_SKB
		/* #define CONFIG_FIX_NR_BULKIN_BUFFER */ /* only use PREALLOC_RECV_SKB buffer, don't alloc skb at runtime */
	#endif
#endif

#ifdef CONFIG_USB_TX_AGGREGATION
/* #define CONFIG_TX_EARLY_MODE */
#endif

#ifdef CONFIG_TX_EARLY_MODE
#define RTL8188F_EARLY_MODE_PKT_NUM_10	0
#endif




/*
 * Debug Related Configure
 */
#define CONFIG_DEBUG /* DBG_871X, etc... */
#ifdef CONFIG_DEBUG
	#define DBG	1	/* for ODM & BTCOEX debug */
	#define DBG_PHYDM_MORE 0
#else /* !CONFIG_DEBUG */
	#define DBG	0	/* for ODM & BTCOEX debug */
	#define DBG_PHYDM_MORE 0
#endif /* CONFIG_DEBUG */

#if DBG_PHYDM_MORE
	#define CONFIG_DEBUG_RTL871X /* RT_TRACE, RT_PRINT_DATA, _func_enter_, _func_exit_ */
#endif /* DBG_MORE */


/*

#define DBG_IO
#define DBG_DELAY_OS
#define DBG_MEM_ALLOC
#define DBG_IOCTL

#define DBG_TX
#define DBG_XMIT_BUF
#define DBG_XMIT_BUF_EXT
#define DBG_TX_DROP_FRAME

#define DBG_RX_DROP_FRAME
#define DBG_RX_SEQ
#define DBG_RX_SIGNAL_DISPLAY_PROCESSING
#define DBG_RX_SIGNAL_DISPLAY_SSID_MONITORED "rtw-ap"

#define DBG_SHOW_MCUFWDL_BEFORE_51_ENABLE
#define DBG_ROAMING_TEST

#define DBG_HAL_INIT_PROFILING

#define DBG_MEMORY_LEAK	1
*/
