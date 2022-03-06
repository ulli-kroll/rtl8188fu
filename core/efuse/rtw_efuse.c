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
#define _RTW_EFUSE_C_

#include <drv_types.h>
#include <hal_data.h>

#include "../hal/efuse/efuse_mask.h"

/*------------------------Define local variable------------------------------*/
u8	fakeEfuseBank=0;
u32	fakeEfuseUsedBytes=0;
u8	fakeEfuseContent[EFUSE_MAX_HW_SIZE]={0};
u8	fakeEfuseInitMap[EFUSE_MAX_MAP_LEN]={0};
u8	fakeEfuseModifiedMap[EFUSE_MAX_MAP_LEN]={0};

u32	BTEfuseUsedBytes=0;
u8	BTEfuseContent[EFUSE_MAX_BT_BANK][EFUSE_MAX_HW_SIZE];
u8	BTEfuseInitMap[EFUSE_BT_MAX_MAP_LEN]={0};
u8	BTEfuseModifiedMap[EFUSE_BT_MAX_MAP_LEN]={0};

u32	fakeBTEfuseUsedBytes=0;
u8	fakeBTEfuseContent[EFUSE_MAX_BT_BANK][EFUSE_MAX_HW_SIZE];
u8	fakeBTEfuseInitMap[EFUSE_BT_MAX_MAP_LEN]={0};
u8	fakeBTEfuseModifiedMap[EFUSE_BT_MAX_MAP_LEN]={0};

u8 	maskfileBuffer[32];
/*------------------------Define local variable------------------------------*/

//------------------------------------------------------------------------------
#define REG_EFUSE_CTRL		0x0030
#define EFUSE_CTRL			REG_EFUSE_CTRL		// E-Fuse Control.
//------------------------------------------------------------------------------

BOOLEAN
Efuse_Read1ByteFromFakeContent(
	IN		PADAPTER	pAdapter,
	IN		u16		Offset,
	IN OUT	u8		*Value	);
BOOLEAN
Efuse_Read1ByteFromFakeContent(
	IN		PADAPTER	pAdapter,
	IN		u16		Offset,
	IN OUT	u8		*Value	)
{
	if(Offset >= EFUSE_MAX_HW_SIZE)
	{
		return _FALSE;
	}
	//DbgPrint("Read fake content, offset = %d\n", Offset);
	if(fakeEfuseBank == 0)
		*Value = fakeEfuseContent[Offset];
	else
		*Value = fakeBTEfuseContent[fakeEfuseBank-1][Offset];
	return _TRUE;
}


/*-----------------------------------------------------------------------------
 * Function:	Efuse_PowerSwitch
 *
 * Overview:	When we want to enable write operation, we should change to 
 *				pwr on state. When we stop write, we should switch to 500k mode
 *				and disable LDO 2.5V.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/17/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
VOID
Efuse_PowerSwitch(
	IN	PADAPTER	pAdapter,
	IN	u8		bWrite,
	IN	u8		PwrState)
{
	pAdapter->HalFunc.EfusePowerSwitch(pAdapter, bWrite, PwrState);
}

VOID
BTEfuse_PowerSwitch(
	IN	PADAPTER	pAdapter,
	IN	u8		bWrite,
	IN	u8		PwrState)
{
	if(pAdapter->HalFunc.BTEfusePowerSwitch)
		pAdapter->HalFunc.BTEfusePowerSwitch(pAdapter, bWrite, PwrState);
}

/*-----------------------------------------------------------------------------
 * Function:	efuse_GetCurrentSize
 *
 * Overview:	Get current efuse size!!!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
u16
Efuse_GetCurrentSize(
	IN PADAPTER		pAdapter,
	IN u8			efuseType,
	IN BOOLEAN		bPseudoTest)
{
	u16 ret=0;

	ret = pAdapter->HalFunc.EfuseGetCurrentSize(pAdapter, efuseType, bPseudoTest);

	return ret;
}

/*  11/16/2008 MH Add description. Get current efuse area enabled word!!. */
u8
Efuse_CalculateWordCnts(IN u8	word_en)
{
	u8 word_cnts = 0;
	if(!(word_en & BIT(0)))	word_cnts++; // 0 : write enable
	if(!(word_en & BIT(1)))	word_cnts++;
	if(!(word_en & BIT(2)))	word_cnts++;
	if(!(word_en & BIT(3)))	word_cnts++;
	return word_cnts;
}

