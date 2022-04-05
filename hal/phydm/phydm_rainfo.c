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

#if (defined(CONFIG_RA_DBG_CMD))
VOID
ODM_C2HRaParaReportHandler(
	IN	PVOID	pDM_VOID,
	IN pu1Byte   CmdBuf,
	IN u1Byte   CmdLen
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pRA_T		    pRA_Table = &pDM_Odm->DM_RA_Table;

	u1Byte  para_idx = CmdBuf[0]; //Retry Penalty, NH, NL
	u1Byte  RateTypeStart = CmdBuf[1];
	u1Byte  RateTypeLength = CmdLen - 2;
	u1Byte  i;

	ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("[ From FW C2H RA Para ]  CmdBuf[0]= (( %d ))\n", CmdBuf[0]));

	if (para_idx == RADBG_RTY_PENALTY) {
		ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" |Rate Index|   |RTY Penality Index| \n"));

		for (i = 0 ; i < (RateTypeLength) ; i++) {
			if (pRA_Table->is_ra_dbg_init)
				pRA_Table->RTY_P_default[RateTypeStart + i] = CmdBuf[2 + i];

			pRA_Table->RTY_P[RateTypeStart + i] = CmdBuf[2 + i];
			ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("%8d  %15d \n", (RateTypeStart + i), pRA_Table->RTY_P[RateTypeStart + i]));
		}

	} else	if (para_idx == RADBG_N_HIGH) {
		ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" |Rate Index|    |N-High| \n"));


	} else	if (para_idx == RADBG_N_LOW){
		ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" |Rate Index|   |N-Low| \n"));

	}
	else	 if (para_idx == RADBG_RATE_UP_RTY_RATIO) {
		ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" |Rate Index|   |Rate Up RTY Ratio| \n"));

		for (i = 0 ; i < (RateTypeLength) ; i++) {
			if (pRA_Table->is_ra_dbg_init)
				pRA_Table->RATE_UP_RTY_RATIO_default[RateTypeStart + i] = CmdBuf[2 + i];

			pRA_Table->RATE_UP_RTY_RATIO[RateTypeStart + i] = CmdBuf[2 + i];
			ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("%8d  %15d \n", (RateTypeStart + i), pRA_Table->RATE_UP_RTY_RATIO[RateTypeStart + i]));
		}
	} else	 if (para_idx == RADBG_RATE_DOWN_RTY_RATIO) {
		ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" |Rate Index|   |Rate Down RTY Ratio| \n"));

		for (i = 0 ; i < (RateTypeLength) ; i++) {
			if (pRA_Table->is_ra_dbg_init)
				pRA_Table->RATE_DOWN_RTY_RATIO_default[RateTypeStart + i] = CmdBuf[2 + i];

			pRA_Table->RATE_DOWN_RTY_RATIO[RateTypeStart + i] = CmdBuf[2 + i];
			ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("%8d  %15d \n", (RateTypeStart + i), pRA_Table->RATE_DOWN_RTY_RATIO[RateTypeStart + i]));
		}
	} else	 if (para_idx == RADBG_DEBUG_MONITOR1) {
		ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("-------------------------------\n"));
		if (pDM_Odm->SupportICType & PHYDM_IC_3081_SERIES) {

			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d \n", "RSSI =", CmdBuf[1]));
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  0x%x \n", "Rate =", CmdBuf[2] & 0x7f));
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d \n", "SGI =", (CmdBuf[2] & 0x80) >> 7));
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d \n", "BW =", CmdBuf[3]));
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d \n", "BW_max =", CmdBuf[4]));
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  0x%x \n", "multi_rate0 =", CmdBuf[5]));
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  0x%x \n", "multi_rate1 =", CmdBuf[6]));
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d \n", "DISRA =",	CmdBuf[7]));
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d \n", "VHT_EN =", CmdBuf[8]));
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d \n", "SGI_support =",	CmdBuf[9]));
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d \n", "try_ness =", CmdBuf[10]));
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  0x%x \n", "pre_rate =", CmdBuf[11]));
		} else {
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d \n", "RSSI =", CmdBuf[1]));
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %x \n", "BW =", CmdBuf[2]));
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d \n", "DISRA =", CmdBuf[3]));
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d \n", "VHT_EN =", CmdBuf[4]));
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d \n", "Hightest Rate =", CmdBuf[5]));
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  0x%x \n", "Lowest Rate =", CmdBuf[6]));
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  0x%x \n", "SGI_support =", CmdBuf[7]));
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d \n", "Rate_ID =",	CmdBuf[8]));;
		}
		ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("-------------------------------\n"));
	} else	 if (para_idx == RADBG_DEBUG_MONITOR2) {
		ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("-------------------------------\n"));
		if (pDM_Odm->SupportICType & PHYDM_IC_3081_SERIES) {
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d \n", "RateID =", CmdBuf[1]));
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  0x%x \n", "highest_rate =", CmdBuf[2]));
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  0x%x \n", "lowest_rate =", CmdBuf[3]));

			for (i = 4 ; i <= 11 ; i++)
				ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("RAMASK =  0x%x \n", CmdBuf[i]));
		} else {
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %x%x  %x%x  %x%x  %x%x \n", "RA Mask:",
						 CmdBuf[8], CmdBuf[7], CmdBuf[6], CmdBuf[5], CmdBuf[4], CmdBuf[3], CmdBuf[2], CmdBuf[1]));
		}
		ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("-------------------------------\n"));
	} else	 if (para_idx == RADBG_DEBUG_MONITOR3) {

		for (i = 0 ; i < (CmdLen - 1) ; i++)
			ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("content[%d] =  %d \n", i, CmdBuf[1 + i]));
	} else	 if (para_idx == RADBG_DEBUG_MONITOR4)
		ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  {%d.%d} \n", "RA Version =", CmdBuf[1], CmdBuf[2]));

}

VOID
odm_RA_ParaAdjust_Send_H2C(
	IN	PVOID	pDM_VOID
)
{

	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pRA_T			pRA_Table = &pDM_Odm->DM_RA_Table;
	u1Byte			H2C_Parameter[6] = {0};

	H2C_Parameter[0] =  RA_FIRST_MACID;

	//ODM_RT_TRACE(pDM_Odm,PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("RA_Para_feedback_req= (( %d ))  \n",pRA_Table->RA_Para_feedback_req ));
	if (pRA_Table->RA_Para_feedback_req) { //H2C_Parameter[5]=1 ; ask FW for all RA parameters
		ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("[H2C] Ask FW for RA parameter \n"));
		H2C_Parameter[5] |= BIT1; //ask FW to report RA parameters
		H2C_Parameter[1] =  pRA_Table->para_idx; //pRA_Table->para_idx;
		pRA_Table->RA_Para_feedback_req = 0;
	} else {
		ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("[H2C] Send H2C to FW for modifying RA parameter \n"));

		H2C_Parameter[1] =  pRA_Table->para_idx;
		H2C_Parameter[2] =  pRA_Table->rate_idx;
		//1 [8 bit]
		if (pRA_Table->para_idx == RADBG_RTY_PENALTY || pRA_Table->para_idx == RADBG_RATE_UP_RTY_RATIO || pRA_Table->para_idx == RADBG_RATE_DOWN_RTY_RATIO) {
			H2C_Parameter[3] = pRA_Table->value;
			H2C_Parameter[4] = 0;
		}
		//1 [16 bit]
		else { //if ((pRA_Table->rate_idx==RADBG_N_HIGH)||(pRA_Table->rate_idx==RADBG_N_LOW))
			H2C_Parameter[3] = (u1Byte)(((pRA_Table->value_16) & 0xf0) >> 4); //byte1
			H2C_Parameter[4] = (u1Byte)((pRA_Table->value_16) & 0x0f);	   //byte0
		}
	}
	ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" H2C_Parameter[1] = 0x%x  \n", H2C_Parameter[1]));
	ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" H2C_Parameter[2] = 0x%x  \n", H2C_Parameter[2]));
	ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" H2C_Parameter[3] = 0x%x  \n", H2C_Parameter[3]));
	ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" H2C_Parameter[4] = 0x%x  \n", H2C_Parameter[4]));
	ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" H2C_Parameter[5] = 0x%x  \n", H2C_Parameter[5]));

	ODM_FillH2CCmd(pDM_Odm, ODM_H2C_RA_PARA_ADJUST, 6, H2C_Parameter);

}


