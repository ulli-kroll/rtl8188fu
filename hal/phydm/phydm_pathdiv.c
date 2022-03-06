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

#if(defined(CONFIG_PATH_DIVERSITY))

VOID
odm_pathdiv_debug(
	IN		PVOID		pDM_VOID,
	IN		u4Byte		*const dm_value,
	IN		u4Byte		*_used,
	OUT		char			*output,
	IN		u4Byte		*_out_len
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pPATHDIV_T			pDM_PathDiv  = &(pDM_Odm->DM_PathDiv);
	u4Byte used = *_used;
	u4Byte out_len = *_out_len;
	
	pDM_Odm->path_select = (dm_value[0] & 0xf);
	PHYDM_SNPRINTF((output+used, out_len-used,"Path_select = (( 0x%x ))\n",pDM_Odm->path_select ));
	
	//2 [Fix Path]
	if (pDM_Odm->path_select != PHYDM_AUTO_PATH)
	{
		PHYDM_SNPRINTF((output+used, out_len-used,"Trun on path  [%s%s%s%s]\n",
			((pDM_Odm->path_select) & 0x1)?"A":"",
			((pDM_Odm->path_select) & 0x2)?"B":"",
			((pDM_Odm->path_select) & 0x4)?"C":"",
			((pDM_Odm->path_select) & 0x8)?"D":"" ));
		
		phydm_dtp_fix_tx_path( pDM_Odm, pDM_Odm->path_select );
	}
	else
	{
		PHYDM_SNPRINTF((output+used, out_len-used,"%s\n","Auto Path"));
	}
}

#endif // #if(defined(CONFIG_PATH_DIVERSITY))

VOID
phydm_c2h_dtp_handler(
 IN	PVOID	pDM_VOID,
 IN 	pu1Byte   CmdBuf,
 IN 	u1Byte	CmdLen
)
{
#if(defined(CONFIG_PATH_DIVERSITY))
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pPATHDIV_T		pDM_PathDiv  = &(pDM_Odm->DM_PathDiv);

	u1Byte  macid = CmdBuf[0]; 
	u1Byte  target = CmdBuf[1];	
	u1Byte  nsc_1 = CmdBuf[2];
	u1Byte  nsc_2 = CmdBuf[3];
	u1Byte  nsc_3 = CmdBuf[4];

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_PATH_DIV,ODM_DBG_LOUD,("Target_candidate = (( %d ))\n", target));
	/*
	if( (nsc_1 >= nsc_2) &&  (nsc_1 >= nsc_3))
	{
		phydm_dtp_fix_tx_path(pDM_Odm, pDM_PathDiv->ant_candidate_1);
	}
	else 	if( nsc_2 >= nsc_3)
	{
		phydm_dtp_fix_tx_path(pDM_Odm, pDM_PathDiv->ant_candidate_2);
	}
	else
	{
		phydm_dtp_fix_tx_path(pDM_Odm, pDM_PathDiv->ant_candidate_3);	
	}
	*/
#endif	
}

VOID
odm_PathDiversity(
	IN	PVOID	pDM_VOID
)
{
#if(defined(CONFIG_PATH_DIVERSITY))
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	if(!(pDM_Odm->SupportAbility & ODM_BB_PATH_DIV))
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_PATH_DIV,ODM_DBG_LOUD,("Return: Not Support PathDiv\n"));
		return;
	}

    		{}
#endif
}

VOID
odm_PathDiversityInit(
	IN	PVOID	pDM_VOID
)
{
#if(defined(CONFIG_PATH_DIVERSITY))
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;

	/*pDM_Odm->SupportAbility |= ODM_BB_PATH_DIV;*/
	
	if(pDM_Odm->mp_mode == TRUE)
		return;

	if(!(pDM_Odm->SupportAbility & ODM_BB_PATH_DIV))
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_PATH_DIV,ODM_DBG_LOUD,("Return: Not Support PathDiv\n"));
		return;
	}

#endif
}