//
//	Description:
//		Execute E-Fuse read byte operation.
//		Refered from SD1 Richard.
//
//	Assumption:
//		1. Boot from E-Fuse and successfully auto-load.
//		2. PASSIVE_LEVEL (USB interface)
//
//	Created by Roger, 2008.10.21.
//
VOID
ReadEFuseByte(
		PADAPTER	Adapter,
		u16 			_offset, 
		u8 			*pbuf, 
		IN BOOLEAN	bPseudoTest) 
{
	u32	value32;
	u8	readbyte;
	u16	retry;
	//u32 start=rtw_get_current_time();

	if(bPseudoTest)
	{
		Efuse_Read1ByteFromFakeContent(Adapter, _offset, pbuf);
		return;
	}
	if (IS_HARDWARE_TYPE_8723B(Adapter))
	{
		// <20130121, Kordan> For SMIC S55 EFUSE specificatoin.
		//0x34[11]: SW force PGMEN input of efuse to high. (for the bank selected by 0x34[9:8])
		PHY_SetMacReg(Adapter, EFUSE_TEST, BIT11, 0);
	}
	//Write Address
	rtw_write8(Adapter, EFUSE_CTRL+1, (_offset & 0xff));  		
	readbyte = rtw_read8(Adapter, EFUSE_CTRL+2);
	rtw_write8(Adapter, EFUSE_CTRL+2, ((_offset >> 8) & 0x03) | (readbyte & 0xfc));  		

	//Write bit 32 0
	readbyte = rtw_read8(Adapter, EFUSE_CTRL+3);		
	rtw_write8(Adapter, EFUSE_CTRL+3, (readbyte & 0x7f));  	
	
	//Check bit 32 read-ready
	retry = 0;
	value32 = rtw_read32(Adapter, EFUSE_CTRL);
	//while(!(((value32 >> 24) & 0xff) & 0x80)  && (retry<10))
	while(!(((value32 >> 24) & 0xff) & 0x80)  && (retry<10000))
	{
		value32 = rtw_read32(Adapter, EFUSE_CTRL);
		retry++;
	}

	// 20100205 Joseph: Add delay suggested by SD1 Victor.
	// This fix the problem that Efuse read error in high temperature condition.
	// Designer says that there shall be some delay after ready bit is set, or the
	// result will always stay on last data we read.
	rtw_udelay_os(50);
	value32 = rtw_read32(Adapter, EFUSE_CTRL);
	
	*pbuf = (u8)(value32 & 0xff);
	//DBG_871X("ReadEFuseByte _offset:%08u, in %d ms\n",_offset ,rtw_get_passing_time_ms(start));
	
}

//
//	Description:
//		1. Execute E-Fuse read byte operation according as map offset and 
//		    save to E-Fuse table.
//		2. Refered from SD1 Richard.
//
//	Assumption:
//		1. Boot from E-Fuse and successfully auto-load.
//		2. PASSIVE_LEVEL (USB interface)
//
//	Created by Roger, 2008.10.21.
//
//	2008/12/12 MH 	1. Reorganize code flow and reserve bytes. and add description.
//					2. Add efuse utilization collect.
//	2008/12/22 MH	Read Efuse must check if we write section 1 data again!!! Sec1
//					write addr must be after sec5.
//

VOID
efuse_ReadEFuse(
	PADAPTER	Adapter,
	u8		efuseType,
	u16		_offset,
	u16 		_size_byte,
	u8      	*pbuf,
	IN	BOOLEAN	bPseudoTest
	);
VOID
efuse_ReadEFuse(
	PADAPTER	Adapter,
	u8		efuseType,
	u16		_offset,
	u16 		_size_byte,
	u8      	*pbuf,
	IN	BOOLEAN	bPseudoTest
	)
{
	Adapter->HalFunc.ReadEFuse(Adapter, efuseType, _offset, _size_byte, pbuf, bPseudoTest);
}

VOID
EFUSE_GetEfuseDefinition(
	IN		PADAPTER	pAdapter,
	IN		u8		efuseType,
	IN		u8		type,
	OUT		void		*pOut,
	IN		BOOLEAN		bPseudoTest
	)
{
	pAdapter->HalFunc.EFUSEGetEfuseDefinition(pAdapter, efuseType, type, pOut, bPseudoTest);
}

/*-----------------------------------------------------------------------------
 * Function:	EFUSE_Read1Byte
 *
 * Overview:	Copy from WMAC fot EFUSE read 1 byte.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 09/23/2008 	MHC		Copy from WMAC.
 *
 *---------------------------------------------------------------------------*/