VOID
odm_RA_ParaAdjust(
	IN		PVOID		pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pRA_T			pRA_Table = &pDM_Odm->DM_RA_Table;
	u1Byte			para_idx = pRA_Table->para_idx;
	u1Byte			rate_idx = pRA_Table->rate_idx;
	u1Byte			value = pRA_Table->value;
	u1Byte			Pre_value = 0xff;

	BOOLEAN			sign = 0;

	if (pRA_Table->para_idx == RADBG_RTY_PENALTY) {
		Pre_value = pRA_Table->RTY_P[rate_idx];
		pRA_Table->RTY_P[rate_idx] = value;
		pRA_Table->RTY_P_modify_note[rate_idx] = 1;
	} else 	if (pRA_Table->para_idx == RADBG_N_HIGH) {

	} else 	if (pRA_Table->para_idx == RADBG_N_LOW) {

	} else	 if (pRA_Table->para_idx == RADBG_RATE_UP_RTY_RATIO) {
		Pre_value = pRA_Table->RATE_UP_RTY_RATIO[rate_idx];
		pRA_Table->RATE_UP_RTY_RATIO[rate_idx] = value;
		pRA_Table->RATE_UP_RTY_RATIO_modify_note[rate_idx] = 1;
	} else	 if (pRA_Table->para_idx == RADBG_RATE_DOWN_RTY_RATIO) {
		Pre_value = pRA_Table->RATE_DOWN_RTY_RATIO[rate_idx];
		pRA_Table->RATE_DOWN_RTY_RATIO[rate_idx] = value;
		pRA_Table->RATE_DOWN_RTY_RATIO_modify_note[rate_idx] = 1;
	}
	ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("  Change RA Papa[%d], Rate[ %d ],   ((%d))  ->  ((%d)) \n", pRA_Table->para_idx, rate_idx, Pre_value, value));
	odm_RA_ParaAdjust_Send_H2C(pDM_Odm);
}


VOID
phydm_ra_print_msg(
	IN		PVOID		pDM_VOID,
	IN		u1Byte		*value,
	IN		u1Byte		*value_default,
	IN		u1Byte		*modify_note
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pRA_T			pRA_Table = &pDM_Odm->DM_RA_Table;
	u4Byte i;

	ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" |Rate index| |Current-value| |Default-value| |Modify?| \n"));
	for (i = 0 ; i <= (pRA_Table->rate_length); i++) {
		ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("     [ %d ]  %10d  %14d  %14s \n", i, value[i], value_default[i], ((modify_note[i] == 1) ? "V" : " .  ")));
	}

}

VOID
odm_RA_debug(
	IN		PVOID		pDM_VOID,
	IN		u4Byte		*const dm_value
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pRA_T			pRA_Table = &pDM_Odm->DM_RA_Table;

	pRA_Table->is_ra_dbg_init = FALSE;

	if (dm_value[0] == 100) { /*1 Print RA Parameters*/
		u1Byte	default_pointer_value;
		u1Byte	*pvalue;
		u1Byte	*pvalue_default;
		u1Byte	*pmodify_note;

		pvalue = pvalue_default = pmodify_note = &default_pointer_value;

		ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("\n------------------------------------------------------------------------------------\n"));

		if (dm_value[1] == RADBG_RTY_PENALTY) { /* [1]*/
			ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" [1] RTY_PENALTY\n"));
			pvalue		=	&(pRA_Table->RTY_P[0]);
			pvalue_default	=	&(pRA_Table->RTY_P_default[0]);
			pmodify_note	=	(u1Byte *)&(pRA_Table->RTY_P_modify_note[0]);
		} else if (dm_value[1] == RADBG_N_HIGH) { /* [2]*/
			ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" [2] N_HIGH\n"));

		} else if (dm_value[1] == RADBG_N_LOW) { /*[3]*/
			ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" [3] N_LOW\n"));

		} else if (dm_value[1] == RADBG_RATE_UP_RTY_RATIO) { /* [8]*/
			ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" [8] RATE_UP_RTY_RATIO\n"));
			pvalue		=	&(pRA_Table->RATE_UP_RTY_RATIO[0]);
			pvalue_default	=	&(pRA_Table->RATE_UP_RTY_RATIO_default[0]);
			pmodify_note	=	(u1Byte *)&(pRA_Table->RATE_UP_RTY_RATIO_modify_note[0]);
		} else if (dm_value[1] == RADBG_RATE_DOWN_RTY_RATIO) { /* [9]*/
			ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" [9] RATE_DOWN_RTY_RATIO\n"));
			pvalue		=	&(pRA_Table->RATE_DOWN_RTY_RATIO[0]);
			pvalue_default	=	&(pRA_Table->RATE_DOWN_RTY_RATIO_default[0]);
			pmodify_note	=	(u1Byte *)&(pRA_Table->RATE_DOWN_RTY_RATIO_modify_note[0]);
		}

		phydm_ra_print_msg(pDM_Odm, pvalue, pvalue_default, pmodify_note);
		ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("\n------------------------------------------------------------------------------------\n\n"));

	} else if (dm_value[0] == 101) {
		pRA_Table->para_idx = (u1Byte)dm_value[1];

		pRA_Table->RA_Para_feedback_req = 1;
		odm_RA_ParaAdjust_Send_H2C(pDM_Odm);
	} else {
		pRA_Table->para_idx = (u1Byte)dm_value[0];
		pRA_Table->rate_idx  = (u1Byte)dm_value[1];
		pRA_Table->value = (u1Byte)dm_value[2];

		odm_RA_ParaAdjust(pDM_Odm);
	}

}

VOID
odm_RA_ParaAdjust_init(
	IN		PVOID		pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pRA_T		        pRA_Table = &pDM_Odm->DM_RA_Table;
	u1Byte			i;
	u1Byte			ra_para_pool_u8[3] = { RADBG_RTY_PENALTY,  RADBG_RATE_UP_RTY_RATIO, RADBG_RATE_DOWN_RTY_RATIO};
	/*
		RTY_PENALTY		=	1,  //u8
		N_HIGH 				=	2,
		N_LOW				=	3,
		RATE_UP_TABLE		=	4,
		RATE_DOWN_TABLE	=	5,
		TRYING_NECESSARY	=	6,
		DROPING_NECESSARY =	7,
		RATE_UP_RTY_RATIO	=	8, //u8
		RATE_DOWN_RTY_RATIO=	9, //u8
		ALL_PARA		=	0xff

	*/
	ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("odm_RA_ParaAdjust_init \n"));

	pRA_Table->is_ra_dbg_init = TRUE;
	for (i = 0; i < 3; i++) {
		pRA_Table->RA_Para_feedback_req = 1;
		pRA_Table->para_idx	=	ra_para_pool_u8[i];
		odm_RA_ParaAdjust_Send_H2C(pDM_Odm);
	}

	if (pDM_Odm->SupportICType == ODM_RTL8192E)
		pRA_Table->rate_length = ODM_RATEMCS15;
	else if ((pDM_Odm->SupportICType == ODM_RTL8723B) || (pDM_Odm->SupportICType == ODM_RTL8188E))
		pRA_Table->rate_length = ODM_RATEMCS7;
	else if ((pDM_Odm->SupportICType == ODM_RTL8821) || (pDM_Odm->SupportICType == ODM_RTL8881A))
		pRA_Table->rate_length = ODM_RATEVHTSS1MCS9;
	else if (pDM_Odm->SupportICType == ODM_RTL8812)
		pRA_Table->rate_length = ODM_RATEVHTSS2MCS9;
	else if (pDM_Odm->SupportICType == ODM_RTL8814A)
		pRA_Table->rate_length = ODM_RATEVHTSS3MCS9;
	else
		pRA_Table->rate_length = ODM_RATEVHTSS4MCS9;

}