u8
EFUSE_Read1Byte(	
	IN	PADAPTER	Adapter, 
	IN	u16		Address)
{
	u8	data;
	u8	Bytetemp = {0x00};
	u8	temp = {0x00};
	u32	k=0;
	u16	contentLen=0;

	EFUSE_GetEfuseDefinition(Adapter, EFUSE_WIFI , TYPE_EFUSE_REAL_CONTENT_LEN, (PVOID)&contentLen, _FALSE);

	if (Address < contentLen)	//E-fuse 512Byte
	{
		//Write E-fuse Register address bit0~7
		temp = Address & 0xFF;	
		rtw_write8(Adapter, EFUSE_CTRL+1, temp);	
		Bytetemp = rtw_read8(Adapter, EFUSE_CTRL+2);	
		//Write E-fuse Register address bit8~9
		temp = ((Address >> 8) & 0x03) | (Bytetemp & 0xFC);	
		rtw_write8(Adapter, EFUSE_CTRL+2, temp);	

		//Write 0x30[31]=0
		Bytetemp = rtw_read8(Adapter, EFUSE_CTRL+3);
		temp = Bytetemp & 0x7F;
		rtw_write8(Adapter, EFUSE_CTRL+3, temp);

		//Wait Write-ready (0x30[31]=1)
		Bytetemp = rtw_read8(Adapter, EFUSE_CTRL+3);
		while(!(Bytetemp & 0x80))
		{				
			Bytetemp = rtw_read8(Adapter, EFUSE_CTRL+3);
			k++;
			if(k==1000)
			{
				k=0;
				break;
			}
		}
		data=rtw_read8(Adapter, EFUSE_CTRL);
		return data;
	}
	else
		return 0xFF;
	
}/* EFUSE_Read1Byte */



/*  11/16/2008 MH Read one byte from real Efuse. */
u8
efuse_OneByteRead(
	IN	PADAPTER	pAdapter, 
	IN	u16			addr,
	IN	u8			*data,
	IN	BOOLEAN		bPseudoTest)
{
	u32	tmpidx = 0;
	u8	bResult;
	u8	readbyte;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	
	//DBG_871X("===> EFUSE_OneByteRead(), addr = %x\n", addr);
	//DBG_871X("===> EFUSE_OneByteRead() start, 0x34 = 0x%X\n", rtw_read32(pAdapter, EFUSE_TEST));

	if(bPseudoTest)
	{
		bResult = Efuse_Read1ByteFromFakeContent(pAdapter, addr, data);
		return bResult;
	}
	
	if(	IS_HARDWARE_TYPE_8723B(pAdapter) ||
		(IS_HARDWARE_TYPE_8192E(pAdapter) && (!IS_A_CUT(pHalData->VersionID))) ||
		(IS_VENDOR_8188E_I_CUT_SERIES(pAdapter)) || (IS_CHIP_VENDOR_SMIC(pHalData->VersionID))
	  )
	{
		// <20130121, Kordan> For SMIC EFUSE specificatoin.
		//0x34[11]: SW force PGMEN input of efuse to high. (for the bank selected by 0x34[9:8])	
		//PHY_SetMacReg(pAdapter, 0x34, BIT11, 0);
		rtw_write16(pAdapter, 0x34, rtw_read16(pAdapter,0x34)& (~BIT11) ); 
	}

	// -----------------e-fuse reg ctrl ---------------------------------
	//address			
	rtw_write8(pAdapter, EFUSE_CTRL+1, (u8)(addr&0xff));		
	rtw_write8(pAdapter, EFUSE_CTRL+2, ((u8)((addr>>8) &0x03) ) |
	(rtw_read8(pAdapter, EFUSE_CTRL+2)&0xFC ));	

	//rtw_write8(pAdapter, EFUSE_CTRL+3,  0x72);//read cmd	
	//Write bit 32 0
	readbyte = rtw_read8(pAdapter, EFUSE_CTRL+3);		
	rtw_write8(pAdapter, EFUSE_CTRL+3, (readbyte & 0x7f));

	while(!(0x80 &rtw_read8(pAdapter, EFUSE_CTRL+3))&&(tmpidx<1000))
	{
		rtw_mdelay_os(1);
		tmpidx++;
	}
	if(tmpidx<100)
	{			
		*data=rtw_read8(pAdapter, EFUSE_CTRL);		
		bResult = _TRUE;
	}
	else
	{
		*data = 0xff;	
		bResult = _FALSE;
		DBG_871X("%s: [ERROR] addr=0x%x bResult=%d time out 1s !!!\n", __FUNCTION__, addr, bResult);
		DBG_871X("%s: [ERROR] EFUSE_CTRL =0x%08x !!!\n", __FUNCTION__, rtw_read32(pAdapter, EFUSE_CTRL));
	}

	return bResult;
}