#else

VOID
ODM_C2HRaParaReportHandler(
	IN	PVOID	pDM_VOID,
	IN pu1Byte   CmdBuf,
	IN u1Byte   CmdLen
)
{
}

VOID
odm_RA_debug(
	IN		PVOID		pDM_VOID,
	IN		u4Byte		*const dm_value
)
{
}

VOID
odm_RA_ParaAdjust_init(
	IN		PVOID		pDM_VOID
)

{
}

#endif //#if (defined(CONFIG_RA_DBG_CMD))

VOID
phydm_ra_dynamic_retry_count(
	IN	PVOID	pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pRA_T		        pRA_Table = &pDM_Odm->DM_RA_Table;
	PSTA_INFO_T		pEntry;
	u1Byte	i, retry_offset;
	u4Byte	ma_rx_tp;
	/*ODM_RT_TRACE(pDM_Odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("pDM_Odm->pre_b_noisy = %d\n", pDM_Odm->pre_b_noisy ));*/
	if (pDM_Odm->pre_b_noisy != pDM_Odm->NoisyDecision) {

		if (pDM_Odm->NoisyDecision) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("->Noisy Env. RA fallback value\n"));
			ODM_SetMACReg(pDM_Odm, 0x430, bMaskDWord, 0x0);
			ODM_SetMACReg(pDM_Odm, 0x434, bMaskDWord, 0x04030201);		
		} else {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("->Clean Env. RA fallback value\n"));
			ODM_SetMACReg(pDM_Odm, 0x430, bMaskDWord, 0x02010000);
			ODM_SetMACReg(pDM_Odm, 0x434, bMaskDWord, 0x06050403);		
		}
		pDM_Odm->pre_b_noisy = pDM_Odm->NoisyDecision;
	}
}

#if (defined(CONFIG_RA_DYNAMIC_RTY_LIMIT))

VOID
phydm_retry_limit_table_bound(
	IN	PVOID	pDM_VOID,
	IN	u1Byte	*retry_limit,
	IN	u1Byte	offset
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pRA_T		        pRA_Table = &pDM_Odm->DM_RA_Table;

	if (*retry_limit >  offset) {
		
		*retry_limit -= offset;
		
		if (*retry_limit < pRA_Table->retrylimit_low)
			*retry_limit = pRA_Table->retrylimit_low;
		else if (*retry_limit > pRA_Table->retrylimit_high)
			*retry_limit = pRA_Table->retrylimit_high;
	} else
		*retry_limit = pRA_Table->retrylimit_low;
}

VOID
phydm_reset_retry_limit_table(
	IN	PVOID	pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pRA_T		        pRA_Table = &pDM_Odm->DM_RA_Table;
	u1Byte			i;

		#if ((RTL8192E_SUPPORT == 1) || (RTL8723B_SUPPORT == 1) || (RTL8188E_SUPPORT == 1)) 
			u1Byte per_rate_retrylimit_table_20M[ODM_RATEMCS15+1] = {
				1, 1, 2, 4,					/*CCK*/
				2, 2, 4, 6, 8, 12, 16, 18,		/*OFDM*/
				2, 4, 6, 8, 12, 18, 20, 22,		/*20M HT-1SS*/
				2, 4, 6, 8, 12, 18, 20, 22		/*20M HT-2SS*/
			};
			u1Byte per_rate_retrylimit_table_40M[ODM_RATEMCS15+1] = {
				1, 1, 2, 4,					/*CCK*/
				2, 2, 4, 6, 8, 12, 16, 18,		/*OFDM*/
				4, 8, 12, 16, 24, 32, 32, 32,		/*40M HT-1SS*/
				4, 8, 12, 16, 24, 32, 32, 32		/*40M HT-2SS*/
			};

		#endif

	memcpy(&(pRA_Table->per_rate_retrylimit_20M[0]), &(per_rate_retrylimit_table_20M[0]), ODM_NUM_RATE_IDX);
	memcpy(&(pRA_Table->per_rate_retrylimit_40M[0]), &(per_rate_retrylimit_table_40M[0]), ODM_NUM_RATE_IDX);

	for (i = 0; i < ODM_NUM_RATE_IDX; i++) {
		phydm_retry_limit_table_bound(pDM_Odm, &(pRA_Table->per_rate_retrylimit_20M[i]), 0);
		phydm_retry_limit_table_bound(pDM_Odm, &(pRA_Table->per_rate_retrylimit_40M[i]), 0);
	}	
}

VOID
phydm_ra_dynamic_retry_limit(
	IN	PVOID	pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pRA_T		        pRA_Table = &pDM_Odm->DM_RA_Table;
	PSTA_INFO_T		pEntry;
	u1Byte	i, retry_offset;
	u4Byte	ma_rx_tp;


	if (pDM_Odm->pre_number_active_client == pDM_Odm->number_active_client) {
		
		ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" pre_number_active_client ==  number_active_client\n"));
		return;
		
	} else {
		if (pDM_Odm->number_active_client == 1) {
			phydm_reset_retry_limit_table(pDM_Odm);
			ODM_RT_TRACE(pDM_Odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("one client only->reset to default value\n"));
		} else {

			retry_offset = pDM_Odm->number_active_client * pRA_Table->retry_descend_num;
			
			for (i = 0; i < ODM_NUM_RATE_IDX; i++) {

				phydm_retry_limit_table_bound(pDM_Odm, &(pRA_Table->per_rate_retrylimit_20M[i]), retry_offset);
				phydm_retry_limit_table_bound(pDM_Odm, &(pRA_Table->per_rate_retrylimit_40M[i]), retry_offset);	
			}				
		}
	}
}

VOID
phydm_ra_dynamic_retry_limit_init(
	IN	PVOID	pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pRA_T		        pRA_Table = &pDM_Odm->DM_RA_Table;

	pRA_Table->retry_descend_num = RA_RETRY_DESCEND_NUM;
	pRA_Table->retrylimit_low = RA_RETRY_LIMIT_LOW;
	pRA_Table->retrylimit_high = RA_RETRY_LIMIT_HIGH;
	
	phydm_reset_retry_limit_table(pDM_Odm);
	
}
#else
VOID
phydm_ra_dynamic_retry_limit(
	IN	PVOID	pDM_VOID
)
{
}
#endif


VOID
phydm_c2h_ra_report_handler(
	IN PVOID	pDM_VOID,
	IN pu1Byte   CmdBuf,
	IN u1Byte   CmdLen
)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pRA_T		pRA_Table = &pDM_Odm->DM_RA_Table;
	u1Byte	legacy_table[12] = {1,2,5,11,6,9,12,18,24,36,48,54};
	u1Byte	macid = CmdBuf[1];
	
	u1Byte	rate = CmdBuf[0];
	u1Byte	rate_idx = rate & 0x7f; /*remove bit7 SGI*/
	u1Byte	vht_en=(rate_idx >= ODM_RATEVHTSS1MCS0)? 1 :0;	
	u1Byte	b_sgi = (rate & 0x80)>>7;
	
	u1Byte	pre_rate = pRA_Table->link_tx_rate[macid];
	u1Byte	pre_rate_idx = pre_rate & 0x7f; /*remove bit7 SGI*/
	u1Byte	pre_vht_en=(pre_rate_idx >= ODM_RATEVHTSS1MCS0)? 1 :0;	
	u1Byte	pre_b_sgi = (pre_rate & 0x80)>>7;
	
	ODM_UpdateInitRate(pDM_Odm, rate_idx);

	/*ODM_RT_TRACE(pDM_Odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD,("RA: rate_idx=0x%x , sgi = %d\n", rate_idx, b_sgi));*/
	/*if (pDM_Odm->SupportICType & (ODM_RTL8703B))*/
	{
		if (CmdLen >= 4) {
			if (CmdBuf[3] == 0) {
				ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("Init-Rate Update\n"));
				/**/
			} else if (CmdBuf[3] == 0xff) {
				ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("FW Level: Fix rate\n"));
				/**/
			} else if (CmdBuf[3] == 1) {
				ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("Try Success\n"));
				/**/
			} else if (CmdBuf[3] == 2) {
				ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("Try Fail & Try Again\n"));
				/**/
			} else if (CmdBuf[3] == 3) {
				ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("Rate Back\n"));
				/**/
			} else if (CmdBuf[3] == 4) {
				ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("start rate by RSSI\n"));
				/**/
			} else if (CmdBuf[3] == 5) {
				ODM_RT_TRACE(pDM_Odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("Try rate\n"));
				/**/
			}
		}
	}
	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("Tx Rate Update, MACID[%d] ( %s%s%s%s%d%s%s ) -> ( %s%s%s%s%d%s%s)\n",
		macid,
		((pre_rate_idx >= ODM_RATEVHTSS1MCS0) && (pre_rate_idx <= ODM_RATEVHTSS1MCS9)) ? "VHT 1ss  " : "",
		((pre_rate_idx >= ODM_RATEVHTSS2MCS0) && (pre_rate_idx <= ODM_RATEVHTSS2MCS9)) ? "VHT 2ss " : "",
		((pre_rate_idx >= ODM_RATEVHTSS3MCS0) && (pre_rate_idx <= ODM_RATEVHTSS3MCS9)) ? "VHT 3ss " : "",
		(pre_rate_idx >= ODM_RATEMCS0) ? "MCS " : "",
		(pre_vht_en) ? ((pre_rate_idx - ODM_RATEVHTSS1MCS0)%10) : ((pre_rate_idx >= ODM_RATEMCS0)? (pre_rate_idx - ODM_RATEMCS0) : ((pre_rate_idx <= ODM_RATE54M)?legacy_table[pre_rate_idx]:0)),
		(pre_b_sgi) ? "-S" : "  ",
		(pre_rate_idx >= ODM_RATEMCS0) ? "" : "M",
		((rate_idx >= ODM_RATEVHTSS1MCS0) && (rate_idx <= ODM_RATEVHTSS1MCS9)) ? "VHT 1ss  " : "",
		((rate_idx >= ODM_RATEVHTSS2MCS0) && (rate_idx <= ODM_RATEVHTSS2MCS9)) ? "VHT 2ss " : "",
		((rate_idx >= ODM_RATEVHTSS3MCS0) && (rate_idx <= ODM_RATEVHTSS3MCS9)) ? "VHT 3ss " : "",
		(rate_idx >= ODM_RATEMCS0) ? "MCS " : "",
		(vht_en) ? ((rate_idx - ODM_RATEVHTSS1MCS0)%10) : ((rate_idx >= ODM_RATEMCS0)? (rate_idx - ODM_RATEMCS0) : ((rate_idx <= ODM_RATE54M)?legacy_table[rate_idx]:0)),
		(b_sgi) ? "-S" : "  ",
		(rate_idx >= ODM_RATEMCS0) ? "" : "M" ));

	pRA_Table->link_tx_rate[macid] = rate;



}

VOID
odm_RSSIMonitorInit(
	IN		PVOID		pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pRA_T		pRA_Table = &pDM_Odm->DM_RA_Table;
	pRA_Table->firstconnect = FALSE;

}

VOID
ODM_RAPostActionOnAssoc(
	IN	PVOID	pDM_VOID
)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;

	pDM_Odm->H2C_RARpt_connect = 1;
	odm_RSSIMonitorCheck(pDM_Odm);
	pDM_Odm->H2C_RARpt_connect = 0;
}

VOID
odm_RSSIMonitorCheck(
	IN		PVOID		pDM_VOID
)
{
	//
	// For AP/ADSL use prtl8192cd_priv
	// For CE/NIC use PADAPTER
	//
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	if (!(pDM_Odm->SupportAbility & ODM_BB_RSSI_MONITOR))
		return;

	//
	// 2011/09/29 MH In HW integration first stage, we provide 4 different handle to operate
	// at the same time. In the stage2/3, we need to prive universal interface and merge all
	// HW dynamic mechanism.
	//
	odm_RSSIMonitorCheckCE(pDM_Odm);

}	// odm_RSSIMonitorCheck

VOID
odm_RSSIMonitorCheckMP(
	IN	PVOID	pDM_VOID
)
{
}

/*H2C_RSSI_REPORT*/
s8 phydm_rssi_report(PDM_ODM_T pDM_Odm, u8 mac_id)
{
	PADAPTER Adapter = pDM_Odm->Adapter;
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(Adapter);
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(Adapter);
	u8 H2C_Parameter[4] = {0};
	u8 UL_DL_STATE = 0, STBC_TX = 0, TxBF_EN = 0;
	u8 cmdlen = 4, first_connect = _FALSE;
	u64	curTxOkCnt = 0, curRxOkCnt = 0;
	PSTA_INFO_T pEntry = pDM_Odm->pODM_StaInfo[mac_id];
	
	if (!IS_STA_VALID(pEntry))
		return _FAIL;

	if (mac_id != pEntry->mac_id) {
		DBG_871X("%s mac_id:%u:%u invalid\n", __func__, mac_id, pEntry->mac_id);
		rtw_warn_on(1);
		return _FAIL;
	}	
	
	if (IS_MCAST(pEntry->hwaddr))  /*if(psta->mac_id ==1)*/
		return _FAIL;

	if (pEntry->rssi_stat.UndecoratedSmoothedPWDB == (-1)) {
		DBG_871X("%s mac_id:%u, mac:"MAC_FMT", rssi == -1\n", __func__, pEntry->mac_id, MAC_ARG(pEntry->hwaddr));
		return _FAIL;
	}

	curTxOkCnt = pdvobjpriv->traffic_stat.cur_tx_bytes;
	curRxOkCnt = pdvobjpriv->traffic_stat.cur_rx_bytes;
	if (curRxOkCnt > (curTxOkCnt * 6))
		UL_DL_STATE = 1;
	else
		UL_DL_STATE = 0;
	
	#ifdef CONFIG_BEAMFORMING
	{
		#if (BEAMFORMING_SUPPORT == 1)
		BEAMFORMING_CAP Beamform_cap = phydm_Beamforming_GetEntryBeamCapByMacId(pDM_Odm, pEntry->mac_id);
		#else/*for drv beamforming*/
		BEAMFORMING_CAP Beamform_cap = beamforming_get_entry_beam_cap_by_mac_id(&Adapter->mlmepriv, pEntry->mac_id);
		#endif

		if (Beamform_cap & (BEAMFORMER_CAP_HT_EXPLICIT | BEAMFORMER_CAP_VHT_SU))
			TxBF_EN = 1;
		else
			TxBF_EN = 0;
	}
	#endif /*#ifdef CONFIG_BEAMFORMING*/
		
	if (TxBF_EN)
		STBC_TX = 0;
	else {
		#ifdef CONFIG_80211AC_VHT
		if (IsSupportedVHT(pEntry->wireless_mode))
			STBC_TX = TEST_FLAG(pEntry->vhtpriv.stbc_cap, STBC_VHT_ENABLE_TX);
		else
		#endif
			STBC_TX = TEST_FLAG(pEntry->htpriv.stbc_cap, STBC_HT_ENABLE_TX);
	}
		
	H2C_Parameter[0] = (u8)(pEntry->mac_id & 0xFF);
	H2C_Parameter[2] = pEntry->rssi_stat.UndecoratedSmoothedPWDB & 0x7F;
		
	if (UL_DL_STATE)
		H2C_Parameter[3] |= RAINFO_BE_RX_STATE;
		
	if (TxBF_EN)
		H2C_Parameter[3] |= RAINFO_BF_STATE;
	if (STBC_TX)
		H2C_Parameter[3] |= RAINFO_STBC_STATE;
	if (pDM_Odm->NoisyDecision)
		H2C_Parameter[3] |= RAINFO_NOISY_STATE;
		
	if (pEntry->ra_rpt_linked == _FALSE) {
		H2C_Parameter[3] |= RAINFO_INIT_RSSI_RATE_STATE;
		pEntry->ra_rpt_linked = _TRUE;
		first_connect = _TRUE;
	}
		
	#if 1
	if (first_connect) {
		DBG_871X("%s mac_id:%u, mac:"MAC_FMT", rssi:%d\n", __func__,
			pEntry->mac_id, MAC_ARG(pEntry->hwaddr), pEntry->rssi_stat.UndecoratedSmoothedPWDB);
			
		DBG_871X("%s RAINFO - TP:%s, TxBF:%s, STBC:%s, Noisy:%s, Firstcont:%s\n", __func__,
			(UL_DL_STATE) ? "DL" : "UL", (TxBF_EN) ? "EN" : "DIS", (STBC_TX) ? "EN" : "DIS",
			(pDM_Odm->NoisyDecision) ? "True" : "False", (first_connect) ? "True" : "False");
	}
	#endif
		
	if (pHalData->fw_ractrl == _TRUE) {
		ODM_FillH2CCmd(pDM_Odm, ODM_H2C_RSSI_REPORT, cmdlen, H2C_Parameter);
	} else {
	}
	return _SUCCESS;
}