int
Efuse_PgPacketRead(	IN	PADAPTER	pAdapter,
					IN	u8			offset,
					IN	u8			*data,
					IN	BOOLEAN		bPseudoTest)
{
	int	ret=0;

	ret =  pAdapter->HalFunc.Efuse_PgPacketRead(pAdapter, offset, data, bPseudoTest);

	return ret;
}

/*-----------------------------------------------------------------------------
 * Function:	efuse_WordEnableDataRead
 *
 * Overview:	Read allowed word in current efuse section data.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008 	MHC		Create Version 0.
 * 11/21/2008 	MHC		Fix Write bug when we only enable late word.
 *
 *---------------------------------------------------------------------------*/
void
efuse_WordEnableDataRead(IN	u8	word_en,
							IN	u8	*sourdata,
							IN	u8	*targetdata)
{	
	if (!(word_en&BIT(0)))
	{
		targetdata[0] = sourdata[0];
		targetdata[1] = sourdata[1];
	}
	if (!(word_en&BIT(1)))
	{
		targetdata[2] = sourdata[2];
		targetdata[3] = sourdata[3];
	}
	if (!(word_en&BIT(2)))
	{
		targetdata[4] = sourdata[4];
		targetdata[5] = sourdata[5];
	}
	if (!(word_en&BIT(3)))
	{
		targetdata[6] = sourdata[6];
		targetdata[7] = sourdata[7];
	}
}

static u8 efuse_read8(PADAPTER padapter, u16 address, u8 *value)
{
	return efuse_OneByteRead(padapter,address, value, _FALSE);
}

//------------------------------------------------------------------------------
u16 efuse_GetMaxSize(PADAPTER padapter)
{
	u16	max_size;

	max_size = 0;
	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI , TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_size, _FALSE);
	return max_size;
}
//------------------------------------------------------------------------------
u8 efuse_GetCurrentSize(PADAPTER padapter, u16 *size)
{
	Efuse_PowerSwitch(padapter, _FALSE, _TRUE);
	*size = Efuse_GetCurrentSize(padapter, EFUSE_WIFI, _FALSE);
	Efuse_PowerSwitch(padapter, _FALSE, _FALSE);

	return _SUCCESS;
}
//------------------------------------------------------------------------------
u16 efuse_bt_GetMaxSize(PADAPTER padapter)
{
	u16	max_size;

	max_size = 0;
	EFUSE_GetEfuseDefinition(padapter, EFUSE_BT , TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_size, _FALSE);
	return max_size;
}

u8 efuse_bt_GetCurrentSize(PADAPTER padapter, u16 *size)
{
	Efuse_PowerSwitch(padapter, _FALSE, _TRUE);
	*size = Efuse_GetCurrentSize(padapter, EFUSE_BT, _FALSE);
	Efuse_PowerSwitch(padapter, _FALSE, _FALSE);

	return _SUCCESS;
}

u8 rtw_efuse_map_read(PADAPTER padapter, u16 addr, u16 cnts, u8 *data)
{
	u16	mapLen=0;

	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (PVOID)&mapLen, _FALSE);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	Efuse_PowerSwitch(padapter, _FALSE, _TRUE);

	efuse_ReadEFuse(padapter, EFUSE_WIFI, addr, cnts, data, _FALSE);

	Efuse_PowerSwitch(padapter, _FALSE, _FALSE);

	return _SUCCESS;
}

u8 rtw_BT_efuse_map_read(PADAPTER padapter, u16 addr, u16 cnts, u8 *data)
{
	u16	mapLen=0;

	EFUSE_GetEfuseDefinition(padapter, EFUSE_BT, TYPE_EFUSE_MAP_LEN, (PVOID)&mapLen, _FALSE);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	Efuse_PowerSwitch(padapter, _FALSE, _TRUE);

	efuse_ReadEFuse(padapter, EFUSE_BT, addr, cnts, data, _FALSE);

	Efuse_PowerSwitch(padapter, _FALSE, _FALSE);

	return _SUCCESS;
}