void phydm_ra_rssi_rpt_wk_hdl(PVOID pContext)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pContext;
	int i;
	u8 mac_id = 0xFF;
	PSTA_INFO_T	pEntry = NULL;	
	
	for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {
		pEntry = pDM_Odm->pODM_StaInfo[i];
		if (IS_STA_VALID(pEntry)) {
			if (IS_MCAST(pEntry->hwaddr))  /*if(psta->mac_id ==1)*/
				continue;
			if (pEntry->ra_rpt_linked == _FALSE) {
				mac_id = i;
				break;
			}
		}
	}
	if (mac_id != 0xFF)
		phydm_rssi_report(pDM_Odm, mac_id);
}
void phydm_ra_rssi_rpt_wk(PVOID pContext)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pContext;
	
	rtw_run_in_thread_cmd(pDM_Odm->Adapter, phydm_ra_rssi_rpt_wk_hdl, pDM_Odm);
}

VOID
odm_RSSIMonitorCheckCE(
	IN		PVOID		pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PADAPTER		Adapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);	
	PSTA_INFO_T           pEntry;
	int	i;
	int	tmpEntryMaxPWDB = 0, tmpEntryMinPWDB = 0xff;
	u8	sta_cnt = 0;
	
	if (pDM_Odm->bLinked != _TRUE)
		return;	

	for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {
		pEntry = pDM_Odm->pODM_StaInfo[i];
		if (IS_STA_VALID(pEntry)) {
			if (IS_MCAST(pEntry->hwaddr))  /*if(psta->mac_id ==1)*/
				continue;

			if (pEntry->rssi_stat.UndecoratedSmoothedPWDB == (-1))
				continue;

			if (pEntry->rssi_stat.UndecoratedSmoothedPWDB < tmpEntryMinPWDB)
				tmpEntryMinPWDB = pEntry->rssi_stat.UndecoratedSmoothedPWDB;

			if (pEntry->rssi_stat.UndecoratedSmoothedPWDB > tmpEntryMaxPWDB)
				tmpEntryMaxPWDB = pEntry->rssi_stat.UndecoratedSmoothedPWDB;

			if (phydm_rssi_report(pDM_Odm, i))
				sta_cnt++;
		}
	}
	/*DBG_871X("%s==> sta_cnt(%d)\n", __func__, sta_cnt);*/

	if (tmpEntryMaxPWDB != 0)	// If associated entry is found
		pHalData->EntryMaxUndecoratedSmoothedPWDB = tmpEntryMaxPWDB;
	else
		pHalData->EntryMaxUndecoratedSmoothedPWDB = 0;

	if (tmpEntryMinPWDB != 0xff) // If associated entry is found
		pHalData->EntryMinUndecoratedSmoothedPWDB = tmpEntryMinPWDB;
	else
		pHalData->EntryMinUndecoratedSmoothedPWDB = 0;

	FindMinimumRSSI(Adapter);//get pdmpriv->MinUndecoratedPWDBForDM

	pDM_Odm->RSSI_Min = pHalData->MinUndecoratedPWDBForDM;
	//ODM_CmnInfoUpdate(&pHalData->odmpriv ,ODM_CMNINFO_RSSI_MIN, pdmpriv->MinUndecoratedPWDBForDM);
}


VOID
odm_RSSIMonitorCheckAP(
	IN		PVOID		pDM_VOID
)
{

}


VOID
odm_RateAdaptiveMaskInit(
	IN	PVOID	pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PODM_RATE_ADAPTIVE	pOdmRA = &pDM_Odm->RateAdaptive;

	pOdmRA->Type = DM_Type_ByDriver;
	if (pOdmRA->Type == DM_Type_ByDriver)
		pDM_Odm->bUseRAMask = _TRUE;
	else
		pDM_Odm->bUseRAMask = _FALSE;

	pOdmRA->RATRState = DM_RATR_STA_INIT;

	pOdmRA->LdpcThres = 35;
	pOdmRA->bUseLdpc = FALSE;

	pOdmRA->HighRSSIThresh = 50;
	pOdmRA->LowRSSIThresh = 20;
}
/*-----------------------------------------------------------------------------
 * Function:	odm_RefreshRateAdaptiveMask()
 *
 * Overview:	Update rate table mask according to rssi
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	05/27/2009	hpfan	Create Version 0.
 *
 *---------------------------------------------------------------------------*/
VOID
odm_RefreshRateAdaptiveMask(
	IN	PVOID	pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_TRACE, ("odm_RefreshRateAdaptiveMask()---------->\n"));
	if (!(pDM_Odm->SupportAbility & ODM_BB_RA_MASK)) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_TRACE, ("odm_RefreshRateAdaptiveMask(): Return cos not supported\n"));
		return;
	}
	//
	// 2011/09/29 MH In HW integration first stage, we provide 4 different handle to operate
	// at the same time. In the stage2/3, we need to prive universal interface and merge all
	// HW dynamic mechanism.
	//
	odm_RefreshRateAdaptiveMaskCE(pDM_Odm);
		
}

VOID
odm_RefreshRateAdaptiveMaskMP(
	IN		PVOID		pDM_VOID
)
{
}


VOID
odm_RefreshRateAdaptiveMaskCE(
	IN	PVOID	pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u1Byte	i;
	PADAPTER	pAdapter	 =  pDM_Odm->Adapter;
	PODM_RATE_ADAPTIVE		pRA = &pDM_Odm->RateAdaptive;

	if (RTW_CANNOT_RUN(pAdapter)) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_TRACE, ("<---- odm_RefreshRateAdaptiveMask(): driver is going to unload\n"));
		return;
	}

	if (!pDM_Odm->bUseRAMask) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("<---- odm_RefreshRateAdaptiveMask(): driver does not control rate adaptive mask\n"));
		return;
	}

	//printk("==> %s \n",__FUNCTION__);

	for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {
		PSTA_INFO_T pstat = pDM_Odm->pODM_StaInfo[i];
		if (IS_STA_VALID(pstat)) {
			if (IS_MCAST(pstat->hwaddr))  //if(psta->mac_id ==1)
				continue;


			if (TRUE == ODM_RAStateCheck(pDM_Odm, pstat->rssi_stat.UndecoratedSmoothedPWDB, FALSE , &pstat->rssi_level)) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("RSSI:%d, RSSI_LEVEL:%d\n", pstat->rssi_stat.UndecoratedSmoothedPWDB, pstat->rssi_level));
				//printk("RSSI:%d, RSSI_LEVEL:%d\n", pstat->rssi_stat.UndecoratedSmoothedPWDB, pstat->rssi_level);
				rtw_hal_update_ra_mask(pstat, pstat->rssi_level);
			} else if (pDM_Odm->bChangeState) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("Change Power Training State, bDisablePowerTraining = %d\n", pDM_Odm->bDisablePowerTraining));
				rtw_hal_update_ra_mask(pstat, pstat->rssi_level);
			}

		}
	}

}

VOID
odm_RefreshRateAdaptiveMaskAPADSL(
	IN	PVOID	pDM_VOID
)
{
}


// Return Value: BOOLEAN
// - TRUE: RATRState is changed.
BOOLEAN
ODM_RAStateCheck(
	IN		PVOID			pDM_VOID,
	IN		s4Byte			RSSI,
	IN		BOOLEAN			bForceUpdate,
	OUT		pu1Byte			pRATRState
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PODM_RATE_ADAPTIVE pRA = &pDM_Odm->RateAdaptive;
	const u1Byte GoUpGap = 5;
	u1Byte HighRSSIThreshForRA = pRA->HighRSSIThresh;
	u1Byte LowRSSIThreshForRA = pRA->LowRSSIThresh;
	u1Byte RATRState;
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("RSSI= (( %d )), Current_RSSI_level = (( %d ))\n", RSSI, *pRATRState));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("[Ori RA RSSI Thresh]  High= (( %d )), Low = (( %d ))\n", HighRSSIThreshForRA, LowRSSIThreshForRA));
	// Threshold Adjustment:
	// when RSSI state trends to go up one or two levels, make sure RSSI is high enough.
	// Here GoUpGap is added to solve the boundary's level alternation issue.

	switch (*pRATRState) {
	case DM_RATR_STA_INIT:
	case DM_RATR_STA_HIGH:
		break;

	case DM_RATR_STA_MIDDLE:
		HighRSSIThreshForRA += GoUpGap;
		break;

	case DM_RATR_STA_LOW:
		HighRSSIThreshForRA += GoUpGap;
		LowRSSIThreshForRA += GoUpGap;
		break;


	default:
		ODM_RT_ASSERT(pDM_Odm, FALSE, ("wrong rssi level setting %d !", *pRATRState));
		break;
	}

	// Decide RATRState by RSSI.
	if (RSSI > HighRSSIThreshForRA)
		RATRState = DM_RATR_STA_HIGH;
	else if (RSSI > LowRSSIThreshForRA)
		RATRState = DM_RATR_STA_MIDDLE;

	else
		RATRState = DM_RATR_STA_LOW;
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("[Mod RA RSSI Thresh]  High= (( %d )), Low = (( %d ))\n", HighRSSIThreshForRA, LowRSSIThreshForRA));
	/*printk("==>%s,RATRState:0x%02x ,RSSI:%d\n",__FUNCTION__,RATRState,RSSI);*/

	if (*pRATRState != RATRState || bForceUpdate) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("[RSSI Level Update] %d -> %d\n", *pRATRState, RATRState));
		*pRATRState = RATRState;
		return TRUE;
	}

	return FALSE;
}

VOID
odm_RefreshBasicRateMask(
	IN	PVOID	pDM_VOID
)
{
}


VOID
phydm_ra_info_init(
	IN	PVOID	pDM_VOID
	)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;

	#if (defined(CONFIG_RA_DYNAMIC_RTY_LIMIT))
	phydm_ra_dynamic_retry_limit_init(pDM_Odm);
	#endif

	/*phydm_fw_trace_en_h2c(pDM_Odm, 1, 0, 0);*/
}


u1Byte
odm_Find_RTS_Rate(
	IN		PVOID			pDM_VOID,
	IN		u1Byte			Tx_Rate,
	IN		BOOLEAN			bErpProtect
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u1Byte	RTS_Ini_Rate = ODM_RATE6M;
	
	if (bErpProtect) /* use CCK rate as RTS*/
		RTS_Ini_Rate = ODM_RATE1M;
	else {
		switch (Tx_Rate) {
		case ODM_RATEVHTSS3MCS9:
		case ODM_RATEVHTSS3MCS8:
		case ODM_RATEVHTSS3MCS7:
		case ODM_RATEVHTSS3MCS6:
		case ODM_RATEVHTSS3MCS5:
		case ODM_RATEVHTSS3MCS4:
		case ODM_RATEVHTSS3MCS3:
		case ODM_RATEVHTSS2MCS9:
		case ODM_RATEVHTSS2MCS8:
		case ODM_RATEVHTSS2MCS7:
		case ODM_RATEVHTSS2MCS6:
		case ODM_RATEVHTSS2MCS5:
		case ODM_RATEVHTSS2MCS4:
		case ODM_RATEVHTSS2MCS3:
		case ODM_RATEVHTSS1MCS9:
		case ODM_RATEVHTSS1MCS8:
		case ODM_RATEVHTSS1MCS7:
		case ODM_RATEVHTSS1MCS6:
		case ODM_RATEVHTSS1MCS5:
		case ODM_RATEVHTSS1MCS4:
		case ODM_RATEVHTSS1MCS3:
		case ODM_RATEMCS15:
		case ODM_RATEMCS14:
		case ODM_RATEMCS13:
		case ODM_RATEMCS12:
		case ODM_RATEMCS11:
		case ODM_RATEMCS7:
		case ODM_RATEMCS6:
		case ODM_RATEMCS5:
		case ODM_RATEMCS4:
		case ODM_RATEMCS3:
		case ODM_RATE54M:
		case ODM_RATE48M:
		case ODM_RATE36M:
		case ODM_RATE24M:		
			RTS_Ini_Rate = ODM_RATE24M;
			break;
		case ODM_RATEVHTSS3MCS2:
		case ODM_RATEVHTSS3MCS1:
		case ODM_RATEVHTSS2MCS2:
		case ODM_RATEVHTSS2MCS1:
		case ODM_RATEVHTSS1MCS2:
		case ODM_RATEVHTSS1MCS1:
		case ODM_RATEMCS10:
		case ODM_RATEMCS9:
		case ODM_RATEMCS2:
		case ODM_RATEMCS1:
		case ODM_RATE18M:
		case ODM_RATE12M:
			RTS_Ini_Rate = ODM_RATE12M;
			break;
		case ODM_RATEVHTSS3MCS0:
		case ODM_RATEVHTSS2MCS0:
		case ODM_RATEVHTSS1MCS0:
		case ODM_RATEMCS8:
		case ODM_RATEMCS0:
		case ODM_RATE9M:
		case ODM_RATE6M:
			RTS_Ini_Rate = ODM_RATE6M;
			break;
		case ODM_RATE11M:
		case ODM_RATE5_5M:
		case ODM_RATE2M:
		case ODM_RATE1M:
			RTS_Ini_Rate = ODM_RATE1M;
			break;
		default:
			RTS_Ini_Rate = ODM_RATE6M;
			break;
		}
	}

	if (*pDM_Odm->pBandType == 1) {
		if (RTS_Ini_Rate < ODM_RATE6M)
			RTS_Ini_Rate = ODM_RATE6M;
	}
	return RTS_Ini_Rate;

}