BOOLEAN rtw_file_efuse_IsMasked(
	PADAPTER	pAdapter,
	u16		Offset
	)
{
	int r = Offset/16;
	int c = (Offset%16) / 2;
	int result = 0;
	
	if(pAdapter->registrypriv.boffefusemask)
		return FALSE;

	//DBG_871X(" %s ,Offset=%x r= %d , c=%d , maskfileBuffer[r]= %x \n",__func__,Offset,r,c,maskfileBuffer[r]);
	if (c < 4) // Upper double word
	    result = (maskfileBuffer[r] & (0x10 << c));
	else
	    result = (maskfileBuffer[r] & (0x01 << (c-4)));
	
	return (result > 0) ? 0 : 1;

}


u8 rtw_efuse_file_read(PADAPTER padapter,u8 *filepatch,u8 *buf,u32 len)
{
	char *ptmp;
	char *ptmpbuf=NULL;
	u32 rtStatus;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);

	ptmpbuf = rtw_zmalloc(2048);

	if (ptmpbuf == NULL)
		return _FALSE;

	_rtw_memset(ptmpbuf,'\0',2048);
	
	rtStatus = rtw_retrieve_from_file(filepatch, ptmpbuf, 2048);

	if( rtStatus > 100 )
	{
		u32 i,j;
		for(i=0,j=0;j<len;i+=2,j++)
		{
			if (( ptmpbuf[i] == ' ' ) && (ptmpbuf[i+1] != '\n' && ptmpbuf[i+1] != '\0')) {
				i++;
			}
			if( (ptmpbuf[i+1] != '\n' && ptmpbuf[i+1] != '\0'))
			{
					buf[j] = simple_strtoul(&ptmpbuf[i],&ptmp, 16);
					DBG_871X(" i=%d,j=%d, %x \n",i,j,buf[j]);

			} else {
				j--;
			}
			
		}

	} else {
		DBG_871X(" %s ,filepatch %s , FAIL %d\n", __func__, filepatch, rtStatus);
		return _FALSE;
	}
	rtw_mfree(ptmpbuf, 2048);
	DBG_871X(" %s ,filepatch %s , done %d\n", __func__, filepatch, rtStatus);
	return _TRUE;
}


BOOLEAN 
efuse_IsMasked(
	PADAPTER	pAdapter,
	u16		Offset
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	

	//if (bEfuseMaskOFF(pAdapter))
	if(pAdapter->registrypriv.boffefusemask)
		return FALSE;
		
#if defined(CONFIG_RTL8188F)
	if (IS_HARDWARE_TYPE_8188F(pAdapter))
		return (IS_MASKED(8188F, _MUSB, Offset)) ? TRUE : FALSE;
#endif

	return FALSE;	
}