VOID
odm_Set_RA_DM_ARFB_by_Noisy(
	IN	PDM_ODM_T	pDM_Odm
)
{
	/*DbgPrint("DM_ARFB ====>\n");*/
	if (pDM_Odm->bNoisyState) {
		ODM_Write4Byte(pDM_Odm, 0x430, 0x00000000);
		ODM_Write4Byte(pDM_Odm, 0x434, 0x05040200);
		/*DbgPrint("DM_ARFB ====> Noisy State\n");*/
	} else {
		ODM_Write4Byte(pDM_Odm, 0x430, 0x02010000);
		ODM_Write4Byte(pDM_Odm, 0x434, 0x07050403);
		/*DbgPrint("DM_ARFB ====> Clean State\n");*/
	}

}

VOID
ODM_UpdateNoisyState(
	IN	PVOID		pDM_VOID,
	IN	BOOLEAN		bNoisyStateFromC2H
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;

	/*DbgPrint("Get C2H Command! NoisyState=0x%x\n ", bNoisyStateFromC2H);*/
	odm_Set_RA_DM_ARFB_by_Noisy(pDM_Odm);
};

u4Byte
Set_RA_DM_Ratrbitmap_by_Noisy(
	IN	PVOID			pDM_VOID,
	IN	WIRELESS_MODE	WirelessMode,
	IN	u4Byte			ratr_bitmap,
	IN	u1Byte			rssi_level
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u4Byte ret_bitmap = ratr_bitmap;
	
	return ret_bitmap;
	
	switch (WirelessMode) {
	case WIRELESS_MODE_AC_24G:
	case WIRELESS_MODE_AC_5G:
	case WIRELESS_MODE_AC_ONLY:
		if (pDM_Odm->bNoisyState) { /*in Noisy State*/
			if (rssi_level == 1)
				ret_bitmap &= 0xfc3e0c08;		// Reserve MCS 5-9
			else if (rssi_level == 2)
				ret_bitmap &= 0xfe3f8e08;		// Reserve MCS 3-9
			else if (rssi_level == 3)
				ret_bitmap &= 0xffffffff;
			else
				ret_bitmap &= 0xffffffff;
		} else {                                /* in SNR State*/
			if (rssi_level == 1)
				ret_bitmap &= 0xfe3f0e08;		// Reserve MCS 4-9
			else if (rssi_level == 2)
				ret_bitmap &= 0xff3fcf8c;		// Reserve MCS 2-9
			else if (rssi_level == 3)
				ret_bitmap &= 0xffffffff;
			else
				ret_bitmap &= 0xffffffff;
		}
		break;
	case WIRELESS_MODE_B:
	case WIRELESS_MODE_A:
	case WIRELESS_MODE_G:
	case WIRELESS_MODE_N_24G:
	case WIRELESS_MODE_N_5G:
		if (pDM_Odm->bNoisyState) {
			if (rssi_level == 1)
				ret_bitmap &= 0x0f0e0c08;		// Reserve MCS 4-7; MCS12-15
			else if (rssi_level == 2)
				ret_bitmap &= 0x0fcfce0c;		// Reserve MCS 2-7; MCS10-15
			else if (rssi_level == 3)
				ret_bitmap &= 0xffffffff;
			else
				ret_bitmap &= 0xffffffff;
		} else {
			if (rssi_level == 1)
				ret_bitmap &= 0x0f8f8e08;		// Reserve MCS 3-7; MCS11-15
			else if (rssi_level == 2)
				ret_bitmap &= 0x0fefef8c;		// Reserve MCS 1-7; MCS9-15
			else if (rssi_level == 3)
				ret_bitmap &= 0xffffffff;
			else
				ret_bitmap &= 0xffffffff;
		}
		break;
	default:
		break;
	}
	/*DbgPrint("DM_RAMask ====> rssi_LV = %d, BITMAP = %x\n", rssi_level, ret_bitmap);*/
	return ret_bitmap;

}

VOID
ODM_UpdateInitRate(
	IN	PVOID		pDM_VOID,
	IN	u1Byte		Rate
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u1Byte			p = 0;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Get C2H Command! Rate=0x%x\n", Rate));

	pDM_Odm->TxRate = Rate;

}


static void
FindMinimumRSSI(
	IN	PADAPTER	pAdapter
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	PDM_ODM_T		pDM_Odm = &(pHalData->odmpriv);

	/*Determine the minimum RSSI*/

	if ((pDM_Odm->bLinked != _TRUE) &&
		(pHalData->EntryMinUndecoratedSmoothedPWDB == 0)) {
		pHalData->MinUndecoratedPWDBForDM = 0;
		/*ODM_RT_TRACE(pDM_Odm,COMP_BB_POWERSAVING, DBG_LOUD, ("Not connected to any\n"));*/
	} else
		pHalData->MinUndecoratedPWDBForDM = pHalData->EntryMinUndecoratedSmoothedPWDB;

	/*DBG_8192C("%s=>MinUndecoratedPWDBForDM(%d)\n",__FUNCTION__,pdmpriv->MinUndecoratedPWDBForDM);*/
	/*ODM_RT_TRACE(pDM_Odm,COMP_DIG, DBG_LOUD, ("MinUndecoratedPWDBForDM =%d\n",pHalData->MinUndecoratedPWDBForDM));*/
}

u8Byte
PhyDM_Get_Rate_Bitmap_Ex(
	IN	PVOID		pDM_VOID,
	IN	u4Byte		macid,
	IN	u8Byte		ra_mask,
	IN	u1Byte		rssi_level,
	OUT		u8Byte	*dm_RA_Mask,
	OUT		u1Byte	*dm_RteID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PSTA_INFO_T	pEntry;
	u8Byte	rate_bitmap = 0;
	u1Byte	WirelessMode;

	pEntry = pDM_Odm->pODM_StaInfo[macid];
	if (!IS_STA_VALID(pEntry))
		return ra_mask;
	WirelessMode = pEntry->wireless_mode;
	switch (WirelessMode) {
	case ODM_WM_B:
		if (ra_mask & 0x000000000000000c) /* 11M or 5.5M enable */
			rate_bitmap = 0x000000000000000d;
		else
			rate_bitmap = 0x000000000000000f;
		break;

	case (ODM_WM_G):
	case (ODM_WM_A):
		if (rssi_level == DM_RATR_STA_HIGH)
			rate_bitmap = 0x0000000000000f00;
		else
			rate_bitmap = 0x0000000000000ff0;
		break;

	case (ODM_WM_B|ODM_WM_G):
		if (rssi_level == DM_RATR_STA_HIGH)
			rate_bitmap = 0x0000000000000f00;
		else if (rssi_level == DM_RATR_STA_MIDDLE)
			rate_bitmap = 0x0000000000000ff0;
		else
			rate_bitmap = 0x0000000000000ff5;
		break;

	case (ODM_WM_B|ODM_WM_G|ODM_WM_N24G):
	case (ODM_WM_B|ODM_WM_N24G):
	case (ODM_WM_G|ODM_WM_N24G):
	case (ODM_WM_A|ODM_WM_N5G): {
		if (pDM_Odm->RFType == ODM_1T2R || pDM_Odm->RFType == ODM_1T1R) {
			if (rssi_level == DM_RATR_STA_HIGH)
				rate_bitmap = 0x00000000000f0000;
			else if (rssi_level == DM_RATR_STA_MIDDLE)
				rate_bitmap = 0x00000000000ff000;
			else {
				if (*(pDM_Odm->pBandWidth) == ODM_BW40M)
					rate_bitmap = 0x00000000000ff015;
				else
					rate_bitmap = 0x00000000000ff005;
			}
		} else if (pDM_Odm->RFType == ODM_2T2R  || pDM_Odm->RFType == ODM_2T3R  || pDM_Odm->RFType == ODM_2T4R) {
			if (rssi_level == DM_RATR_STA_HIGH)
				rate_bitmap = 0x000000000f8f0000;
			else if (rssi_level == DM_RATR_STA_MIDDLE)
				rate_bitmap = 0x000000000f8ff000;
			else {
				if (*(pDM_Odm->pBandWidth) == ODM_BW40M)
					rate_bitmap = 0x000000000f8ff015;
				else
					rate_bitmap = 0x000000000f8ff005;
			}
		} else {
			if (rssi_level == DM_RATR_STA_HIGH)
				rate_bitmap = 0x0000000f0f0f0000;
			else if (rssi_level == DM_RATR_STA_MIDDLE)
				rate_bitmap = 0x0000000fcfcfe000;
			else {
				if (*(pDM_Odm->pBandWidth) == ODM_BW40M)
					rate_bitmap = 0x0000000ffffff015;
				else
					rate_bitmap = 0x0000000ffffff005;
			}
		}
	}
	break;

	case (ODM_WM_AC|ODM_WM_G):
		if (rssi_level == 1)
			rate_bitmap = 0x00000000fc3f0000;
		else if (rssi_level == 2)
			rate_bitmap = 0x00000000fffff000;
		else
			rate_bitmap = 0x00000000ffffffff;
		break;

	case (ODM_WM_AC|ODM_WM_A):

		if (pDM_Odm->RFType == ODM_1T2R || pDM_Odm->RFType == ODM_1T1R) {
			if (rssi_level == 1)				/* add by Gary for ac-series */
				rate_bitmap = 0x00000000003f8000;
			else if (rssi_level == 2)
				rate_bitmap = 0x00000000003fe000;
			else
				rate_bitmap = 0x00000000003ff010;
		} else if (pDM_Odm->RFType == ODM_2T2R  || pDM_Odm->RFType == ODM_2T3R  || pDM_Odm->RFType == ODM_2T4R) {
			if (rssi_level == 1)				/* add by Gary for ac-series */
				rate_bitmap = 0x00000000fe3f8000;       /* VHT 2SS MCS3~9 */
			else if (rssi_level == 2)
				rate_bitmap = 0x00000000fffff000;       /* VHT 2SS MCS0~9 */
			else
				rate_bitmap = 0x00000000fffff010;       /* All */
		} else {
			if (rssi_level == 1)				/* add by Gary for ac-series */
				rate_bitmap = 0x000003f8fe3f8000ULL;       /* VHT 3SS MCS3~9 */
			else if (rssi_level == 2)
				rate_bitmap = 0x000003fffffff000ULL;       /* VHT3SS MCS0~9 */
			else
				rate_bitmap = 0x000003fffffff010ULL;       /* All */
		}
		break;

	default:
		if (pDM_Odm->RFType == ODM_1T2R || pDM_Odm->RFType == ODM_1T1R)
			rate_bitmap = 0x00000000000fffff;
		else if (pDM_Odm->RFType == ODM_2T2R  || pDM_Odm->RFType == ODM_2T3R  || pDM_Odm->RFType == ODM_2T4R)
			rate_bitmap = 0x000000000fffffff;
		else
			rate_bitmap = 0x0000003fffffffffULL;
		break;

	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, (" ==> rssi_level:0x%02x, WirelessMode:0x%02x, rate_bitmap:0x%016llx\n", rssi_level, WirelessMode, rate_bitmap));

	return (ra_mask & rate_bitmap);
}


u4Byte
ODM_Get_Rate_Bitmap(
	IN	PVOID		pDM_VOID,
	IN	u4Byte		macid,
	IN	u4Byte 		ra_mask,
	IN	u1Byte 		rssi_level
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PSTA_INFO_T   	pEntry;
	u4Byte 	rate_bitmap = 0;
	u1Byte 	WirelessMode;
	//u1Byte 	WirelessMode =*(pDM_Odm->pWirelessMode);


	pEntry = pDM_Odm->pODM_StaInfo[macid];
	if (!IS_STA_VALID(pEntry))
		return ra_mask;

	WirelessMode = pEntry->wireless_mode;

	switch (WirelessMode) {
	case ODM_WM_B:
		if (ra_mask & 0x0000000c)		//11M or 5.5M enable
			rate_bitmap = 0x0000000d;
		else
			rate_bitmap = 0x0000000f;
		break;

	case (ODM_WM_G):
	case (ODM_WM_A):
		if (rssi_level == DM_RATR_STA_HIGH)
			rate_bitmap = 0x00000f00;
		else
			rate_bitmap = 0x00000ff0;
		break;

	case (ODM_WM_B|ODM_WM_G):
		if (rssi_level == DM_RATR_STA_HIGH)
			rate_bitmap = 0x00000f00;
		else if (rssi_level == DM_RATR_STA_MIDDLE)
			rate_bitmap = 0x00000ff0;
		else
			rate_bitmap = 0x00000ff5;
		break;

	case (ODM_WM_B|ODM_WM_G|ODM_WM_N24G)	:
	case (ODM_WM_B|ODM_WM_N24G)	:
	case (ODM_WM_G|ODM_WM_N24G)	:
	case (ODM_WM_A|ODM_WM_N5G)	: {
		if (pDM_Odm->RFType == ODM_1T2R || pDM_Odm->RFType == ODM_1T1R) {
			if (rssi_level == DM_RATR_STA_HIGH)
				rate_bitmap = 0x000f0000;
			else if (rssi_level == DM_RATR_STA_MIDDLE)
				rate_bitmap = 0x000ff000;
			else {
				if (*(pDM_Odm->pBandWidth) == ODM_BW40M)
					rate_bitmap = 0x000ff015;
				else
					rate_bitmap = 0x000ff005;
			}
		} else {
			if (rssi_level == DM_RATR_STA_HIGH)
				rate_bitmap = 0x0f8f0000;
			else if (rssi_level == DM_RATR_STA_MIDDLE)
				rate_bitmap = 0x0f8ff000;
			else {
				if (*(pDM_Odm->pBandWidth) == ODM_BW40M)
					rate_bitmap = 0x0f8ff015;
				else
					rate_bitmap = 0x0f8ff005;
			}
		}
	}
	break;

	case (ODM_WM_AC|ODM_WM_G):
		if (rssi_level == 1)
			rate_bitmap = 0xfc3f0000;
		else if (rssi_level == 2)
			rate_bitmap = 0xfffff000;
		else
			rate_bitmap = 0xffffffff;
		break;

	case (ODM_WM_AC|ODM_WM_A):

		if (pDM_Odm->RFType == RF_1T1R) {
			if (rssi_level == 1)				// add by Gary for ac-series
				rate_bitmap = 0x003f8000;
			else if (rssi_level == 2)
				rate_bitmap = 0x003ff000;
			else
				rate_bitmap = 0x003ff010;
		} else {
			if (rssi_level == 1)				// add by Gary for ac-series
				rate_bitmap = 0xfe3f8000;       // VHT 2SS MCS3~9
			else if (rssi_level == 2)
				rate_bitmap = 0xfffff000;       // VHT 2SS MCS0~9
			else
				rate_bitmap = 0xfffff010;       // All
		}
		break;

	default:
		if (pDM_Odm->RFType == RF_1T2R)
			rate_bitmap = 0x000fffff;
		else
			rate_bitmap = 0x0fffffff;
		break;

	}

	DBG_871X("%s ==> rssi_level:0x%02x, WirelessMode:0x%02x, rate_bitmap:0x%08x\n", __func__, rssi_level, WirelessMode, rate_bitmap);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, (" ==> rssi_level:0x%02x, WirelessMode:0x%02x, rate_bitmap:0x%08x\n", rssi_level, WirelessMode, rate_bitmap));

	return (ra_mask & rate_bitmap);

}