#define RT_ASSERT_RET(expr)												\
	if (!(expr)) {															\
		printk("Assertion failed! %s at ......\n", #expr);							\
		printk("      ......%s,%s, line=%d\n",__FILE__, __FUNCTION__, __LINE__);	\
		return _FAIL;	\
	}



u8 rtw_efuse_mask_map_read(PADAPTER padapter, u16 addr, u16 cnts, u8 *data)
{
	u8	ret = _SUCCESS;
	u16	mapLen = 0, i = 0;
	
	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (PVOID)&mapLen, _FALSE);
	
	ret = rtw_efuse_map_read(padapter, addr, cnts , data);

	if (padapter->registrypriv.boffefusemask == 0) {

			for (i = 0; i < cnts; i++) { 
				if (padapter->registrypriv.bFileMaskEfuse == _TRUE) {
					if (rtw_file_efuse_IsMasked(padapter, addr+i)) /*use file efuse mask.*/ 
							data[i] = 0xff;
				} else {
					/*DBG_8192C(" %s , data[%d] = %x\n", __func__, i, data[i]);*/
					if (efuse_IsMasked(padapter, addr+i)) {
						data[i] = 0xff;
						/*DBG_8192C(" %s ,mask data[%d] = %x\n", __func__, i, data[i]);*/
					}
				}
			}
	
	}
	return ret;

}
/*-----------------------------------------------------------------------------
 * Function:	Efuse_ReadAllMap
 *
 * Overview:	Read All Efuse content
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/11/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
VOID 
Efuse_ReadAllMap(
	IN		PADAPTER	pAdapter, 
	IN		u8		efuseType,
	IN OUT	u8		*Efuse,
	IN		BOOLEAN		bPseudoTest);
VOID 
Efuse_ReadAllMap(
	IN		PADAPTER	pAdapter, 
	IN		u8		efuseType,
	IN OUT	u8		*Efuse,
	IN		BOOLEAN		bPseudoTest)
{
	u16	mapLen=0;

	Efuse_PowerSwitch(pAdapter,_FALSE, _TRUE);

	EFUSE_GetEfuseDefinition(pAdapter, efuseType, TYPE_EFUSE_MAP_LEN, (PVOID)&mapLen, bPseudoTest);

	efuse_ReadEFuse(pAdapter, efuseType, 0, mapLen, Efuse, bPseudoTest);

	Efuse_PowerSwitch(pAdapter,_FALSE, _FALSE);
}

/*-----------------------------------------------------------------------------
 * Function:	efuse_ShadowRead1Byte
 *			efuse_ShadowRead2Byte
 *			efuse_ShadowRead4Byte
 *
 * Overview:	Read from efuse init map by one/two/four bytes !!!!!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/12/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
static VOID
efuse_ShadowRead1Byte(
	IN	PADAPTER	pAdapter,
	IN	u16		Offset,
	IN OUT	u8		*Value)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(pAdapter);

	*Value = pHalData->efuse_eeprom_data[Offset];

}	// EFUSE_ShadowRead1Byte

//---------------Read Two Bytes
static VOID
efuse_ShadowRead2Byte(
	IN	PADAPTER	pAdapter,
	IN	u16		Offset,
	IN OUT	u16		*Value)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(pAdapter);

	*Value = pHalData->efuse_eeprom_data[Offset];
	*Value |= pHalData->efuse_eeprom_data[Offset+1]<<8;

}	// EFUSE_ShadowRead2Byte

//---------------Read Four Bytes
static VOID
efuse_ShadowRead4Byte(
	IN	PADAPTER	pAdapter,
	IN	u16		Offset,
	IN OUT	u32		*Value)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(pAdapter);

	*Value = pHalData->efuse_eeprom_data[Offset];
	*Value |= pHalData->efuse_eeprom_data[Offset+1]<<8;
	*Value |= pHalData->efuse_eeprom_data[Offset+2]<<16;
	*Value |= pHalData->efuse_eeprom_data[Offset+3]<<24;

}	// efuse_ShadowRead4Byte


/*-----------------------------------------------------------------------------
 * Function:	efuse_ShadowWrite1Byte
 *			efuse_ShadowWrite2Byte
 *			efuse_ShadowWrite4Byte
 *
 * Overview:	Write efuse modify map by one/two/four byte.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/12/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Function:	EFUSE_ShadowMapUpdate
 *
 * Overview:	Transfer current EFUSE content to shadow init and modify map.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/13/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
void EFUSE_ShadowMapUpdate(
	IN PADAPTER	pAdapter,
	IN u8		efuseType,
	IN BOOLEAN	bPseudoTest)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(pAdapter);
	u16	mapLen=0;

	EFUSE_GetEfuseDefinition(pAdapter, efuseType, TYPE_EFUSE_MAP_LEN, (PVOID)&mapLen, bPseudoTest);

	if (pHalData->bautoload_fail_flag == _TRUE)
	{
		_rtw_memset(pHalData->efuse_eeprom_data, 0xFF, mapLen);
	}
	else
	{
		#ifdef CONFIG_ADAPTOR_INFO_CACHING_FILE			
		if(_SUCCESS != retriveAdaptorInfoFile(pAdapter->registrypriv.adaptor_info_caching_file_path, pHalData->efuse_eeprom_data)) {
		#endif
		
		Efuse_ReadAllMap(pAdapter, efuseType, pHalData->efuse_eeprom_data, bPseudoTest);
		
		#ifdef CONFIG_ADAPTOR_INFO_CACHING_FILE
			storeAdaptorInfoFile(pAdapter->registrypriv.adaptor_info_caching_file_path, pHalData->efuse_eeprom_data);
		}
		#endif
	}

	//PlatformMoveMemory((PVOID)&pHalData->EfuseMap[EFUSE_MODIFY_MAP][0], 
	//(PVOID)&pHalData->EfuseMap[EFUSE_INIT_MAP][0], mapLen);
}// EFUSE_ShadowMapUpdate


/*-----------------------------------------------------------------------------
 * Function:	EFUSE_ShadowRead
 *
 * Overview:	Read from efuse init map !!!!!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/12/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
void
EFUSE_ShadowRead(
	IN		PADAPTER	pAdapter,
	IN		u8		Type,
	IN		u16		Offset,
	IN OUT	u32		*Value	)
{
	if (Type == 1)
		efuse_ShadowRead1Byte(pAdapter, Offset, (u8 *)Value);
	else if (Type == 2)
		efuse_ShadowRead2Byte(pAdapter, Offset, (u16 *)Value);
	else if (Type == 4)
		efuse_ShadowRead4Byte(pAdapter, Offset, (u32 *)Value);
	
}	// EFUSE_ShadowRead

VOID
Efuse_InitSomeVar(
	IN		PADAPTER	pAdapter
	);
VOID
Efuse_InitSomeVar(
	IN		PADAPTER	pAdapter
	)
{
	u8 i;
	
	_rtw_memset((PVOID)&fakeEfuseContent[0], 0xff, EFUSE_MAX_HW_SIZE);
	_rtw_memset((PVOID)&fakeEfuseInitMap[0], 0xff, EFUSE_MAX_MAP_LEN);
	_rtw_memset((PVOID)&fakeEfuseModifiedMap[0], 0xff, EFUSE_MAX_MAP_LEN);

	for(i=0; i<EFUSE_MAX_BT_BANK; i++)
	{
		_rtw_memset((PVOID)&BTEfuseContent[i][0], EFUSE_MAX_HW_SIZE, 0xff);
	}
	_rtw_memset((PVOID)&BTEfuseInitMap[0], 0xff, EFUSE_BT_MAX_MAP_LEN);
	_rtw_memset((PVOID)&BTEfuseModifiedMap[0], 0xff, EFUSE_BT_MAX_MAP_LEN);

	for(i=0; i<EFUSE_MAX_BT_BANK; i++)
	{
		_rtw_memset((PVOID)&fakeBTEfuseContent[i][0], 0xff, EFUSE_MAX_HW_SIZE);
	}
	_rtw_memset((PVOID)&fakeBTEfuseInitMap[0], 0xff, EFUSE_BT_MAX_MAP_LEN);
	_rtw_memset((PVOID)&fakeBTEfuseModifiedMap[0], 0xff, EFUSE_BT_MAX_MAP_LEN);
}

const u8 _mac_hidden_max_bw_to_hal_bw_cap[MAC_HIDDEN_MAX_BW_NUM] = {
	0,
	0,
	(BW_CAP_160M|BW_CAP_80M|BW_CAP_40M|BW_CAP_20M|BW_CAP_10M|BW_CAP_5M),
	(BW_CAP_5M),
	(BW_CAP_10M|BW_CAP_5M),
	(BW_CAP_20M|BW_CAP_10M|BW_CAP_5M),
	(BW_CAP_40M|BW_CAP_20M|BW_CAP_10M|BW_CAP_5M),
	(BW_CAP_80M|BW_CAP_40M|BW_CAP_20M|BW_CAP_10M|BW_CAP_5M),
};

const u8 _mac_hidden_proto_to_hal_proto_cap[MAC_HIDDEN_PROTOCOL_NUM] = {
	0,
	0,
	(PROTO_CAP_11N|PROTO_CAP_11G|PROTO_CAP_11B),
	(PROTO_CAP_11AC|PROTO_CAP_11N|PROTO_CAP_11G|PROTO_CAP_11B),
};

u8 mac_hidden_wl_func_to_hal_wl_func(u8 func)
{
	u8 wl_func = 0;

	if (func & BIT0)
		wl_func |= WL_FUNC_MIRACAST;
	if (func & BIT1)
		wl_func |= WL_FUNC_P2P;
	if (func & BIT2)
		wl_func |= WL_FUNC_TDLS;
	if (func & BIT3)
		wl_func |= WL_FUNC_FTM;

	return wl_func;
}

#ifdef PLATFORM_LINUX
#ifdef CONFIG_ADAPTOR_INFO_CACHING_FILE
//#include <rtw_eeprom.h>

 int isAdaptorInfoFileValid(void)
{
	return _TRUE;
}

int storeAdaptorInfoFile(char *path, u8* efuse_data)
{
	int ret =_SUCCESS;

	if(path && efuse_data) {
		ret = rtw_store_to_file(path, efuse_data, EEPROM_MAX_SIZE_512);
		if(ret == EEPROM_MAX_SIZE)
			ret = _SUCCESS;
		else
			ret = _FAIL;
	} else {
		DBG_871X("%s NULL pointer\n",__FUNCTION__);
		ret =  _FAIL;
	}
	return ret;
}

int retriveAdaptorInfoFile(char *path, u8* efuse_data)
{
	int ret = _SUCCESS;
	mm_segment_t oldfs;
	struct file *fp;
	
	if(path && efuse_data) {

		ret = rtw_retrieve_from_file(path, efuse_data, EEPROM_MAX_SIZE);
		
		if(ret == EEPROM_MAX_SIZE)
			ret = _SUCCESS;
		else
			ret = _FAIL;

		#if 0
		if(isAdaptorInfoFileValid()) {	
			return 0;
		} else {
			return _FAIL;
		}
		#endif
		
	} else {
		DBG_871X("%s NULL pointer\n",__FUNCTION__);
		ret = _FAIL;
	}
	return ret;
}
#endif /* CONFIG_ADAPTOR_INFO_CACHING_FILE */

#ifdef CONFIG_EFUSE_CONFIG_FILE
u32 rtw_read_efuse_from_file(const char *path, u8 *buf)
{
	u32 i;
	u8 temp[3];
	u32 ret = _FAIL;

	struct file *fp;
	mm_segment_t fs;
	loff_t pos = 0;

	fp = filp_open(path, O_RDONLY, 0);
	if (fp == NULL || IS_ERR(fp)) {
		if (fp != NULL)
			DBG_871X_LEVEL(_drv_always_, "%s open %s fail, err:%ld\n"
				, __func__, path, PTR_ERR(fp));
		else
			DBG_871X_LEVEL(_drv_always_, "%s open %s fail, fp is NULL\n"
				, __func__, path);

		goto exit;
	}

	temp[2] = 0; /* add end of string '\0' */

	fs = get_fs();
	set_fs(KERNEL_DS);

	for (i = 0 ; i < HWSET_MAX_SIZE ; i++) {
		vfs_read(fp, temp, 2, &pos);
		if (sscanf(temp, "%hhx", &buf[i]) != 1) {
			if (0)
				DBG_871X_LEVEL(_drv_err_, "%s sscanf fail\n", __func__);
			buf[i] = 0xFF;
		}
		if ((i % EFUSE_FILE_COLUMN_NUM) == (EFUSE_FILE_COLUMN_NUM - 1)) {
			/* Filter the lates space char. */
			vfs_read(fp, temp, 1, &pos);
			if (strchr(temp, ' ') == NULL) {
				pos--;
				vfs_read(fp, temp, 2, &pos);
			}
		} else {
			pos += 1; /* Filter the space character */
		}
	}

	set_fs(fs);

	DBG_871X_LEVEL(_drv_always_, "efuse file: %s\n", path);
#ifdef CONFIG_DEBUG
	for (i = 0; i < HWSET_MAX_SIZE; i++) {
		if (i % 16 == 0)
			DBG_871X_SEL_NL(RTW_DBGDUMP, "0x%03x: ", i);

		DBG_871X_SEL(RTW_DBGDUMP, "%02X%s"
			, buf[i]
			, ((i + 1) % 16 == 0) ? "\n" : (((i + 1) % 8 == 0) ? "    " : " ")
		);
	}
	DBG_871X_SEL(RTW_DBGDUMP, "\n");
#endif

	ret = _SUCCESS;

exit:
	return ret;
}

u32 rtw_read_macaddr_from_file(const char *path, u8 *buf)
{
	struct file *fp;
	mm_segment_t fs;
	loff_t pos = 0;

	u8 source_addr[18];
	u8 *head, *end;
	int i;
	u32 ret = _FAIL;

	_rtw_memset(source_addr, 0, 18);

	fp = filp_open(path, O_RDONLY, 0);
	if (fp == NULL || IS_ERR(fp)) {
		if (fp != NULL)
			DBG_871X_LEVEL(_drv_always_, "%s open %s fail, err:%ld\n"
				, __func__, path, PTR_ERR(fp));
		else
			DBG_871X_LEVEL(_drv_always_, "%s open %s fail, fp is NULL\n"
				, __func__, path);

		goto exit;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);

	vfs_read(fp, source_addr, 18, &pos);
	source_addr[17] = ':';

	head = end = source_addr;
	for (i = 0; i < ETH_ALEN; i++) {
		while (end && (*end != ':'))
			end++;

		if (end && (*end == ':'))
			*end = '\0';

		if (sscanf(head, "%hhx", &buf[i]) != 1) {
			if (0)
				DBG_871X_LEVEL(_drv_err_, "%s sscanf fail\n", __func__);
			buf[i] = 0xFF;
		}

		if (end) {
			end++;
			head = end;
		}
	}

	set_fs(fs);

	DBG_871X_LEVEL(_drv_always_, "wifi_mac file: %s\n", path);
#ifdef CONFIG_DEBUG
	DBG_871X(MAC_FMT"\n", MAC_ARG(buf));
#endif

	ret = _SUCCESS;

exit:
	return ret;
}
#endif /* CONFIG_EFUSE_CONFIG_FILE */

#endif /* PLATFORM_LINUX */

