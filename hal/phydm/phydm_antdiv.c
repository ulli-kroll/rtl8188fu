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

//======================================================
// when antenna test utility is on or some testing need to disable antenna diversity
// call this function to disable all ODM related mechanisms which will switch antenna.
//======================================================

VOID
ODM_SetAntConfig(
	IN	PVOID	pDM_VOID,
	IN	u1Byte		antSetting	// 0=A, 1=B, 2=C, ....
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
}

//======================================================


VOID
ODM_SwAntDivRestAfterLink(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pSWAT_T		pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	pFAT_T		pDM_FatTable = &pDM_Odm->DM_FatTable;
	u4Byte             i;

	if (pDM_Odm->AntDivType == S0S1_SW_ANTDIV) {
		
		pDM_SWAT_Table->try_flag = SWAW_STEP_INIT;
		pDM_SWAT_Table->RSSI_Trying = 0;
		pDM_SWAT_Table->Double_chk_flag= 0;
		
		pDM_FatTable->RxIdleAnt=MAIN_ANT;
		
		for (i=0; i<ODM_ASSOCIATE_ENTRY_NUM; i++)
		{
			pDM_FatTable->MainAnt_Sum[i] = 0;
			pDM_FatTable->AuxAnt_Sum[i] = 0;
			pDM_FatTable->MainAnt_Cnt[i] = 0;
			pDM_FatTable->AuxAnt_Cnt[i] = 0;
		}

	}
}


#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
VOID
odm_AntDiv_on_off( 
	IN 	PVOID		pDM_VOID ,
	IN 	u1Byte 		swch
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
	
	if(pDM_FatTable->AntDiv_OnOff != swch)
	{
		if (pDM_Odm->AntDivType == S0S1_SW_ANTDIV) 
			return;

		if(pDM_Odm->SupportICType & ODM_N_ANTDIV_SUPPORT)
		{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("(( Turn %s )) N-Series HW-AntDiv block\n",(swch==ANTDIV_ON)?"ON" : "OFF"));
			ODM_SetBBReg(pDM_Odm, 0xc50 , BIT7, swch); //OFDM AntDiv function block enable
			ODM_SetBBReg(pDM_Odm, 0xa00 , BIT15, swch); //CCK AntDiv function block enable
		}
	}
	pDM_FatTable->AntDiv_OnOff =swch;
	
}

VOID
phydm_FastTraining_enable(
	IN		PVOID		pDM_VOID, 
	IN 		u1Byte  			swch
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u1Byte			enable;

	if (swch == FAT_ON)
		enable=1;
	else
		enable=0;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Fast Ant Training_en = ((%d))\n", enable));

	if (pDM_Odm->SupportICType == ODM_RTL8188E) {
		ODM_SetBBReg(pDM_Odm, 0xe08 , BIT16, enable);	/*enable fast training*/
		/**/
	} else if (pDM_Odm->SupportICType == ODM_RTL8192E) {
		ODM_SetBBReg(pDM_Odm, 0xB34 , BIT28, enable);	/*enable fast training (path-A)*/
		/*ODM_SetBBReg(pDM_Odm, 0xB34 , BIT29, enable);*/	/*enable fast training (path-B)*/
	} else if (pDM_Odm->SupportICType == ODM_RTL8821) {
		ODM_SetBBReg(pDM_Odm, 0x900 , BIT19, enable);	/*enable fast training */
		/**/
	}
}

VOID
odm_Tx_By_TxDesc_or_Reg( 
	IN 		PVOID		pDM_VOID, 
	IN 		u1Byte 			swch
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u1Byte enable;

	enable = (swch == TX_BY_DESC) ? 1 : 0;

	if(pDM_Odm->AntDivType != CGCS_RX_HW_ANTDIV)
	{
		if(pDM_Odm->SupportICType & ODM_N_ANTDIV_SUPPORT)
		{
				ODM_SetBBReg(pDM_Odm, 0x80c , BIT21, enable); 
		}	
	}
}

VOID
ODM_UpdateRxIdleAnt(
	IN 		PVOID 		pDM_VOID, 
	IN 		u1Byte 		Ant
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
	u4Byte	DefaultAnt, OptionalAnt,value32;

	if(pDM_FatTable->RxIdleAnt != Ant)
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Update Rx-Idle-Ant ] RxIdleAnt =%s\n",(Ant==MAIN_ANT)?"MAIN_ANT":"AUX_ANT"));

		if(!(pDM_Odm->SupportICType & ODM_RTL8723B))
                	pDM_FatTable->RxIdleAnt = Ant;

		if(Ant == MAIN_ANT)
		{
			DefaultAnt   =  ANT1_2G; 
			OptionalAnt =  ANT2_2G; 
		}
		else
		{
			DefaultAnt  =   ANT2_2G;
			OptionalAnt =  ANT1_2G;
		}
	
		if(pDM_Odm->SupportICType & ODM_N_ANTDIV_SUPPORT)
		{
			if(pDM_Odm->SupportICType==ODM_RTL8192E)
			{
				ODM_SetBBReg(pDM_Odm, 0xB38 , BIT5|BIT4|BIT3, DefaultAnt); //Default RX
				ODM_SetBBReg(pDM_Odm, 0xB38 , BIT8|BIT7|BIT6, OptionalAnt);//Optional RX
				ODM_SetBBReg(pDM_Odm, 0x860, BIT14|BIT13|BIT12, DefaultAnt);//Default TX	
			}
			#if (RTL8723B_SUPPORT == 1)
			else if (pDM_Odm->SupportICType == ODM_RTL8723B) {
				
				value32 = ODM_GetBBReg(pDM_Odm, 0x948, 0xFFF);
			
				if(value32 !=0x280)
					ODM_UpdateRxIdleAnt_8723B(pDM_Odm, Ant, DefaultAnt, OptionalAnt);
				else {
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Update Rx-Idle-Ant ] 8723B: Fail to set RX antenna due to 0x948 = 0x280\n"));
					/**/
				}
			}
			#endif
			else { /*8188E & 8188F*/

				#if (RTL8188F_SUPPORT == 1)
				if (pDM_Odm->SupportICType == ODM_RTL8188F) {
					phydm_update_rx_idle_antenna_8188F(pDM_Odm, DefaultAnt);
					/**/
				}
				#endif
				
				ODM_SetBBReg(pDM_Odm, 0x864 , BIT5|BIT4|BIT3, DefaultAnt);		/*Default RX*/
				ODM_SetBBReg(pDM_Odm, 0x864 , BIT8|BIT7|BIT6, OptionalAnt);	/*Optional RX*/
				ODM_SetBBReg(pDM_Odm, 0x860, BIT14|BIT13|BIT12, DefaultAnt);	/*Default TX*/
			}
		}

		if(pDM_Odm->SupportICType==ODM_RTL8188E)
		{		
			ODM_SetMACReg(pDM_Odm, 0x6D8 , BIT7|BIT6, DefaultAnt);	//PathA Resp Tx
		}
		else
		{
			ODM_SetMACReg(pDM_Odm, 0x6D8 , BIT10|BIT9|BIT8, DefaultAnt);	//PathA Resp Tx
		}	

	}
	else// pDM_FatTable->RxIdleAnt == Ant
        {
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Stay in Ori-Ant ]  RxIdleAnt =%s\n",(Ant==MAIN_ANT)?"MAIN_ANT":"AUX_ANT"));
		pDM_FatTable->RxIdleAnt = Ant;
	}
}

VOID
odm_UpdateTxAnt(
	IN 		PVOID 		pDM_VOID, 
	IN 		u1Byte 		Ant, 
	IN 		u4Byte 		MacId
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
	u1Byte	TxAnt;

	if (pDM_Odm->AntDivType==CG_TRX_SMART_ANTDIV)
	{
		TxAnt=Ant;
	}
	else
	{
        	if(Ant == MAIN_ANT)
        		TxAnt = ANT1_2G;
        	else
        		TxAnt = ANT2_2G;
	}
	
	pDM_FatTable->antsel_a[MacId] = TxAnt&BIT0;
	pDM_FatTable->antsel_b[MacId] = (TxAnt&BIT1)>>1;
	pDM_FatTable->antsel_c[MacId] = (TxAnt&BIT2)>>2;
	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Tx from TxInfo]: MacID:(( %d )),  TxAnt = (( %s ))\n", MacId, (Ant == MAIN_ANT)?"MAIN_ANT":"AUX_ANT"));
	//ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("antsel_tr_mux=(( 3'b%d%d%d ))\n",pDM_FatTable->antsel_c[MacId] , pDM_FatTable->antsel_b[MacId] , pDM_FatTable->antsel_a[MacId] ));
	
}

#ifdef BEAMFORMING_SUPPORT
#endif //#ifdef BEAMFORMING_SUPPORT


#if (RTL8188F_SUPPORT == 1)
VOID
odm_S0S1_SWAntDiv_Init_8188F(
	IN		PVOID		pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pSWAT_T		pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	pFAT_T		pDM_FatTable = &pDM_Odm->DM_FatTable;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***8188F AntDiv_Init => AntDivType=[ S0S1_SW_AntDiv]\n"));


	/*GPIO Setting*/
	/*ODM_SetMACReg(pDM_Odm, 0x64 , BIT18, 0); */
	/*ODM_SetMACReg(pDM_Odm, 0x44 , BIT28|BIT27, 0);*/
	ODM_SetMACReg(pDM_Odm, 0x44 , BIT20|BIT19, 0x3);	/*enable_output for P_GPIO[4:3]*/
	/*ODM_SetMACReg(pDM_Odm, 0x44 , BIT12|BIT11, 0);*/ /*output value*/
	/*ODM_SetMACReg(pDM_Odm, 0x40 , BIT1|BIT0, 0);*/		/*GPIO function*/

	pDM_FatTable->bBecomeLinked  = FALSE;
	pDM_SWAT_Table->try_flag = SWAW_STEP_INIT;	
	pDM_SWAT_Table->Double_chk_flag = 0;
}

VOID
phydm_update_rx_idle_antenna_8188F(
	IN	PVOID	pDM_VOID,
	IN	u4Byte	default_ant
)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u1Byte		codeword;

	if (default_ant == ANT1_2G)
		codeword = 1; /*2'b01*/
	else
		codeword = 2;/*2'b10*/
	
	ODM_SetMACReg(pDM_Odm, 0x44 , (BIT12|BIT11), codeword); /*GPIO[4:3] output value*/
}

#endif



#ifdef ODM_EVM_ENHANCE_ANTDIV

VOID
odm_EVM_FastAnt_Reset(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
		
	pDM_FatTable->EVM_method_enable=0;
	odm_AntDiv_on_off(pDM_Odm, ANTDIV_ON);
	pDM_FatTable->FAT_State = NORMAL_STATE_MIAN;
	pDM_Odm->antdiv_period=0;
	ODM_SetMACReg(pDM_Odm, 0x608, BIT8, 0);
}


VOID
odm_EVM_Enhance_AntDiv(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u4Byte	Main_RSSI, Aux_RSSI ;
	u4Byte	Main_CRC_utility=0,Aux_CRC_utility=0,utility_ratio=1;
	u4Byte	Main_EVM, Aux_EVM,Diff_RSSI=0,diff_EVM=0;	
	u1Byte	score_EVM=0,score_CRC=0;
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
	u4Byte	value32, i;
	BOOLEAN Main_above1=FALSE,Aux_above1=FALSE;
	BOOLEAN Force_antenna=FALSE;
	PSTA_INFO_T   	pEntry;
	pDM_FatTable->TargetAnt_enhance=0xFF;
	
	
	if((pDM_Odm->SupportICType & ODM_EVM_ENHANCE_ANTDIV_SUPPORT_IC))
	{
		if(pDM_Odm->bOneEntryOnly)
		{
			//ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[One Client only] \n"));
			i = pDM_Odm->OneEntry_MACID;

			Main_RSSI = (pDM_FatTable->MainAnt_Cnt[i]!=0)?(pDM_FatTable->MainAnt_Sum[i]/pDM_FatTable->MainAnt_Cnt[i]):0;
			Aux_RSSI = (pDM_FatTable->AuxAnt_Cnt[i]!=0)?(pDM_FatTable->AuxAnt_Sum[i]/pDM_FatTable->AuxAnt_Cnt[i]):0;

			if((Main_RSSI==0 && Aux_RSSI !=0 && Aux_RSSI>=FORCE_RSSI_DIFF) || (Main_RSSI!=0 && Aux_RSSI==0 && Main_RSSI>=FORCE_RSSI_DIFF))
			{
				Diff_RSSI=FORCE_RSSI_DIFF;
			}
			else if(Main_RSSI!=0 && Aux_RSSI !=0)
			{
				Diff_RSSI = (Main_RSSI>=Aux_RSSI)?(Main_RSSI-Aux_RSSI):(Aux_RSSI-Main_RSSI); 
			}
			

			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, (" Main_Cnt = (( %d ))  , Main_RSSI= ((  %d )) \n", pDM_FatTable->MainAnt_Cnt[i], Main_RSSI));
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, (" Aux_Cnt   = (( %d ))  , Aux_RSSI = ((  %d )) \n" , pDM_FatTable->AuxAnt_Cnt[i] , Aux_RSSI));
						
			if(  ((Main_RSSI>=Evm_RSSI_TH_High||Aux_RSSI>=Evm_RSSI_TH_High )|| (pDM_FatTable->EVM_method_enable==1)  )
				//&& (Diff_RSSI <= FORCE_RSSI_DIFF + 1)
                                    )
			{
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[> TH_H || EVM_method_enable==1]  && "));
				
				if(((Main_RSSI>=Evm_RSSI_TH_Low)||(Aux_RSSI>=Evm_RSSI_TH_Low) ))
				{
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[> TH_L ] \n"));

					//2 [ Normal state Main]
					if(pDM_FatTable->FAT_State == NORMAL_STATE_MIAN)
					{

						pDM_FatTable->EVM_method_enable=1;
						odm_AntDiv_on_off(pDM_Odm, ANTDIV_OFF);
						pDM_Odm->antdiv_period=3;

						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ start training: MIAN] \n"));
						pDM_FatTable->MainAntEVM_Sum[i] = 0;
						pDM_FatTable->AuxAntEVM_Sum[i] = 0;
						pDM_FatTable->MainAntEVM_Cnt[i] = 0;
						pDM_FatTable->AuxAntEVM_Cnt[i] = 0;

						pDM_FatTable->FAT_State = NORMAL_STATE_AUX;
						ODM_SetMACReg(pDM_Odm, 0x608, BIT8, 1); //Accept CRC32 Error packets.
						ODM_UpdateRxIdleAnt(pDM_Odm, MAIN_ANT);
						
						pDM_FatTable->CRC32_Ok_Cnt=0;
						pDM_FatTable->CRC32_Fail_Cnt=0;
						ODM_SetTimer(pDM_Odm,&pDM_Odm->EVM_FastAntTrainingTimer, pDM_Odm->antdiv_intvl ); //m
					}
					//2 [ Normal state Aux ]
					else if(pDM_FatTable->FAT_State == NORMAL_STATE_AUX)
					{
						pDM_FatTable->MainCRC32_Ok_Cnt=pDM_FatTable->CRC32_Ok_Cnt;
						pDM_FatTable->MainCRC32_Fail_Cnt=pDM_FatTable->CRC32_Fail_Cnt;
						
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ start training: AUX] \n"));
						pDM_FatTable->FAT_State = TRAINING_STATE;
						ODM_UpdateRxIdleAnt(pDM_Odm, AUX_ANT);

						pDM_FatTable->CRC32_Ok_Cnt=0;
						pDM_FatTable->CRC32_Fail_Cnt=0;
						ODM_SetTimer(pDM_Odm,&pDM_Odm->EVM_FastAntTrainingTimer, pDM_Odm->antdiv_intvl ); //ms
					}					
					else if(pDM_FatTable->FAT_State == TRAINING_STATE)
					{
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Training state ] \n"));
						pDM_FatTable->FAT_State = NORMAL_STATE_MIAN;
						
						//3 [CRC32 statistic]
						pDM_FatTable->AuxCRC32_Ok_Cnt=pDM_FatTable->CRC32_Ok_Cnt;
						pDM_FatTable->AuxCRC32_Fail_Cnt=pDM_FatTable->CRC32_Fail_Cnt;

						if( (pDM_FatTable->MainCRC32_Ok_Cnt  >= ((pDM_FatTable->AuxCRC32_Ok_Cnt)<<1)) || (Diff_RSSI>=18))
						{
							pDM_FatTable->TargetAnt_CRC32=MAIN_ANT;
							Force_antenna=TRUE;
							ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("CRC32 Force Main \n"));
						}
						else if((pDM_FatTable->AuxCRC32_Ok_Cnt  >= ((pDM_FatTable->MainCRC32_Ok_Cnt)<<1)) || (Diff_RSSI>=18))
						{
							pDM_FatTable->TargetAnt_CRC32=AUX_ANT;
							Force_antenna=TRUE;
							ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("CRC32 Force Aux \n"));
						}
						else
						{
							if(pDM_FatTable->MainCRC32_Fail_Cnt<=5)
								pDM_FatTable->MainCRC32_Fail_Cnt=5;
							
							if(pDM_FatTable->AuxCRC32_Fail_Cnt<=5)
								pDM_FatTable->AuxCRC32_Fail_Cnt=5;
						
							if(pDM_FatTable->MainCRC32_Ok_Cnt >pDM_FatTable->MainCRC32_Fail_Cnt )
								Main_above1=TRUE;
						
							if(pDM_FatTable->AuxCRC32_Ok_Cnt >pDM_FatTable->AuxCRC32_Fail_Cnt )
								Aux_above1=TRUE;

							if(Main_above1==TRUE && Aux_above1==FALSE)
							{
								Force_antenna=TRUE;
								pDM_FatTable->TargetAnt_CRC32=MAIN_ANT;
							}
							else if(Main_above1==FALSE && Aux_above1==TRUE)
							{
								Force_antenna=TRUE;
								pDM_FatTable->TargetAnt_CRC32=AUX_ANT;
							}
							else if(Main_above1==TRUE && Aux_above1==TRUE)
							{
								Main_CRC_utility=((pDM_FatTable->MainCRC32_Ok_Cnt)<<7)/pDM_FatTable->MainCRC32_Fail_Cnt;
								Aux_CRC_utility=((pDM_FatTable->AuxCRC32_Ok_Cnt)<<7)/pDM_FatTable->AuxCRC32_Fail_Cnt;
								pDM_FatTable->TargetAnt_CRC32 = (Main_CRC_utility==Aux_CRC_utility)?(pDM_FatTable->pre_TargetAnt_enhance):((Main_CRC_utility>=Aux_CRC_utility)?MAIN_ANT:AUX_ANT);
								
								if(Main_CRC_utility!=0 && Aux_CRC_utility!=0)
								{
									if(Main_CRC_utility>=Aux_CRC_utility)
										utility_ratio=(Main_CRC_utility<<1)/Aux_CRC_utility;
									else
										utility_ratio=(Aux_CRC_utility<<1)/Main_CRC_utility;
								}
							}
							else if(Main_above1==FALSE && Aux_above1==FALSE)
							{
								if(pDM_FatTable->MainCRC32_Ok_Cnt==0)
									pDM_FatTable->MainCRC32_Ok_Cnt=1;
								if(pDM_FatTable->AuxCRC32_Ok_Cnt==0)
									pDM_FatTable->AuxCRC32_Ok_Cnt=1;
								
								Main_CRC_utility=((pDM_FatTable->MainCRC32_Fail_Cnt)<<7)/pDM_FatTable->MainCRC32_Ok_Cnt;
								Aux_CRC_utility=((pDM_FatTable->AuxCRC32_Fail_Cnt)<<7)/pDM_FatTable->AuxCRC32_Ok_Cnt;
								pDM_FatTable->TargetAnt_CRC32 = (Main_CRC_utility==Aux_CRC_utility)?(pDM_FatTable->pre_TargetAnt_enhance):((Main_CRC_utility<=Aux_CRC_utility)?MAIN_ANT:AUX_ANT);	

								if(Main_CRC_utility!=0 && Aux_CRC_utility!=0)
								{
									if(Main_CRC_utility>=Aux_CRC_utility)
										utility_ratio=(Main_CRC_utility<<1)/(Aux_CRC_utility);
									else
										utility_ratio=(Aux_CRC_utility<<1)/(Main_CRC_utility);
								}
							}
						}
						ODM_SetMACReg(pDM_Odm, 0x608, BIT8, 0);//NOT Accept CRC32 Error packets.

						//3 [EVM statistic]			
						Main_EVM = (pDM_FatTable->MainAntEVM_Cnt[i]!=0)?(pDM_FatTable->MainAntEVM_Sum[i]/pDM_FatTable->MainAntEVM_Cnt[i]):0;
						Aux_EVM = (pDM_FatTable->AuxAntEVM_Cnt[i]!=0)?(pDM_FatTable->AuxAntEVM_Sum[i]/pDM_FatTable->AuxAntEVM_Cnt[i]):0;
						pDM_FatTable->TargetAnt_EVM = (Main_EVM==Aux_EVM)?(pDM_FatTable->pre_TargetAnt_enhance):((Main_EVM>=Aux_EVM)?MAIN_ANT:AUX_ANT);

						if((Main_EVM==0 || Aux_EVM==0))
							diff_EVM=0;
						else if(Main_EVM>=Aux_EVM)
							diff_EVM=Main_EVM-Aux_EVM;
						else
							diff_EVM=Aux_EVM-Main_EVM;

						//2 [ Decision state ]					
						if(pDM_FatTable->TargetAnt_EVM ==pDM_FatTable->TargetAnt_CRC32 )
						{
							if( (utility_ratio<2 && Force_antenna==FALSE)  && diff_EVM<=2)
								pDM_FatTable->TargetAnt_enhance=pDM_FatTable->pre_TargetAnt_enhance;
							else
								pDM_FatTable->TargetAnt_enhance=pDM_FatTable->TargetAnt_EVM;
						}
						else if(diff_EVM<=2 && (utility_ratio > 4 && Force_antenna==FALSE)) 
						{
							pDM_FatTable->TargetAnt_enhance=pDM_FatTable->TargetAnt_CRC32;
						}
						else if(diff_EVM>=20) // 
						{
							pDM_FatTable->TargetAnt_enhance=pDM_FatTable->TargetAnt_EVM;
						}
						else if(utility_ratio>=6 && Force_antenna==FALSE) // utility_ratio>3
						{
							pDM_FatTable->TargetAnt_enhance=pDM_FatTable->TargetAnt_CRC32;
						}
						else
						{
							if(Force_antenna==TRUE)
								score_CRC=3;
							else if(utility_ratio>=4) //>2
								score_CRC=2;
					else if(utility_ratio>=3) //>1.5
						score_CRC=1;
					else
						score_CRC=0;
					
					if(diff_EVM>=10)
						score_EVM=2;
					else if(diff_EVM>=5)
						score_EVM=1;
					else
						score_EVM=0;

					if(score_CRC>score_EVM)
						pDM_FatTable->TargetAnt_enhance=pDM_FatTable->TargetAnt_CRC32;
					else if(score_CRC<score_EVM)
						pDM_FatTable->TargetAnt_enhance=pDM_FatTable->TargetAnt_EVM;
					else
						pDM_FatTable->TargetAnt_enhance=pDM_FatTable->pre_TargetAnt_enhance;
				}
				pDM_FatTable->pre_TargetAnt_enhance=pDM_FatTable->TargetAnt_enhance;

						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** Client[ %d ] : MainEVM_Cnt = (( %d ))  , Main_EVM= ((  %d )) \n",i, pDM_FatTable->MainAntEVM_Cnt[i], Main_EVM));
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** Client[ %d ] : AuxEVM_Cnt   = (( %d ))  , Aux_EVM = ((  %d )) \n" ,i, pDM_FatTable->AuxAntEVM_Cnt[i] , Aux_EVM));
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** TargetAnt_EVM = (( %s ))\n", ( pDM_FatTable->TargetAnt_EVM  ==MAIN_ANT)?"MAIN_ANT":"AUX_ANT"));
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("M_CRC_Ok = (( %d ))  , M_CRC_Fail = ((  %d )), Main_CRC_utility = (( %d )) \n" , pDM_FatTable->MainCRC32_Ok_Cnt, pDM_FatTable->MainCRC32_Fail_Cnt,Main_CRC_utility));
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("A_CRC_Ok  = (( %d ))  , A_CRC_Fail = ((  %d )), Aux_CRC_utility   = ((  %d )) \n" , pDM_FatTable->AuxCRC32_Ok_Cnt, pDM_FatTable->AuxCRC32_Fail_Cnt,Aux_CRC_utility));
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** TargetAnt_CRC32 = (( %s ))\n", ( pDM_FatTable->TargetAnt_CRC32 ==MAIN_ANT)?"MAIN_ANT":"AUX_ANT"));
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("****** TargetAnt_enhance = (( %s ))******\n", ( pDM_FatTable->TargetAnt_enhance ==MAIN_ANT)?"MAIN_ANT":"AUX_ANT"));
				
					
					}
				}
				else // RSSI< = Evm_RSSI_TH_Low
				{ 
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ <TH_L: escape from > TH_L ] \n"));
					odm_EVM_FastAnt_Reset(pDM_Odm);
				}
			}
			else 
			{ 
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[escape from> TH_H || EVM_method_enable==1] \n"));
				odm_EVM_FastAnt_Reset(pDM_Odm);
			}
		}
		else
		{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[multi-Client] \n"));
			odm_EVM_FastAnt_Reset(pDM_Odm);
		}			
	}
}

VOID
odm_EVM_FastAntTrainingCallback(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("******odm_EVM_FastAntTrainingCallback****** \n"));
	odm_HW_AntDiv(pDM_Odm);
}
#endif

VOID
odm_HW_AntDiv(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u4Byte	i,MinMaxRSSI=0xFF,  AntDivMaxRSSI=0, MaxRSSI=0, LocalMaxRSSI;
	u4Byte	Main_RSSI, Aux_RSSI;
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;	
	u1Byte	RxIdleAnt = pDM_FatTable->RxIdleAnt, TargetAnt = 7;
	pDIG_T	pDM_DigTable = &pDM_Odm->DM_DigTable;
	PSTA_INFO_T   	pEntry;

	#ifdef BEAMFORMING_SUPPORT
	#endif

	if(!pDM_Odm->bLinked) //bLinked==False
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[No Link!!!]\n"));
		
		if(pDM_FatTable->bBecomeLinked == TRUE)
		{
			odm_AntDiv_on_off(pDM_Odm, ANTDIV_OFF);
			ODM_UpdateRxIdleAnt(pDM_Odm, MAIN_ANT);
			odm_Tx_By_TxDesc_or_Reg(pDM_Odm, TX_BY_REG);
			pDM_Odm->antdiv_period=0;

			pDM_FatTable->bBecomeLinked = pDM_Odm->bLinked;
		}
		return;
	}	
	else
	{
		if(pDM_FatTable->bBecomeLinked ==FALSE)
		{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Linked !!!]\n"));
			odm_AntDiv_on_off(pDM_Odm, ANTDIV_ON);
			
			//if(pDM_Odm->SupportICType == ODM_RTL8821 )
				//ODM_SetBBReg(pDM_Odm, 0x800 , BIT25, 0); //CCK AntDiv function disable
				
			
			//else if(pDM_Odm->SupportICType == ODM_RTL8723B ||pDM_Odm->SupportICType == ODM_RTL8812)
				//ODM_SetBBReg(pDM_Odm, 0xA00 , BIT15, 0); //CCK AntDiv function disable
			
			pDM_FatTable->bBecomeLinked = pDM_Odm->bLinked;

			if(pDM_Odm->SupportICType==ODM_RTL8723B && pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV)
			{
				ODM_SetBBReg(pDM_Odm, 0x930 , 0xF0, 8); // DPDT_P = ANTSEL[0]   // for 8723B AntDiv function patch.  BB  Dino  130412	
				ODM_SetBBReg(pDM_Odm, 0x930 , 0xF, 8); // DPDT_N = ANTSEL[0]
			}
			
			//2 BDC Init
			#ifdef BEAMFORMING_SUPPORT
			#endif
			
			#ifdef ODM_EVM_ENHANCE_ANTDIV
				odm_EVM_FastAnt_Reset(pDM_Odm);
			#endif
		}	
	}	

	if (pDM_Odm->bOneEntryOnly == TRUE)
		odm_Tx_By_TxDesc_or_Reg(pDM_Odm, TX_BY_REG);
	else
		odm_Tx_By_TxDesc_or_Reg(pDM_Odm, TX_BY_DESC);
		

	//ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("\n AntDiv Start =>\n"));

	#ifdef ODM_EVM_ENHANCE_ANTDIV
	if(pDM_Odm->antdiv_evm_en==1)
	{
		odm_EVM_Enhance_AntDiv(pDM_Odm);
		if(pDM_FatTable->FAT_State !=NORMAL_STATE_MIAN)
			return;
	}
	else
	{
		odm_EVM_FastAnt_Reset(pDM_Odm);
	}
	#endif
	
	//2 BDC Mode Arbitration
	#ifdef BEAMFORMING_SUPPORT
	#endif

	for (i=0; i<ODM_ASSOCIATE_ENTRY_NUM; i++)
	{
		pEntry = pDM_Odm->pODM_StaInfo[i];
		if(IS_STA_VALID(pEntry))
		{
			//2 Caculate RSSI per Antenna
			Main_RSSI = (pDM_FatTable->MainAnt_Cnt[i]!=0)?(pDM_FatTable->MainAnt_Sum[i]/pDM_FatTable->MainAnt_Cnt[i]):0;
			Aux_RSSI = (pDM_FatTable->AuxAnt_Cnt[i]!=0)?(pDM_FatTable->AuxAnt_Sum[i]/pDM_FatTable->AuxAnt_Cnt[i]):0;
			TargetAnt = (Main_RSSI==Aux_RSSI)?pDM_FatTable->RxIdleAnt:((Main_RSSI>=Aux_RSSI)?MAIN_ANT:AUX_ANT);
		
			//ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("*** SupportICType=[%d] \n",pDM_Odm->SupportICType));
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** Client[ %d ] : Main_Cnt = (( %d ))  , Main_RSSI= ((  %d )) \n",i, pDM_FatTable->MainAnt_Cnt[i], Main_RSSI));
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** Client[ %d ] : Aux_Cnt   = (( %d ))  , Aux_RSSI = ((  %d )) \n" ,i, pDM_FatTable->AuxAnt_Cnt[i] , Aux_RSSI));
			//ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** MAC ID:[ %d ] , TargetAnt = (( %s )) \n", i ,( TargetAnt ==MAIN_ANT)?"MAIN_ANT":"AUX_ANT"));
			//ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("*** Phy_AntSel_A=[ %d, %d, %d] \n",((pDM_Odm->DM_FatTable.antsel_rx_keep_0)&BIT2)>>2,
			//	                                                                              ((pDM_Odm->DM_FatTable.antsel_rx_keep_0)&BIT1) >>1, ((pDM_Odm->DM_FatTable.antsel_rx_keep_0)&BIT0)));

			LocalMaxRSSI = (Main_RSSI>Aux_RSSI)?Main_RSSI:Aux_RSSI;
			//2 Select MaxRSSI for DIG
			if((LocalMaxRSSI > AntDivMaxRSSI) && (LocalMaxRSSI < 40))
				AntDivMaxRSSI = LocalMaxRSSI;
			if(LocalMaxRSSI > MaxRSSI)
				MaxRSSI = LocalMaxRSSI;

			//2 Select RX Idle Antenna
			if ( (LocalMaxRSSI != 0) &&  (LocalMaxRSSI < MinMaxRSSI) )
			{
				RxIdleAnt = TargetAnt;
				MinMaxRSSI = LocalMaxRSSI;
			}

			#ifdef ODM_EVM_ENHANCE_ANTDIV
			if(pDM_Odm->antdiv_evm_en==1)
			{
				if(pDM_FatTable->TargetAnt_enhance!=0xFF)
				{
					TargetAnt=pDM_FatTable->TargetAnt_enhance;
					RxIdleAnt = pDM_FatTable->TargetAnt_enhance;
				}
			}
			#endif

			//2 Select TX Antenna
			if(pDM_Odm->AntDivType != CGCS_RX_HW_ANTDIV)
			{
				#ifdef BEAMFORMING_SUPPORT
				#endif
					{
						odm_UpdateTxAnt(pDM_Odm, TargetAnt, i);
					}
			}

			//------------------------------------------------------------

			#ifdef BEAMFORMING_SUPPORT
			#endif

		}

		#ifdef BEAMFORMING_SUPPORT
		#endif	
		{
        		pDM_FatTable->MainAnt_Sum[i] = 0;
        		pDM_FatTable->AuxAnt_Sum[i] = 0;
        		pDM_FatTable->MainAnt_Cnt[i] = 0;
         		pDM_FatTable->AuxAnt_Cnt[i] = 0;
                }
	}
       

	
	//2 Set RX Idle Antenna & TX Antenna(Because of HW Bug )	
	
		ODM_UpdateRxIdleAnt(pDM_Odm, RxIdleAnt);
	



	//2 BDC Main Algorithm
	#ifdef BEAMFORMING_SUPPORT
	#endif

	if(AntDivMaxRSSI == 0)
		pDM_DigTable->AntDiv_RSSI_max = pDM_Odm->RSSI_Min;
	else
		pDM_DigTable->AntDiv_RSSI_max = AntDivMaxRSSI;
	
	pDM_DigTable->RSSI_max = MaxRSSI;
}



#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY

VOID
odm_S0S1_SWAntDiv_Reset(
	IN		PVOID		pDM_VOID
)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pSWAT_T		pDM_SWAT_Table	= &pDM_Odm->DM_SWAT_Table;
	pFAT_T		pDM_FatTable		= &pDM_Odm->DM_FatTable;

	pDM_FatTable->bBecomeLinked  = FALSE;
	pDM_SWAT_Table->try_flag = SWAW_STEP_INIT;	
	pDM_SWAT_Table->Double_chk_flag = 0;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("odm_S0S1_SWAntDiv_Reset(): pDM_FatTable->bBecomeLinked = %d\n", pDM_FatTable->bBecomeLinked));
}

VOID
odm_S0S1_SwAntDiv(
	IN		PVOID			pDM_VOID,	
	IN		u1Byte			Step
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pSWAT_T			pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	pFAT_T			pDM_FatTable = &pDM_Odm->DM_FatTable;	
	u4Byte			i, MinMaxRSSI = 0xFF, LocalMaxRSSI, LocalMinRSSI;
	u4Byte			Main_RSSI, Aux_RSSI;
	u1Byte			HighTraffic_TrainTime_U = 0x32, HighTraffic_TrainTime_L = 0, Train_time_temp;
	u1Byte			LowTraffic_TrainTime_U = 200, LowTraffic_TrainTime_L = 0;
	u1Byte			RxIdleAnt = pDM_SWAT_Table->PreAntenna, TargetAnt, nextAnt = 0;
	PSTA_INFO_T		pEntry = NULL;
	u4Byte			value32;

	
	if(!pDM_Odm->bLinked) //bLinked==False
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[No Link!!!]\n"));
		if(pDM_FatTable->bBecomeLinked == TRUE)
		{
			odm_Tx_By_TxDesc_or_Reg(pDM_Odm, TX_BY_REG);
			if (pDM_Odm->SupportICType == ODM_RTL8723B) {
				
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Set REG 948[9:6]=0x0\n"));
				ODM_SetBBReg(pDM_Odm, 0x948 , (BIT9|BIT8|BIT7|BIT6), 0x0); 
			}
			pDM_FatTable->bBecomeLinked = pDM_Odm->bLinked;
		}
		return;
	}
	else
	{
		if(pDM_FatTable->bBecomeLinked ==FALSE)
		{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Linked !!!]\n"));
			
			if(pDM_Odm->SupportICType == ODM_RTL8723B)
			{
				value32 = ODM_GetBBReg(pDM_Odm, 0x864, BIT5|BIT4|BIT3);

				#if (RTL8723B_SUPPORT == 1)
					if (value32 == 0x0)
						ODM_UpdateRxIdleAnt_8723B(pDM_Odm, MAIN_ANT, ANT1_2G, ANT2_2G);
					else if (value32 == 0x1)
						ODM_UpdateRxIdleAnt_8723B(pDM_Odm, AUX_ANT, ANT2_2G, ANT1_2G);
				#endif
				
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("8723B: First link! Force antenna to  %s\n",(value32 == 0x0?"MAIN":"AUX") ));
			}			
			pDM_FatTable->bBecomeLinked = pDM_Odm->bLinked;
		}
	}

	if (pDM_Odm->bOneEntryOnly == TRUE)
		odm_Tx_By_TxDesc_or_Reg(pDM_Odm, TX_BY_REG);
	else
		odm_Tx_By_TxDesc_or_Reg(pDM_Odm, TX_BY_DESC);
	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[%d] { try_flag=(( %d )), Step=(( %d )), Double_chk_flag = (( %d )) }\n",
		__LINE__,pDM_SWAT_Table->try_flag,Step,pDM_SWAT_Table->Double_chk_flag));

	// Handling step mismatch condition.
	// Peak step is not finished at last time. Recover the variable and check again.
	if(	Step != pDM_SWAT_Table->try_flag	)
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Step != try_flag]    Need to Reset After Link\n"));
		ODM_SwAntDivRestAfterLink(pDM_Odm);
	}

	if (pDM_SWAT_Table->try_flag == SWAW_STEP_INIT) {
		
		pDM_SWAT_Table->try_flag = SWAW_STEP_PEEK;
		pDM_SWAT_Table->Train_time_flag=0;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[set try_flag = 0]  Prepare for peek!\n\n"));
		return;
		
	} else {
	
		//1 Normal State (Begin Trying)
		if (pDM_SWAT_Table->try_flag == SWAW_STEP_PEEK) {
		
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("TxOkCnt=(( %llu )), RxOkCnt=(( %llu )), TrafficLoad = (%d))\n", pDM_Odm->curTxOkCnt, pDM_Odm->curRxOkCnt, pDM_Odm->TrafficLoad));
			
			if (pDM_Odm->TrafficLoad == TRAFFIC_HIGH)
			{
				Train_time_temp = pDM_SWAT_Table->Train_time ;
				
				if(pDM_SWAT_Table->Train_time_flag==3)
				{
					HighTraffic_TrainTime_L=0xa;
					
					if(Train_time_temp<=16)
						Train_time_temp=HighTraffic_TrainTime_L;
					else
						Train_time_temp-=16;
					
				}				
				else if(pDM_SWAT_Table->Train_time_flag==2)
				{
					Train_time_temp-=8;
					HighTraffic_TrainTime_L=0xf;
				}	
				else if(pDM_SWAT_Table->Train_time_flag==1)
				{
					Train_time_temp-=4;
					HighTraffic_TrainTime_L=0x1e;
				}
				else if(pDM_SWAT_Table->Train_time_flag==0)
				{
					Train_time_temp+=8;
					HighTraffic_TrainTime_L=0x28;
				}

				
				//ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** Train_time_temp = ((%d))\n",Train_time_temp));

				//--
				if(Train_time_temp > HighTraffic_TrainTime_U)
					Train_time_temp=HighTraffic_TrainTime_U;
				
				else if(Train_time_temp < HighTraffic_TrainTime_L)
					Train_time_temp=HighTraffic_TrainTime_L;

				pDM_SWAT_Table->Train_time = Train_time_temp; /*10ms~200ms*/
				
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Train_time_flag=((%d)), Train_time=((%d))\n", pDM_SWAT_Table->Train_time_flag, pDM_SWAT_Table->Train_time));

			} else if ((pDM_Odm->TrafficLoad == TRAFFIC_MID) || (pDM_Odm->TrafficLoad == TRAFFIC_LOW)) { 
			
				Train_time_temp=pDM_SWAT_Table->Train_time ;
				
				if(pDM_SWAT_Table->Train_time_flag==3)
				{
					LowTraffic_TrainTime_L=10;
					if(Train_time_temp<50)
						Train_time_temp=LowTraffic_TrainTime_L;
					else
						Train_time_temp-=50;
				}				
				else if(pDM_SWAT_Table->Train_time_flag==2)
				{
					Train_time_temp-=30;
					LowTraffic_TrainTime_L=36;
				}	
				else if(pDM_SWAT_Table->Train_time_flag==1)
				{
					Train_time_temp-=10;
					LowTraffic_TrainTime_L=40;
				}
				else
					Train_time_temp+=10;	
				
				//--
				if(Train_time_temp >= LowTraffic_TrainTime_U)
					Train_time_temp=LowTraffic_TrainTime_U;
				
				else if(Train_time_temp <= LowTraffic_TrainTime_L)
					Train_time_temp=LowTraffic_TrainTime_L;

				pDM_SWAT_Table->Train_time = Train_time_temp; /*10ms~200ms*/
				
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Train_time_flag=((%d)) , Train_time=((%d))\n", pDM_SWAT_Table->Train_time_flag, pDM_SWAT_Table->Train_time));

			} else {
				pDM_SWAT_Table->Train_time = 0xc8; /*200ms*/

			}
				
			//-----------------
		
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Current MinMaxRSSI is ((%d))\n", pDM_FatTable->MinMaxRSSI));

                        //---reset index---
			if (pDM_SWAT_Table->reset_idx >= RSSI_CHECK_RESET_PERIOD) {
				
				pDM_FatTable->MinMaxRSSI = 0;
				pDM_SWAT_Table->reset_idx = 0;
			}
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("reset_idx = (( %d ))\n", pDM_SWAT_Table->reset_idx));

			pDM_SWAT_Table->reset_idx++;

			//---double check flag---
			if ((pDM_FatTable->MinMaxRSSI > RSSI_CHECK_THRESHOLD) && (pDM_SWAT_Table->Double_chk_flag == 0))
			{			
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, (" MinMaxRSSI is ((%d)), and > %d\n",
					pDM_FatTable->MinMaxRSSI, RSSI_CHECK_THRESHOLD));

				pDM_SWAT_Table->Double_chk_flag =1;
				pDM_SWAT_Table->try_flag = SWAW_STEP_DETERMINE; 
				pDM_SWAT_Table->RSSI_Trying = 0;

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Test the current Ant for (( %d )) ms again\n", pDM_SWAT_Table->Train_time));
				ODM_UpdateRxIdleAnt(pDM_Odm, pDM_FatTable->RxIdleAnt);
				ODM_SetTimer(pDM_Odm, &(pDM_SWAT_Table->phydm_SwAntennaSwitchTimer), pDM_SWAT_Table->Train_time); /*ms*/	
				return;
			}
			
			nextAnt = (pDM_FatTable->RxIdleAnt == MAIN_ANT)? AUX_ANT : MAIN_ANT;

			pDM_SWAT_Table->try_flag = SWAW_STEP_DETERMINE;
			
			if(pDM_SWAT_Table->reset_idx<=1)
				pDM_SWAT_Table->RSSI_Trying = 2;
			else
				pDM_SWAT_Table->RSSI_Trying = 1;

			odm_S0S1_SwAntDivByCtrlFrame(pDM_Odm, SWAW_STEP_PEEK);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[set try_flag=1]  Normal State:  Begin Trying!!\n"));	
			
		} else if ((pDM_SWAT_Table->try_flag == SWAW_STEP_DETERMINE) && (pDM_SWAT_Table->Double_chk_flag == 0)) {
			
			nextAnt = (pDM_FatTable->RxIdleAnt  == MAIN_ANT)? AUX_ANT : MAIN_ANT;		
			pDM_SWAT_Table->RSSI_Trying--;
		}
		
		//1 Decision State
		if ((pDM_SWAT_Table->try_flag == SWAW_STEP_DETERMINE) && (pDM_SWAT_Table->RSSI_Trying == 0)) {
			
			BOOLEAN bByCtrlFrame = FALSE;
			u8Byte	pkt_cnt_total = 0;
		
			for (i=0; i<ODM_ASSOCIATE_ENTRY_NUM; i++)
			{
				pEntry = pDM_Odm->pODM_StaInfo[i];
				if(IS_STA_VALID(pEntry))
				{
					//2 Caculate RSSI per Antenna
					Main_RSSI = (pDM_FatTable->MainAnt_Cnt[i]!=0)?(pDM_FatTable->MainAnt_Sum[i]/pDM_FatTable->MainAnt_Cnt[i]):0;
					Aux_RSSI = (pDM_FatTable->AuxAnt_Cnt[i]!=0)?(pDM_FatTable->AuxAnt_Sum[i]/pDM_FatTable->AuxAnt_Cnt[i]):0;
					
					if(pDM_FatTable->MainAnt_Cnt[i]<=1 && pDM_FatTable->CCK_counter_main>=1)
						Main_RSSI=0;	
					
					if(pDM_FatTable->AuxAnt_Cnt[i]<=1 && pDM_FatTable->CCK_counter_aux>=1)
						Aux_RSSI=0;
					
					TargetAnt = (Main_RSSI==Aux_RSSI)?pDM_SWAT_Table->PreAntenna:((Main_RSSI>=Aux_RSSI)?MAIN_ANT:AUX_ANT);
					LocalMaxRSSI = (Main_RSSI>=Aux_RSSI) ? Main_RSSI : Aux_RSSI;
					LocalMinRSSI = (Main_RSSI>=Aux_RSSI) ? Aux_RSSI : Main_RSSI;
					
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***  CCK_counter_main = (( %d ))  , CCK_counter_aux= ((  %d )) \n", pDM_FatTable->CCK_counter_main, pDM_FatTable->CCK_counter_aux));
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***  OFDM_counter_main = (( %d ))  , OFDM_counter_aux= ((  %d )) \n", pDM_FatTable->OFDM_counter_main, pDM_FatTable->OFDM_counter_aux));
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***  Main_Cnt = (( %d ))  , Main_RSSI= ((  %d )) \n", pDM_FatTable->MainAnt_Cnt[i], Main_RSSI));
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***  Aux_Cnt   = (( %d ))  , Aux_RSSI = ((  %d )) \n", pDM_FatTable->AuxAnt_Cnt[i]  , Aux_RSSI ));
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** MAC ID:[ %d ] , TargetAnt = (( %s )) \n", i ,( TargetAnt ==MAIN_ANT)?"MAIN_ANT":"AUX_ANT"));
					
					//2 Select RX Idle Antenna
					
					if (LocalMaxRSSI != 0 && LocalMaxRSSI < MinMaxRSSI)
					{
						RxIdleAnt = TargetAnt;
						MinMaxRSSI = LocalMaxRSSI;
						ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** LocalMaxRSSI-LocalMinRSSI = ((%d))\n",(LocalMaxRSSI-LocalMinRSSI)));
				
						if((LocalMaxRSSI-LocalMinRSSI)>8)
						{
							if(LocalMinRSSI != 0)
								pDM_SWAT_Table->Train_time_flag=3;
							else
							{
								if (MinMaxRSSI > RSSI_CHECK_THRESHOLD)
									pDM_SWAT_Table->Train_time_flag=0;
								else
									pDM_SWAT_Table->Train_time_flag=3;
							}
						}
						else if((LocalMaxRSSI-LocalMinRSSI)>5)
							pDM_SWAT_Table->Train_time_flag=2;
						else if((LocalMaxRSSI-LocalMinRSSI)>2)
							pDM_SWAT_Table->Train_time_flag=1;
						else
							pDM_SWAT_Table->Train_time_flag=0;
							
					}
					
					//2 Select TX Antenna
					if(TargetAnt == MAIN_ANT)
						pDM_FatTable->antsel_a[i] = ANT1_2G;
					else
						pDM_FatTable->antsel_a[i] = ANT2_2G;
			
				}
				pDM_FatTable->MainAnt_Sum[i] = 0;
				pDM_FatTable->AuxAnt_Sum[i] = 0;
				pDM_FatTable->MainAnt_Cnt[i] = 0;
				pDM_FatTable->AuxAnt_Cnt[i] = 0;
			}

			if(pDM_SWAT_Table->bSWAntDivByCtrlFrame)
			{
				odm_S0S1_SwAntDivByCtrlFrame(pDM_Odm, SWAW_STEP_DETERMINE);
				bByCtrlFrame = TRUE;
			}

			pkt_cnt_total = pDM_FatTable->CCK_counter_main + pDM_FatTable->CCK_counter_aux + 
							pDM_FatTable->OFDM_counter_main + pDM_FatTable->OFDM_counter_aux;
			pDM_FatTable->CCK_counter_main=0;
			pDM_FatTable->CCK_counter_aux=0;
			pDM_FatTable->OFDM_counter_main=0;
			pDM_FatTable->OFDM_counter_aux=0;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("Control frame packet counter = %d, Data frame packet counter = %llu\n", 
				pDM_SWAT_Table->PktCnt_SWAntDivByCtrlFrame, pkt_cnt_total));
			
			if(MinMaxRSSI == 0xff || ((pkt_cnt_total < (pDM_SWAT_Table->PktCnt_SWAntDivByCtrlFrame >> 1)) && pDM_Odm->PhyDbgInfo.NumQryBeaconPkt < 2))
			{	
				MinMaxRSSI = 0;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("Check RSSI of control frame because MinMaxRSSI == 0xff\n"));
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("bByCtrlFrame = %d\n", bByCtrlFrame));
				
				if(bByCtrlFrame)
				{
					Main_RSSI = (pDM_FatTable->MainAnt_CtrlFrame_Cnt!=0)?(pDM_FatTable->MainAnt_CtrlFrame_Sum/pDM_FatTable->MainAnt_CtrlFrame_Cnt):0;
					Aux_RSSI = (pDM_FatTable->AuxAnt_CtrlFrame_Cnt!=0)?(pDM_FatTable->AuxAnt_CtrlFrame_Sum/pDM_FatTable->AuxAnt_CtrlFrame_Cnt):0;
					
					if(pDM_FatTable->MainAnt_CtrlFrame_Cnt<=1 && pDM_FatTable->CCK_CtrlFrame_Cnt_main>=1)
						Main_RSSI=0;	
					
					if(pDM_FatTable->AuxAnt_CtrlFrame_Cnt<=1 && pDM_FatTable->CCK_CtrlFrame_Cnt_aux>=1)
						Aux_RSSI=0;

					if (Main_RSSI != 0 || Aux_RSSI != 0)
					{
						RxIdleAnt = (Main_RSSI==Aux_RSSI)?pDM_SWAT_Table->PreAntenna:((Main_RSSI>=Aux_RSSI)?MAIN_ANT:AUX_ANT);
						LocalMaxRSSI = (Main_RSSI>=Aux_RSSI) ? Main_RSSI : Aux_RSSI;
						LocalMinRSSI = (Main_RSSI>=Aux_RSSI) ? Aux_RSSI : Main_RSSI;

						if((LocalMaxRSSI-LocalMinRSSI)>8)
							pDM_SWAT_Table->Train_time_flag=3;
						else if((LocalMaxRSSI-LocalMinRSSI)>5)
							pDM_SWAT_Table->Train_time_flag=2;
						else if((LocalMaxRSSI-LocalMinRSSI)>2)
							pDM_SWAT_Table->Train_time_flag=1;
						else
							pDM_SWAT_Table->Train_time_flag=0;

						ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("Control frame: Main_RSSI = %d, Aux_RSSI = %d\n", Main_RSSI, Aux_RSSI));
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("RxIdleAnt decided by control frame = %s\n", (RxIdleAnt == MAIN_ANT?"MAIN":"AUX")));
					}
				}
			}

			pDM_FatTable->MinMaxRSSI = MinMaxRSSI;
			pDM_SWAT_Table->try_flag = SWAW_STEP_PEEK;
						
			if( pDM_SWAT_Table->Double_chk_flag==1)
			{
				pDM_SWAT_Table->Double_chk_flag=0;
				
				if (pDM_FatTable->MinMaxRSSI > RSSI_CHECK_THRESHOLD) {
					
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, (" [Double check] MinMaxRSSI ((%d)) > %d again!!\n",
						pDM_FatTable->MinMaxRSSI, RSSI_CHECK_THRESHOLD));
					
					ODM_UpdateRxIdleAnt(pDM_Odm, RxIdleAnt);	
					
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[reset try_flag = 0] Training accomplished !!!]\n\n\n"));
					return;
				}
				else
				{
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, (" [Double check] MinMaxRSSI ((%d)) <= %d !!\n",
						pDM_FatTable->MinMaxRSSI, RSSI_CHECK_THRESHOLD));

					nextAnt = (pDM_FatTable->RxIdleAnt  == MAIN_ANT)? AUX_ANT : MAIN_ANT;
					pDM_SWAT_Table->try_flag = SWAW_STEP_PEEK; 
					pDM_SWAT_Table->reset_idx = RSSI_CHECK_RESET_PERIOD;
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[set try_flag=0]  Normal State:  Need to tryg again!!\n\n\n"));
					return;
				}
			}
			else
			{
				if (pDM_FatTable->MinMaxRSSI < RSSI_CHECK_THRESHOLD)
					pDM_SWAT_Table->reset_idx = RSSI_CHECK_RESET_PERIOD;
				
				pDM_SWAT_Table->PreAntenna =RxIdleAnt;
				ODM_UpdateRxIdleAnt(pDM_Odm, RxIdleAnt );
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("[reset try_flag = 0] Training accomplished !!!] \n\n\n"));
				return;
			}
			
		}

	}

	//1 4.Change TRX antenna

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("RSSI_Trying = (( %d )),    Ant: (( %s )) >>> (( %s )) \n",
		pDM_SWAT_Table->RSSI_Trying, (pDM_FatTable->RxIdleAnt  == MAIN_ANT?"MAIN":"AUX"),(nextAnt == MAIN_ANT?"MAIN":"AUX")));
		
	ODM_UpdateRxIdleAnt(pDM_Odm, nextAnt);

	//1 5.Reset Statistics

	pDM_FatTable->RxIdleAnt  = nextAnt;

	//1 6.Set next timer   (Trying State)
	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, (" Test ((%s)) Ant for (( %d )) ms\n", (nextAnt == MAIN_ANT?"MAIN":"AUX"), pDM_SWAT_Table->Train_time));
	ODM_SetTimer(pDM_Odm, &(pDM_SWAT_Table->phydm_SwAntennaSwitchTimer), pDM_SWAT_Table->Train_time); /*ms*/
}


#if (DM_ODM_SUPPORT_TYPE == ODM_CE)

VOID
ODM_SW_AntDiv_WorkitemCallback(
	IN PVOID	pContext
)
{
	PADAPTER
	pAdapter = (PADAPTER)pContext;
	HAL_DATA_TYPE
	*pHalData = GET_HAL_DATA(pAdapter);

	/*DbgPrint("SW_antdiv_Workitem_Callback");*/
	odm_S0S1_SwAntDiv(&pHalData->odmpriv, SWAW_STEP_DETERMINE);
}

VOID
ODM_SW_AntDiv_Callback(void *FunctionContext)
{
	PDM_ODM_T	pDM_Odm= (PDM_ODM_T)FunctionContext;
	PADAPTER	padapter = pDM_Odm->Adapter;
	if(padapter->net_closed == _TRUE)
		return;
	
	#if 0 /* Can't do I/O in timer callback*/
	odm_S0S1_SwAntDiv(pDM_Odm, SWAW_STEP_DETERMINE);
	#else
	rtw_run_in_thread_cmd(padapter, ODM_SW_AntDiv_WorkitemCallback, padapter);
	#endif
}


#endif

VOID
odm_S0S1_SwAntDivByCtrlFrame(
	IN		PVOID			pDM_VOID,	
	IN		u1Byte			Step
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pSWAT_T	pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	pFAT_T		pDM_FatTable = &pDM_Odm->DM_FatTable;
	
	switch(Step)
	{
		case SWAW_STEP_PEEK:
			pDM_SWAT_Table->PktCnt_SWAntDivByCtrlFrame = 0;
			pDM_SWAT_Table->bSWAntDivByCtrlFrame = TRUE;
			pDM_FatTable->MainAnt_CtrlFrame_Cnt = 0;
			pDM_FatTable->AuxAnt_CtrlFrame_Cnt = 0;
			pDM_FatTable->MainAnt_CtrlFrame_Sum = 0;
			pDM_FatTable->AuxAnt_CtrlFrame_Sum = 0;
			pDM_FatTable->CCK_CtrlFrame_Cnt_main = 0;
			pDM_FatTable->CCK_CtrlFrame_Cnt_aux = 0;
			pDM_FatTable->OFDM_CtrlFrame_Cnt_main = 0;
			pDM_FatTable->OFDM_CtrlFrame_Cnt_aux = 0;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("odm_S0S1_SwAntDivForAPMode(): Start peek and reset counter\n"));
			break;
		case SWAW_STEP_DETERMINE:
			pDM_SWAT_Table->bSWAntDivByCtrlFrame = FALSE;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("odm_S0S1_SwAntDivForAPMode(): Stop peek\n"));
			break;
		default:
			pDM_SWAT_Table->bSWAntDivByCtrlFrame = FALSE;
			break;
	}			
}

VOID
odm_AntselStatisticsOfCtrlFrame(
	IN		PVOID			pDM_VOID,
	IN		u1Byte			antsel_tr_mux,
	IN		u4Byte			RxPWDBAll
	
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;

	if(antsel_tr_mux == ANT1_2G)
	{
		pDM_FatTable->MainAnt_CtrlFrame_Sum+=RxPWDBAll;
		pDM_FatTable->MainAnt_CtrlFrame_Cnt++;
	}
	else
	{
		pDM_FatTable->AuxAnt_CtrlFrame_Sum+=RxPWDBAll;
		pDM_FatTable->AuxAnt_CtrlFrame_Cnt++;
	}
}

VOID
odm_S0S1_SwAntDivByCtrlFrame_ProcessRSSI(
	IN		PVOID			pDM_VOID,	
	IN		PVOID			p_phy_info_void,
	IN		PVOID			p_pkt_info_void
	//IN		PODM_PHY_INFO_T		pPhyInfo,
	//IN		PODM_PACKET_INFO_T		pPktinfo
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PODM_PHY_INFO_T	 pPhyInfo=(PODM_PHY_INFO_T)p_phy_info_void;
	PODM_PACKET_INFO_T 	pPktinfo=(PODM_PACKET_INFO_T)p_pkt_info_void;
	pSWAT_T	pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	pFAT_T		pDM_FatTable = &pDM_Odm->DM_FatTable;
	BOOLEAN		isCCKrate;

	if(!(pDM_Odm->SupportAbility & ODM_BB_ANT_DIV))
		return;

	if(pDM_Odm->AntDivType != S0S1_SW_ANTDIV)
		return;

	// In try state
	if(!pDM_SWAT_Table->bSWAntDivByCtrlFrame)
		return;

	// No HW error and match receiver address
	if(!pPktinfo->bToSelf)
		return;
	
	pDM_SWAT_Table->PktCnt_SWAntDivByCtrlFrame++;
	isCCKrate = ((pPktinfo->DataRate >= DESC_RATE1M ) && (pPktinfo->DataRate <= DESC_RATE11M ))?TRUE :FALSE;

	if(isCCKrate)
	{
	 	pDM_FatTable->antsel_rx_keep_0 = (pDM_FatTable->RxIdleAnt == MAIN_ANT) ? ANT1_2G : ANT2_2G;

		if(pDM_FatTable->antsel_rx_keep_0==ANT1_2G)
			pDM_FatTable->CCK_CtrlFrame_Cnt_main++;
		else
			pDM_FatTable->CCK_CtrlFrame_Cnt_aux++;

		odm_AntselStatisticsOfCtrlFrame(pDM_Odm, pDM_FatTable->antsel_rx_keep_0, pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_A]);
	}
	else
	{
		if(pDM_FatTable->antsel_rx_keep_0==ANT1_2G)
			pDM_FatTable->OFDM_CtrlFrame_Cnt_main++;
		else
			pDM_FatTable->OFDM_CtrlFrame_Cnt_aux++;

		odm_AntselStatisticsOfCtrlFrame(pDM_Odm, pDM_FatTable->antsel_rx_keep_0, pPhyInfo->RxPWDBAll);
	}
}

#endif //#if (RTL8723B_SUPPORT == 1)||(RTL8821A_SUPPORT == 1)




VOID
odm_SetNextMACAddrTarget(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pFAT_T			pDM_FatTable = &pDM_Odm->DM_FatTable;
	PSTA_INFO_T   	pEntry;
	u4Byte			value32, i;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("odm_SetNextMACAddrTarget() ==>\n"));
	
	if (pDM_Odm->bLinked)
	{
		for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {
			
			if ((pDM_FatTable->TrainIdx+1) == ODM_ASSOCIATE_ENTRY_NUM)
				pDM_FatTable->TrainIdx = 0;
			else
				pDM_FatTable->TrainIdx++;
			
			pEntry = pDM_Odm->pODM_StaInfo[pDM_FatTable->TrainIdx];
			
			if (IS_STA_VALID(pEntry)) {
				
				/*Match MAC ADDR*/
				#if (DM_ODM_SUPPORT_TYPE & (ODM_CE))
				value32 = (pEntry->hwaddr[5]<<8)|pEntry->hwaddr[4];
				#else
				value32 = (pEntry->MacAddr[5]<<8)|pEntry->MacAddr[4];
				#endif
				
				ODM_SetMACReg(pDM_Odm, 0x7b4, 0xFFFF, value32);/*0x7b4~0x7b5*/
				
				#if (DM_ODM_SUPPORT_TYPE & (ODM_CE))
				value32 = (pEntry->hwaddr[3]<<24)|(pEntry->hwaddr[2]<<16) |(pEntry->hwaddr[1]<<8) |pEntry->hwaddr[0];
				#else
				value32 = (pEntry->MacAddr[3]<<24)|(pEntry->MacAddr[2]<<16) |(pEntry->MacAddr[1]<<8) |pEntry->MacAddr[0];
				#endif
				ODM_SetMACReg(pDM_Odm, 0x7b0, bMaskDWord, value32);/*0x7b0~0x7b3*/

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("pDM_FatTable->TrainIdx=%d\n", pDM_FatTable->TrainIdx));
				
				#if (DM_ODM_SUPPORT_TYPE & (ODM_CE))
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Training MAC Addr = %x:%x:%x:%x:%x:%x\n",
					pEntry->hwaddr[5], pEntry->hwaddr[4], pEntry->hwaddr[3], pEntry->hwaddr[2], pEntry->hwaddr[1], pEntry->hwaddr[0]));
				#else
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Training MAC Addr = %x:%x:%x:%x:%x:%x\n",
					pEntry->MacAddr[5], pEntry->MacAddr[4], pEntry->MacAddr[3], pEntry->MacAddr[2], pEntry->MacAddr[1], pEntry->MacAddr[0]));
				#endif

				break;
			}
		}
	}

#if 0
	//
	//2012.03.26 LukeLee: This should be removed later, the MAC address is changed according to MACID in turn
	//

	//odm_SetNextMACAddrTarget(pDM_Odm);
	
	//1 Select MAC Address Filter
	for (i=0; i<6; i++)
	{
		if(Bssid[i] != pDM_FatTable->Bssid[i])
		{
			bMatchBSSID = FALSE;
			break;
		}
	}
	if(bMatchBSSID == FALSE)
	{
		//Match MAC ADDR
		value32 = (Bssid[5]<<8)|Bssid[4];
		ODM_SetMACReg(pDM_Odm, 0x7b4, 0xFFFF, value32);
		value32 = (Bssid[3]<<24)|(Bssid[2]<<16) |(Bssid[1]<<8) |Bssid[0];
		ODM_SetMACReg(pDM_Odm, 0x7b0, bMaskDWord, value32);
	}

	return bMatchBSSID;
#endif
				
}

#if (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY)) || (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))

VOID
odm_FastAntTraining(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;

	u4Byte	MaxRSSI_pathA=0, Pckcnt_pathA=0;
	u1Byte	i,TargetAnt_pathA=0;
	BOOLEAN	bPktFilterMacth_pathA = FALSE;
	#if(RTL8192E_SUPPORT == 1)
	u4Byte	MaxRSSI_pathB=0, Pckcnt_pathB=0;
	u1Byte	TargetAnt_pathB=0;
	BOOLEAN	bPktFilterMacth_pathB = FALSE;
	#endif


	if(!pDM_Odm->bLinked) //bLinked==False
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[No Link!!!]\n"));
		
		if(pDM_FatTable->bBecomeLinked == TRUE)
		{
			odm_AntDiv_on_off(pDM_Odm, ANTDIV_OFF);
			phydm_FastTraining_enable(pDM_Odm , FAT_OFF);
			odm_Tx_By_TxDesc_or_Reg(pDM_Odm, TX_BY_REG);
			pDM_FatTable->bBecomeLinked = pDM_Odm->bLinked;
		}
		return;
	}
	else
	{
		if(pDM_FatTable->bBecomeLinked ==FALSE)
		{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Linked!!!]\n"));
			pDM_FatTable->bBecomeLinked = pDM_Odm->bLinked;
		}
	}

	if (pDM_Odm->bOneEntryOnly == TRUE)
		odm_Tx_By_TxDesc_or_Reg(pDM_Odm, TX_BY_REG);
	else
		odm_Tx_By_TxDesc_or_Reg(pDM_Odm, TX_BY_DESC);

		
        if(pDM_Odm->SupportICType == ODM_RTL8188E)
	{
           ODM_SetBBReg(pDM_Odm, 0x864 , BIT2|BIT1|BIT0, ((pDM_Odm->fat_comb_a)-1));
        }
	#if(RTL8192E_SUPPORT == 1)
        else if(pDM_Odm->SupportICType == ODM_RTL8192E)
        {
           ODM_SetBBReg(pDM_Odm, 0xB38 , BIT2|BIT1|BIT0, ((pDM_Odm->fat_comb_a)-1) );	   //path-A  // ant combination=regB38[2:0]+1
	   ODM_SetBBReg(pDM_Odm, 0xB38 , BIT18|BIT17|BIT16, ((pDM_Odm->fat_comb_b)-1) );  //path-B  // ant combination=regB38[18:16]+1
        }
	#endif

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("==>odm_FastAntTraining()\n"));

	//1 TRAINING STATE
	if(pDM_FatTable->FAT_State == FAT_TRAINING_STATE)
	{
		//2 Caculate RSSI per Antenna

                //3 [path-A]---------------------------
		for (i=0; i<(pDM_Odm->fat_comb_a); i++) // i : antenna index
		{
			if(pDM_FatTable->antRSSIcnt[i] == 0)
				pDM_FatTable->antAveRSSI[i] = 0;
			else
			{
			pDM_FatTable->antAveRSSI[i] = pDM_FatTable->antSumRSSI[i] /pDM_FatTable->antRSSIcnt[i];
				bPktFilterMacth_pathA = TRUE;
			}
			
			if(pDM_FatTable->antAveRSSI[i] > MaxRSSI_pathA)
			{
				MaxRSSI_pathA = pDM_FatTable->antAveRSSI[i];
                                Pckcnt_pathA = pDM_FatTable ->antRSSIcnt[i];
				TargetAnt_pathA =  i ; 
			}
                        else if(pDM_FatTable->antAveRSSI[i] == MaxRSSI_pathA)
			{
				if(  (pDM_FatTable->antRSSIcnt[i] )   >   Pckcnt_pathA)
			{
					MaxRSSI_pathA = pDM_FatTable->antAveRSSI[i];
					Pckcnt_pathA = pDM_FatTable ->antRSSIcnt[i];
				        TargetAnt_pathA = i ;
			        }
			}

			ODM_RT_TRACE("*** Ant-Index : [ %d ],      Counter = (( %d )),     Avg RSSI = (( %d )) \n", i, pDM_FatTable->antRSSIcnt[i],  pDM_FatTable->antAveRSSI[i] );
		}


		/*
		#if(RTL8192E_SUPPORT == 1)
		//3 [path-B]---------------------------
		for (i=0; i<(pDM_Odm->fat_comb_b); i++)
		{
			if(pDM_FatTable->antRSSIcnt_pathB[i] == 0)
				pDM_FatTable->antAveRSSI_pathB[i] = 0;				
			else // (antRSSIcnt[i] != 0)
			{
				pDM_FatTable->antAveRSSI_pathB[i] = pDM_FatTable->antSumRSSI_pathB[i] /pDM_FatTable->antRSSIcnt_pathB[i];
				bPktFilterMacth_pathB = TRUE;
			}
			if(pDM_FatTable->antAveRSSI_pathB[i] > MaxRSSI_pathB)
			{
				MaxRSSI_pathB = pDM_FatTable->antAveRSSI_pathB[i];
                                Pckcnt_pathB = pDM_FatTable ->antRSSIcnt_pathB[i];
				TargetAnt_pathB = (u1Byte) i; 
			}
                        if(pDM_FatTable->antAveRSSI_pathB[i] == MaxRSSI_pathB)
			{
				if(pDM_FatTable ->antRSSIcnt_pathB > Pckcnt_pathB)
		{
					MaxRSSI_pathB = pDM_FatTable->antAveRSSI_pathB[i];
					TargetAnt_pathB = (u1Byte) i;
				} 
		}
			if (pDM_Odm->fat_print_rssi==1)
		{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***{Path-B}: Sum RSSI[%d] = (( %d )),      cnt RSSI [%d] = (( %d )),     Avg RSSI[%d] = (( %d )) \n",
				i, pDM_FatTable->antSumRSSI_pathB[i], i, pDM_FatTable->antRSSIcnt_pathB[i], i, pDM_FatTable->antAveRSSI_pathB[i]));
			}
		}
		#endif
		*/

	//1 DECISION STATE

		//2 Select TRX Antenna

		phydm_FastTraining_enable(pDM_Odm, FAT_OFF);

		//3 [path-A]---------------------------
		if(bPktFilterMacth_pathA  == FALSE)
		{
			//ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("{Path-A}: None Packet is matched\n"));
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("{Path-A}: None Packet is matched\n"));
			odm_AntDiv_on_off(pDM_Odm, ANTDIV_OFF);
		}
		else
		{
			ODM_RT_TRACE("TargetAnt_pathA = (( %d )) , MaxRSSI_pathA = (( %d )) \n",TargetAnt_pathA,MaxRSSI_pathA);

			//3 [ update RX-optional ant ]        Default RX is Omni, Optional RX is the best decision by FAT
			if(pDM_Odm->SupportICType == ODM_RTL8188E)
			{
				ODM_SetBBReg(pDM_Odm, 0x864 , BIT8|BIT7|BIT6, TargetAnt_pathA);	
			}
			else if(pDM_Odm->SupportICType == ODM_RTL8192E)
			{
				ODM_SetBBReg(pDM_Odm, 0xB38 , BIT8|BIT7|BIT6, TargetAnt_pathA);//Optional RX [pth-A]
			}
			//3 [ update TX ant ]
			odm_UpdateTxAnt(pDM_Odm, TargetAnt_pathA, (pDM_FatTable->TrainIdx)); 

			if(TargetAnt_pathA == 0)
				odm_AntDiv_on_off(pDM_Odm, ANTDIV_OFF);
		}
		/*
		#if(RTL8192E_SUPPORT == 1)
		//3 [path-B]---------------------------
		if(bPktFilterMacth_pathB == FALSE)
		{
			if (pDM_Odm->fat_print_rssi==1)
			{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***[%d]{Path-B}: None Packet is matched\n\n\n",__LINE__));
			}
		}
		else
		{
			if (pDM_Odm->fat_print_rssi==1)
			{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, 
				(" ***TargetAnt_pathB = (( %d )) *** MaxRSSI = (( %d ))***\n\n\n",TargetAnt_pathB,MaxRSSI_pathB));
			}
			ODM_SetBBReg(pDM_Odm, 0xB38 , BIT21|BIT20|BIT19, TargetAnt_pathB);	//Default RX is Omni, Optional RX is the best decision by FAT		
			ODM_SetBBReg(pDM_Odm, 0x80c , BIT21, 1); //Reg80c[21]=1'b1		//from TX Info

			pDM_FatTable->antsel_pathB[pDM_FatTable->TrainIdx] = TargetAnt_pathB;
		}
		#endif
		*/

		//2 Reset Counter
		for(i=0; i<(pDM_Odm->fat_comb_a); i++)
		{
			pDM_FatTable->antSumRSSI[i] = 0;
			pDM_FatTable->antRSSIcnt[i] = 0;
		}
		/*
		#if(RTL8192E_SUPPORT == 1)
		for(i=0; i<=(pDM_Odm->fat_comb_b); i++)
		{
			pDM_FatTable->antSumRSSI_pathB[i] = 0;
			pDM_FatTable->antRSSIcnt_pathB[i] = 0;
		}
		#endif
		*/
		
		pDM_FatTable->FAT_State = FAT_PREPARE_STATE;
		return;
	}

	//1 NORMAL STATE
	if (pDM_FatTable->FAT_State == FAT_PREPARE_STATE)
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Start Prepare State ]\n"));

		odm_SetNextMACAddrTarget(pDM_Odm);

		//2 Prepare Training
		pDM_FatTable->FAT_State = FAT_TRAINING_STATE;
		phydm_FastTraining_enable(pDM_Odm , FAT_ON);
		odm_AntDiv_on_off(pDM_Odm, ANTDIV_ON);		//enable HW AntDiv
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Start Training State]\n"));

		ODM_SetTimer(pDM_Odm,&pDM_Odm->FastAntTrainingTimer, pDM_Odm->antdiv_intvl ); //ms
	}
		
}

VOID
odm_FastAntTrainingCallback(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;

#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PADAPTER	padapter = pDM_Odm->Adapter;
	if(padapter->net_closed == _TRUE)
	    return;
	//if(*pDM_Odm->pbNet_closed == TRUE)
	   // return;
#endif

#if USE_WORKITEM
	ODM_ScheduleWorkItem(&pDM_Odm->FastAntTrainingWorkitem);
#else
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("******odm_FastAntTrainingCallback****** \n"));
	odm_FastAntTraining(pDM_Odm);
#endif
}

VOID
odm_FastAntTrainingWorkItemCallback(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("******odm_FastAntTrainingWorkItemCallback****** \n"));
	odm_FastAntTraining(pDM_Odm);
}

#endif

#ifdef CONFIG_HL_SMART_ANTENNA_TYPE1

u4Byte
phydm_construct_hl_beam_codeword(
	IN		PVOID		pDM_VOID,
	IN		u4Byte		*beam_pattern_idx,
	IN		u4Byte		ant_num
	)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u4Byte		codeword = 0;
	u4Byte		data_tmp;
	u1Byte		i;

	if (ant_num < 8) {
		for (i = 0; i < ant_num; i++) {
			/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("beam_pattern_num[%x] = %x\n",i,beam_pattern_num[i] ));*/
			if (beam_pattern_idx[i] == 0) {
				data_tmp = 0x1;
				/**/
			} else if (beam_pattern_idx[i] == 1) {
				data_tmp = 0x2;
				/**/
			} else if (beam_pattern_idx[i] == 2) {
				data_tmp = 0x4;
				/**/
			} else if (beam_pattern_idx[i] == 3) {
				data_tmp = 0x8;
				/**/
			}  
			codeword |= (data_tmp<<(i*4));
		}
	}

	return codeword;
}

VOID
phydm_update_beam_pattern(
	IN		PVOID		pDM_VOID,
	IN		u4Byte		codeword,
	IN		u4Byte		codeword_length
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pSAT_T			pdm_sat_table = &(pDM_Odm->dm_sat_table);
	u1Byte			i;
	BOOLEAN			beam_ctrl_signal;
	u4Byte			one = 0x1;
	u4Byte			reg44_tmp_p, reg44_tmp_n, reg44_ori;
	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ SmartAnt ] Set Beam Pattern =0x%x\n", codeword));
	
	reg44_ori = ODM_GetMACReg(pDM_Odm, 0x44, bMaskDWord);
	/*ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("reg44_ori =0x%x\n", reg44_ori));*/
	
	for (i = 0; i <= (codeword_length-1); i++) {
		beam_ctrl_signal = (BOOLEAN)((codeword&BIT(i)) >> i);

		if (pDM_Odm->DebugComponents & ODM_COMP_ANT_DIV) {
			
			if (i == (codeword_length-1)) {
				DbgPrint("%d ]\n", beam_ctrl_signal);
				/**/
			} else if (i == 0) {
				DbgPrint("Send codeword[1:24] ---> [ %d ", beam_ctrl_signal);	
				/**/
			} else if ((i % 4) == 3) {
				DbgPrint("%d  |  ", beam_ctrl_signal);	
				/**/
			} else {
				DbgPrint("%d ", beam_ctrl_signal);
				/**/
			}
		}
		
		#if 1
		reg44_tmp_p = reg44_ori & (~(BIT11|BIT10)); /*clean bit 10 & 11*/
		reg44_tmp_p |= ((1<<11) | (beam_ctrl_signal<<10));
		reg44_tmp_n = reg44_ori & (~(BIT11|BIT10));

		/*ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("reg44_tmp_p =(( 0x%x )), reg44_tmp_n = (( 0x%x ))\n", reg44_tmp_p, reg44_tmp_n));*/
		ODM_SetMACReg(pDM_Odm, 0x44 , bMaskDWord, reg44_tmp_p);
		ODM_SetMACReg(pDM_Odm, 0x44 , bMaskDWord, reg44_tmp_n); 
		#else
		ODM_SetMACReg(pDM_Odm, 0x44 , BIT11|BIT10, ((1<<1) | beam_ctrl_signal));
		ODM_SetMACReg(pDM_Odm, 0x44 , BIT11, 0); 
		#endif

	}
}

VOID
phydm_update_rx_idle_beam(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pFAT_T			pDM_FatTable = &pDM_Odm->DM_FatTable;
	pSAT_T			pdm_sat_table = &(pDM_Odm->dm_sat_table);
	u4Byte			i;

	pdm_sat_table->update_beam_codeword = phydm_construct_hl_beam_codeword(pDM_Odm, &(pdm_sat_table->rx_idle_beam[0]), pdm_sat_table->ant_num);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Set target beam_pattern codeword = (( 0x%x ))\n", pdm_sat_table->update_beam_codeword));

	for (i = 0; i < (pdm_sat_table->ant_num); i++) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Update Rx-Idle-Beam ] RxIdleBeam[%d] =%d\n", i, pdm_sat_table->rx_idle_beam[i]));
		/**/
	}
	
	ODM_ScheduleWorkItem(&pdm_sat_table->hl_smart_antenna_workitem);
	/*ODM_StallExecution(1);*/
	
	pdm_sat_table->pre_codeword = pdm_sat_table->update_beam_codeword;
}

VOID
phydm_hl_smart_ant_cmd(
	IN		PVOID		pDM_VOID,
	IN		u4Byte		*const dm_value,
	IN		u4Byte		*_used,
	OUT		char			*output,
	IN		u4Byte		*_out_len
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pSAT_T			pdm_sat_table = &(pDM_Odm->dm_sat_table);	
	u4Byte			used = *_used;
	u4Byte			out_len = *_out_len;
	u4Byte			one = 0x1;
	u4Byte			codeword_length = pdm_sat_table->data_codeword_bit_num;
	u4Byte			beam_ctrl_signal, i;
	
	if (dm_value[0] == 1) { /*fix beam pattern*/
		
		pdm_sat_table->fix_beam_pattern_en = dm_value[1];
		
		if (pdm_sat_table->fix_beam_pattern_en == 1) {
			
			pdm_sat_table->fix_beam_pattern_codeword = dm_value[2];

			if (pdm_sat_table->fix_beam_pattern_codeword  > (one<<codeword_length)) {
				
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ SmartAnt ] Codeword overflow, Current codeword is ((0x%x)), and should be less than ((%d))bit\n", 
					pdm_sat_table->fix_beam_pattern_codeword, codeword_length));
				(pdm_sat_table->fix_beam_pattern_codeword) &= 0xffffff;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ SmartAnt ] Auto modify to (0x%x)\n", pdm_sat_table->fix_beam_pattern_codeword));
			}
			
			pdm_sat_table->update_beam_codeword = pdm_sat_table->fix_beam_pattern_codeword;

			/*---------------------------------------------------------*/
			PHYDM_SNPRINTF((output+used, out_len-used, "Fix Beam Pattern\n"));
			for (i = 0; i <= (codeword_length-1); i++) {
				beam_ctrl_signal = (BOOLEAN)((pdm_sat_table->update_beam_codeword&BIT(i)) >> i);
				
				if (i == (codeword_length-1)) {
					PHYDM_SNPRINTF((output+used, out_len-used, "%d]\n", beam_ctrl_signal));
					/**/
				} else if (i == 0) {
					PHYDM_SNPRINTF((output+used, out_len-used, "Send Codeword[1:24] to RFU -> [%d", beam_ctrl_signal));
					/**/
				} else if ((i % 4) == 3) {
					PHYDM_SNPRINTF((output+used, out_len-used, "%d|", beam_ctrl_signal));
					/**/
				} else {
					PHYDM_SNPRINTF((output+used, out_len-used, "%d", beam_ctrl_signal));
					/**/
				}
			}
			/*---------------------------------------------------------*/

			
			ODM_ScheduleWorkItem(&pdm_sat_table->hl_smart_antenna_workitem);
			/*ODM_StallExecution(1);*/
		} else if (pdm_sat_table->fix_beam_pattern_en == 0) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ SmartAnt ] Smart Antenna: Enable\n"));
		}
		
	} else if (dm_value[0] == 2) { /*set latch time*/
		
		pdm_sat_table->latch_time = dm_value[1];
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ SmartAnt ]  latch_time =0x%x\n", pdm_sat_table->latch_time));
	} else if (dm_value[0] == 3) {
	
		pdm_sat_table->fix_training_num_en = dm_value[1];
		
		if (pdm_sat_table->fix_training_num_en == 1) {
			pdm_sat_table->per_beam_training_pkt_num = dm_value[2];
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ SmartAnt ]  Fix per_beam_training_pkt_num = (( 0x%x ))\n", pdm_sat_table->per_beam_training_pkt_num));
		} else if (pdm_sat_table->fix_training_num_en == 0) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ SmartAnt ]  AUTO per_beam_training_pkt_num\n"));
			/**/
		}
	}
		
}


void
phydm_set_all_ant_same_beam_num(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pSAT_T			pdm_sat_table = &(pDM_Odm->dm_sat_table);

	if (pDM_Odm->AntDivType == HL_SW_SMART_ANT_TYPE1) { /*2Ant for 8821A*/
		
		pdm_sat_table->rx_idle_beam[0] = pdm_sat_table->fast_training_beam_num;
		pdm_sat_table->rx_idle_beam[1] = pdm_sat_table->fast_training_beam_num;
	}
	
	pdm_sat_table->update_beam_codeword = phydm_construct_hl_beam_codeword(pDM_Odm, &(pdm_sat_table->rx_idle_beam[0]), pdm_sat_table->ant_num);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ SmartAnt ] Set all ant beam_pattern: codeword = (( 0x%x ))\n", pdm_sat_table->update_beam_codeword));

	ODM_ScheduleWorkItem(&pdm_sat_table->hl_smart_antenna_workitem);
}

VOID
odm_FastAntTraining_hl_smart_antenna_type1(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pSAT_T		pdm_sat_table = &(pDM_Odm->dm_sat_table);
	pFAT_T		pDM_FatTable	 = &(pDM_Odm->DM_FatTable);
	pSWAT_T		pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	u4Byte		codeword = 0, i, j;
	u4Byte		TargetAnt;
	u4Byte		avg_rssi_tmp;
	u4Byte		target_ant_beam_max_rssi[SUPPORT_RF_PATH_NUM] = {0};
	u4Byte		max_beam_ant_rssi = 0;
	u4Byte		target_ant_beam[SUPPORT_RF_PATH_NUM] = {0};
	u4Byte		beam_tmp;


	if (!pDM_Odm->bLinked) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[No Link!!!]\n"));
				
		if (pDM_FatTable->bBecomeLinked == TRUE) {
			
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Link -> no Link\n"));
			pDM_FatTable->FAT_State = FAT_BEFORE_LINK_STATE;
			odm_AntDiv_on_off(pDM_Odm, ANTDIV_OFF);
			odm_Tx_By_TxDesc_or_Reg(pDM_Odm, TX_BY_REG);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("change to (( %d )) FAT_state\n", pDM_FatTable->FAT_State));
			
			pDM_FatTable->bBecomeLinked = pDM_Odm->bLinked;
		}
		return;
		
	} else {
		if (pDM_FatTable->bBecomeLinked == FALSE) {
			
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Linked !!!]\n"));
			
			pDM_FatTable->FAT_State = FAT_PREPARE_STATE;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("change to (( %d )) FAT_state\n", pDM_FatTable->FAT_State));
			
			/*pdm_sat_table->fast_training_beam_num = 0;*/
			/*phydm_set_all_ant_same_beam_num(pDM_Odm);*/
			
			pDM_FatTable->bBecomeLinked = pDM_Odm->bLinked;
		}
	}
	
	if (pDM_Odm->bOneEntryOnly == TRUE)
		odm_Tx_By_TxDesc_or_Reg(pDM_Odm, TX_BY_REG);
	else
		odm_Tx_By_TxDesc_or_Reg(pDM_Odm, TX_BY_DESC);

	/*ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("HL Smart Ant Training: State (( %d ))\n", pDM_FatTable->FAT_State));*/

	/* [DECISION STATE] */
	/*=======================================================================================*/
	if (pDM_FatTable->FAT_State == FAT_DECISION_STATE) {

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ 3. In Decision State]\n"));
		phydm_FastTraining_enable(pDM_Odm , FAT_OFF);
		
		/*compute target beam in each antenna*/
		for (i = 0; i < (pdm_sat_table->ant_num); i++) {
			for (j = 0; j < (pdm_sat_table->beam_patten_num_each_ant); j++) {

				if (pdm_sat_table->pkt_rssi_cnt[i][j] == 0) {
					avg_rssi_tmp = pdm_sat_table->pkt_rssi_pre[i][j];
					/**/
				} else {
					avg_rssi_tmp = (pdm_sat_table->pkt_rssi_sum[i][j]) / (pdm_sat_table->pkt_rssi_cnt[i][j]);
					pdm_sat_table->pkt_rssi_pre[i][j] = avg_rssi_tmp;
					/**/
				}

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Ant[%d], Beam[%d]: pkt_num=(( %d )), avg_rssi=(( %d ))\n", i, j, pdm_sat_table->pkt_rssi_cnt[i][j], avg_rssi_tmp));
				
				if (avg_rssi_tmp > target_ant_beam_max_rssi[i]) {
					target_ant_beam[i] = j;
					target_ant_beam_max_rssi[i] = avg_rssi_tmp;
				}

				/*reset counter value*/
				pdm_sat_table->pkt_rssi_sum[i][j] = 0;
				pdm_sat_table->pkt_rssi_cnt[i][j] = 0;
								
			}
			pdm_sat_table->rx_idle_beam[i] = target_ant_beam[i];
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("---------> Target of Ant[%d]: Beam_num-(( %d )) RSSI= ((%d))\n", 
					i,  target_ant_beam[i], target_ant_beam_max_rssi[i]));

			if (target_ant_beam_max_rssi[i] > max_beam_ant_rssi) {
				TargetAnt = i;
				max_beam_ant_rssi = target_ant_beam_max_rssi[i];
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Target of Ant = (( %d )) max_beam_ant_rssi = (( %d ))\n", 
					TargetAnt,  max_beam_ant_rssi));
			}
		}

		if (TargetAnt == 0) 
			TargetAnt = MAIN_ANT;
		else if (TargetAnt == 1)
			TargetAnt = AUX_ANT;

		/* [ update RX ant ]*/
		ODM_UpdateRxIdleAnt(pDM_Odm, (u1Byte)TargetAnt);

		/* [ update TX ant ]*/
		odm_UpdateTxAnt(pDM_Odm, (u1Byte)TargetAnt, (pDM_FatTable->TrainIdx));
		
		/*set beam in each antenna*/
		phydm_update_rx_idle_beam(pDM_Odm);

		phydm_FastTraining_enable(pDM_Odm , FAT_OFF);
		odm_AntDiv_on_off(pDM_Odm, ANTDIV_ON);		
		pDM_FatTable->FAT_State = FAT_PREPARE_STATE;

	} 
	/* [TRAINING STATE] */
	else if (pDM_FatTable->FAT_State == FAT_TRAINING_STATE) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ 2. In Training State]\n"));

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("fat_beam_n = (( %d )), pre_fat_beam_n = (( %d ))\n", 
			pdm_sat_table->fast_training_beam_num, pdm_sat_table->pre_fast_training_beam_num));
		
		if (pdm_sat_table->fast_training_beam_num > pdm_sat_table->pre_fast_training_beam_num) {
			
			pdm_sat_table->force_update_beam_en = 0;

		} else {
		
			pdm_sat_table->force_update_beam_en = 1;
			
			pdm_sat_table->pkt_counter = 0;
			beam_tmp = pdm_sat_table->fast_training_beam_num;
			if (pdm_sat_table->fast_training_beam_num >= (pdm_sat_table->beam_patten_num_each_ant-1)) {
				
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Timeout Update]  Beam_num (( %d )) -> (( decision ))\n", pdm_sat_table->fast_training_beam_num));	
				phydm_FastTraining_enable(pDM_Odm , FAT_OFF);
				pDM_FatTable->FAT_State = FAT_DECISION_STATE;					
				odm_FastAntTraining_hl_smart_antenna_type1(pDM_Odm);

			} else {
				pdm_sat_table->fast_training_beam_num++;
				
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Timeout Update]  Beam_num (( %d )) -> (( %d ))\n", beam_tmp, pdm_sat_table->fast_training_beam_num));
				phydm_set_all_ant_same_beam_num(pDM_Odm);
				pDM_FatTable->FAT_State = FAT_TRAINING_STATE;	
				
			}
		}
		pdm_sat_table->pre_fast_training_beam_num = pdm_sat_table->fast_training_beam_num;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Update Pre_Beam =(( %d ))\n", pdm_sat_table->pre_fast_training_beam_num));
	}
	/*  [Prepare State] */
	/*=======================================================================================*/
	else if (pDM_FatTable->FAT_State == FAT_PREPARE_STATE) {

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("\n\n[ 1. In Prepare State]\n"));
		
		if (pDM_Odm->pre_TrafficLoad == (pDM_Odm->TrafficLoad)) {
			if (pdm_sat_table->decision_holding_period != 0) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Holding_period = (( %d )), return!!!\n", pdm_sat_table->decision_holding_period));
				pdm_sat_table->decision_holding_period--;
				return;
			}
		}
				
		
		/* Set training packet number*/
		if (pdm_sat_table->fix_training_num_en == 0) {

			switch (pDM_Odm->TrafficLoad) {

			case TRAFFIC_HIGH: 
				pdm_sat_table->per_beam_training_pkt_num = 20;
				pdm_sat_table->decision_holding_period = 0;
				break;
			case TRAFFIC_MID: 
				pdm_sat_table->per_beam_training_pkt_num = 10;
				pdm_sat_table->decision_holding_period = 1;
				break;
			case TRAFFIC_LOW: 
				pdm_sat_table->per_beam_training_pkt_num = 5; /*ping 60000*/
				pdm_sat_table->decision_holding_period = 3;
				break;
			case TRAFFIC_ULTRA_LOW: 
				pdm_sat_table->per_beam_training_pkt_num = 2;
				pdm_sat_table->decision_holding_period = 5;
				break;
			default:
				break;			
			}
		}
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Fix_training_num = (( %d )), per_beam_training_pkt_num = (( %d ))\n",
			pdm_sat_table->fix_training_num_en , pdm_sat_table->per_beam_training_pkt_num));
		
		/* Set training MAC Addr. of target */
		odm_SetNextMACAddrTarget(pDM_Odm);

		phydm_FastTraining_enable(pDM_Odm , FAT_ON);
		odm_AntDiv_on_off(pDM_Odm, ANTDIV_OFF);
		pdm_sat_table->pkt_counter = 0;
		pdm_sat_table->fast_training_beam_num = 0;
		phydm_set_all_ant_same_beam_num(pDM_Odm);
		pdm_sat_table->pre_fast_training_beam_num = pdm_sat_table->fast_training_beam_num;
		pDM_FatTable->FAT_State = FAT_TRAINING_STATE;
	}
		
}


#endif /*#ifdef CONFIG_HL_SMART_ANTENNA_TYPE1*/

VOID
ODM_AntDivInit(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pFAT_T			pDM_FatTable = &pDM_Odm->DM_FatTable;
	pSWAT_T			pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;


	if(!(pDM_Odm->SupportAbility & ODM_BB_ANT_DIV))
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("[Return!!!]   Not Support Antenna Diversity Function\n"));
		return;
	}
        //---
	//---

	//2 [--General---]
	pDM_Odm->antdiv_period=0;

	pDM_FatTable->bBecomeLinked =FALSE;
	pDM_FatTable->AntDiv_OnOff =0xff;

	//3       -   AP   -

	//2 [---Set MAIN_ANT as default antenna if Auto-Ant enable---]
	odm_AntDiv_on_off(pDM_Odm, ANTDIV_OFF);

	pDM_Odm->AntType = ODM_AUTO_ANT;

	pDM_FatTable->RxIdleAnt = 0xff; /*to make RX-idle-antenna will be updated absolutly*/
	ODM_UpdateRxIdleAnt(pDM_Odm, MAIN_ANT);
		
	//2 [---Set TX Antenna---]
	odm_Tx_By_TxDesc_or_Reg(pDM_Odm, TX_BY_REG);

	/*[--8188F---]*/
	#if (RTL8188F_SUPPORT == 1)
	else if (pDM_Odm->SupportICType == ODM_RTL8188F) {
		
		pDM_Odm->AntDivType = S0S1_SW_ANTDIV;
		odm_S0S1_SWAntDiv_Init_8188F(pDM_Odm);
	}
	#endif
	/*
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** SupportICType=[%lu]\n",pDM_Odm->SupportICType));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** AntDiv SupportAbility=[%lu]\n",(pDM_Odm->SupportAbility & ODM_BB_ANT_DIV)>>6));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** AntDiv Type=[%d]\n",pDM_Odm->AntDivType));
	*/
}

VOID
ODM_AntDiv(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PADAPTER		pAdapter	= pDM_Odm->Adapter;
	pFAT_T			pDM_FatTable = &pDM_Odm->DM_FatTable;
	#ifdef CONFIG_HL_SMART_ANTENNA_TYPE1
	pSAT_T			pdm_sat_table = &(pDM_Odm->dm_sat_table);
	#endif

	if(*pDM_Odm->pBandType == ODM_BAND_5G )
	{
		if(pDM_FatTable->idx_AntDiv_counter_5G <  pDM_Odm->antdiv_period )
		{
			pDM_FatTable->idx_AntDiv_counter_5G++;
			return;
		}
		else
			pDM_FatTable->idx_AntDiv_counter_5G=0;
	}
	else 	if(*pDM_Odm->pBandType == ODM_BAND_2_4G )
	{
		if(pDM_FatTable->idx_AntDiv_counter_2G <  pDM_Odm->antdiv_period )
		{
			pDM_FatTable->idx_AntDiv_counter_2G++;
			return;
		}
		else
			pDM_FatTable->idx_AntDiv_counter_2G=0;
	}
	
	//----------
	if(!(pDM_Odm->SupportAbility & ODM_BB_ANT_DIV))
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("[Return!!!]   Not Support Antenna Diversity Function\n"));
		return;
	}

	//----------

	//----------

	if (pDM_Odm->antdiv_select==1)
		pDM_Odm->AntType = ODM_FIX_MAIN_ANT;
	else if (pDM_Odm->antdiv_select==2)
		pDM_Odm->AntType = ODM_FIX_AUX_ANT;
	else  //if (pDM_Odm->antdiv_select==0)
		pDM_Odm->AntType = ODM_AUTO_ANT;

	//ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV,ODM_DBG_LOUD,("AntType= (( %d )) , pre_AntType= (( %d ))  \n",pDM_Odm->AntType,pDM_Odm->pre_AntType));
	
	if(pDM_Odm->AntType != ODM_AUTO_ANT)
	{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Fix Antenna at (( %s ))\n",(pDM_Odm->AntType == ODM_FIX_MAIN_ANT)?"MAIN":"AUX"));
			
		if(pDM_Odm->AntType != pDM_Odm->pre_AntType)
		{
			odm_AntDiv_on_off(pDM_Odm, ANTDIV_OFF);
			odm_Tx_By_TxDesc_or_Reg(pDM_Odm, TX_BY_REG);
						
			if(pDM_Odm->AntType == ODM_FIX_MAIN_ANT)
				ODM_UpdateRxIdleAnt(pDM_Odm, MAIN_ANT);
			else if(pDM_Odm->AntType == ODM_FIX_AUX_ANT)
				ODM_UpdateRxIdleAnt(pDM_Odm, AUX_ANT);
		}
		pDM_Odm->pre_AntType=pDM_Odm->AntType; 
		return;
	}
	else
	{
		if(pDM_Odm->AntType != pDM_Odm->pre_AntType)
		{
			odm_AntDiv_on_off(pDM_Odm, ANTDIV_ON);
			/*odm_Tx_By_TxDesc_or_Reg(pDM_Odm, TX_BY_DESC);*/
		}
		pDM_Odm->pre_AntType=pDM_Odm->AntType;
	}
	 
	
	//3 -----------------------------------------------------------------------------------------------------------
	//2 [--88E---]
	if(pDM_Odm->SupportICType == ODM_RTL8188E)
	{
		#if (RTL8188E_SUPPORT == 1)
		if(pDM_Odm->AntDivType==CG_TRX_HW_ANTDIV ||pDM_Odm->AntDivType==CGCS_RX_HW_ANTDIV)
			odm_HW_AntDiv(pDM_Odm);

		#if( defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY) ) ||( defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY) )
		else if (pDM_Odm->AntDivType==CG_TRX_SMART_ANTDIV)
			odm_FastAntTraining(pDM_Odm);	
		#endif
		
		#endif

	}

	#if (RTL8188F_SUPPORT == 1)	
	/* [--8188F---]*/
	else if (pDM_Odm->SupportICType == ODM_RTL8188F)	{
	
		#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY
		odm_S0S1_SwAntDiv(pDM_Odm, SWAW_STEP_PEEK);
		#endif		
	}
	#endif

}


VOID
odm_AntselStatistics(
	IN		PVOID			pDM_VOID,	
	IN		u1Byte			antsel_tr_mux,
	IN		u4Byte			MacId,
	IN		u4Byte			utility,
	IN            u1Byte			method

	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
	if(method==RSSI_METHOD)
	{
		if(antsel_tr_mux == ANT1_2G)
		{
			pDM_FatTable->MainAnt_Sum[MacId]+=utility;
			pDM_FatTable->MainAnt_Cnt[MacId]++;
		}
		else
		{
			pDM_FatTable->AuxAnt_Sum[MacId]+=utility;
			pDM_FatTable->AuxAnt_Cnt[MacId]++;
		}
	}
	#ifdef ODM_EVM_ENHANCE_ANTDIV
	else if(method==EVM_METHOD)
	{
		if(antsel_tr_mux == ANT1_2G)
		{
			pDM_FatTable->MainAntEVM_Sum[MacId]+=(utility<<5);
			pDM_FatTable->MainAntEVM_Cnt[MacId]++;
		}
		else
		{
			pDM_FatTable->AuxAntEVM_Sum[MacId]+=(utility<<5);
			pDM_FatTable->AuxAntEVM_Cnt[MacId]++;
		}
	}
	else if(method==CRC32_METHOD)
	{
		if(utility==0)
			pDM_FatTable->CRC32_Fail_Cnt++;
		else
			pDM_FatTable->CRC32_Ok_Cnt+=utility;
	}
	#endif
}


VOID
ODM_Process_RSSIForAntDiv(	
	IN OUT		PVOID			pDM_VOID,	
	IN			PVOID			p_phy_info_void,
	IN			PVOID			p_pkt_info_void
	//IN		PODM_PHY_INFO_T				pPhyInfo,
	//IN		PODM_PACKET_INFO_T			pPktinfo
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PODM_PHY_INFO_T 	pPhyInfo=(PODM_PHY_INFO_T)p_phy_info_void;
	PODM_PACKET_INFO_T	 pPktinfo=(PODM_PACKET_INFO_T)p_pkt_info_void;
	u1Byte			isCCKrate=0,CCKMaxRate=ODM_RATE11M;
	pFAT_T			pDM_FatTable = &pDM_Odm->DM_FatTable;
	#ifdef CONFIG_HL_SMART_ANTENNA_TYPE1
	pSAT_T			pdm_sat_table = &(pDM_Odm->dm_sat_table);
	u4Byte			beam_tmp;
	#endif


	CCKMaxRate=ODM_RATE11M;
	isCCKrate = (pPktinfo->DataRate <= CCKMaxRate)?TRUE:FALSE;
		
	if ((pDM_Odm->SupportICType & (ODM_RTL8192E|ODM_RTL8812)) && (pPktinfo->DataRate > CCKMaxRate))
	{
		RxPower_Ant0 = pPhyInfo->RxMIMOSignalStrength[0];
		RxPower_Ant1= pPhyInfo->RxMIMOSignalStrength[1];

		RxEVM_Ant0 =pPhyInfo->RxMIMOSignalQuality[0];
		RxEVM_Ant1 =pPhyInfo->RxMIMOSignalQuality[1];
	}
	else
		RxPower_Ant0=pPhyInfo->RxPWDBAll;

	#ifdef CONFIG_HL_SMART_ANTENNA_TYPE1
	if (pDM_Odm->AntDivType == HL_SW_SMART_ANT_TYPE1)
	{
		if ((pDM_Odm->SupportICType & ODM_HL_SMART_ANT_TYPE1_SUPPORT) && 
			(pPktinfo->bPacketToSelf)   && 
			(pDM_FatTable->FAT_State == FAT_TRAINING_STATE)
			) {
			
			if (pdm_sat_table->pkt_skip_statistic_en == 0) {
				/*
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("StaID[%d]:  antsel_pathA = ((%d)), hw_antsw_occur = ((%d)), Beam_num = ((%d)), RSSI = ((%d))\n",
					pPktinfo->StationID, pDM_FatTable->antsel_rx_keep_0, pDM_FatTable->hw_antsw_occur, pdm_sat_table->fast_training_beam_num, RxPower_Ant0));
				*/
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("StaID[%d]:  antsel_pathA = ((%d)), bPacketToSelf = ((%d)), Beam_num = ((%d)), RSSI = ((%d))\n",
					pPktinfo->StationID, pDM_FatTable->antsel_rx_keep_0, pPktinfo->bPacketToSelf, pdm_sat_table->fast_training_beam_num, RxPower_Ant0));

				
				pdm_sat_table->pkt_rssi_sum[pDM_FatTable->antsel_rx_keep_0][pdm_sat_table->fast_training_beam_num] += RxPower_Ant0;
				pdm_sat_table->pkt_rssi_cnt[pDM_FatTable->antsel_rx_keep_0][pdm_sat_table->fast_training_beam_num]++;
				pdm_sat_table->pkt_counter++;
			
				/*swich beam every N pkt*/
				if ((pdm_sat_table->pkt_counter) >= (pdm_sat_table->per_beam_training_pkt_num)) {

					pdm_sat_table->pkt_counter = 0;
					beam_tmp = pdm_sat_table->fast_training_beam_num;
									
					if (pdm_sat_table->fast_training_beam_num >= (pdm_sat_table->beam_patten_num_each_ant-1)) {
						
						pDM_FatTable->FAT_State = FAT_DECISION_STATE;
						
						ODM_ScheduleWorkItem(&pdm_sat_table->hl_smart_antenna_decision_workitem);


					} else {
						pdm_sat_table->fast_training_beam_num++;
						phydm_set_all_ant_same_beam_num(pDM_Odm);
						
						pDM_FatTable->FAT_State = FAT_TRAINING_STATE;	
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Update  Beam_num (( %d )) -> (( %d ))\n", beam_tmp, pdm_sat_table->fast_training_beam_num));
					}
				}
			}
		}
	} else 
	#endif
	if (pDM_Odm->AntDivType == CG_TRX_SMART_ANTDIV) {
		if( (pDM_Odm->SupportICType & ODM_SMART_ANT_SUPPORT) &&  (pPktinfo->bPacketToSelf)   && (pDM_FatTable->FAT_State == FAT_TRAINING_STATE) )//(pPktinfo->bPacketMatchBSSID && (!pPktinfo->bPacketBeacon))
		{
			u1Byte	antsel_tr_mux;
			antsel_tr_mux = (pDM_FatTable->antsel_rx_keep_2<<2) |(pDM_FatTable->antsel_rx_keep_1 <<1) |pDM_FatTable->antsel_rx_keep_0;
			pDM_FatTable->antSumRSSI[antsel_tr_mux] += RxPower_Ant0;
			pDM_FatTable->antRSSIcnt[antsel_tr_mux]++;
		}
	}
	else //AntDivType != CG_TRX_SMART_ANTDIV 
	{
		if ((pDM_Odm->SupportICType & ODM_ANTDIV_SUPPORT) && (pPktinfo->bPacketToSelf || pDM_FatTable->use_ctrl_frame_antdiv))
		{
			if(pDM_Odm->SupportICType == ODM_RTL8188E || pDM_Odm->SupportICType == ODM_RTL8192E)
			{
				odm_AntselStatistics(pDM_Odm, pDM_FatTable->antsel_rx_keep_0, pPktinfo->StationID,RxPower_Ant0,RSSI_METHOD);

				#ifdef ODM_EVM_ENHANCE_ANTDIV
				if(!isCCKrate)
				{
					odm_AntselStatistics(pDM_Odm, pDM_FatTable->antsel_rx_keep_0, pPktinfo->StationID,RxEVM_Ant0,EVM_METHOD);
				}
				#endif
			}
			else// SupportICType == ODM_RTL8821 and ODM_RTL8723B and ODM_RTL8812)
			{
				if(isCCKrate && (pDM_Odm->AntDivType == S0S1_SW_ANTDIV))
				{
				 	pDM_FatTable->antsel_rx_keep_0 = (pDM_FatTable->RxIdleAnt == MAIN_ANT) ? ANT1_2G : ANT2_2G;


					if(pDM_FatTable->antsel_rx_keep_0==ANT1_2G)
						pDM_FatTable->CCK_counter_main++;
					else// if(pDM_FatTable->antsel_rx_keep_0==ANT2_2G)
						pDM_FatTable->CCK_counter_aux++;

					odm_AntselStatistics(pDM_Odm, pDM_FatTable->antsel_rx_keep_0, pPktinfo->StationID, RxPower_Ant0,RSSI_METHOD);
				}
				else
				{
					if(pDM_FatTable->antsel_rx_keep_0==ANT1_2G)
						pDM_FatTable->OFDM_counter_main++;
					else// if(pDM_FatTable->antsel_rx_keep_0==ANT2_2G)
						pDM_FatTable->OFDM_counter_aux++;
					odm_AntselStatistics(pDM_Odm, pDM_FatTable->antsel_rx_keep_0, pPktinfo->StationID, RxPower_Ant0,RSSI_METHOD);
				}
			}
		}
	}
	//ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("isCCKrate=%d, PWDB_ALL=%d\n",isCCKrate, pPhyInfo->RxPWDBAll));
	//ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("antsel_tr_mux=3'b%d%d%d\n",pDM_FatTable->antsel_rx_keep_2, pDM_FatTable->antsel_rx_keep_1, pDM_FatTable->antsel_rx_keep_0));
}


VOID
ODM_AntDiv_Config(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pFAT_T			pDM_FatTable = &pDM_Odm->DM_FatTable;
		/* do noting, branch only */

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("SupportAbility = (( %x ))\n", pDM_Odm->SupportAbility ));

}


VOID
ODM_AntDivTimers(
	IN		PVOID		pDM_VOID,	
	IN 		u1Byte		state
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	if(state==INIT_ANTDIV_TIMMER)
	{
		#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY
			ODM_InitializeTimer(pDM_Odm, &(pDM_Odm->DM_SWAT_Table.phydm_SwAntennaSwitchTimer),
			(RT_TIMER_CALL_BACK)ODM_SW_AntDiv_Callback, NULL, "phydm_SwAntennaSwitchTimer");
		#elif ( defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY) ) ||( defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY) )
			ODM_InitializeTimer(pDM_Odm,&pDM_Odm->FastAntTrainingTimer,
			(RT_TIMER_CALL_BACK)odm_FastAntTrainingCallback, NULL, "FastAntTrainingTimer");
		#endif

		#ifdef ODM_EVM_ENHANCE_ANTDIV
			ODM_InitializeTimer(pDM_Odm,&pDM_Odm->EVM_FastAntTrainingTimer,
			(RT_TIMER_CALL_BACK)odm_EVM_FastAntTrainingCallback, NULL, "EVM_FastAntTrainingTimer");
		#endif
	}
	else if(state==CANCEL_ANTDIV_TIMMER)
	{
		#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY
			ODM_CancelTimer(pDM_Odm, &(pDM_Odm->DM_SWAT_Table.phydm_SwAntennaSwitchTimer));
		#elif ( defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY) ) ||( defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY) )
			ODM_CancelTimer(pDM_Odm,&pDM_Odm->FastAntTrainingTimer);
		#endif

		#ifdef ODM_EVM_ENHANCE_ANTDIV
			ODM_CancelTimer(pDM_Odm,&pDM_Odm->EVM_FastAntTrainingTimer);
		#endif
	}
	else if(state==RELEASE_ANTDIV_TIMMER)
	{
		#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY
			ODM_ReleaseTimer(pDM_Odm, &(pDM_Odm->DM_SWAT_Table.phydm_SwAntennaSwitchTimer));
		#elif ( defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY) ) ||( defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY) )
			ODM_ReleaseTimer(pDM_Odm,&pDM_Odm->FastAntTrainingTimer);
		#endif

		#ifdef ODM_EVM_ENHANCE_ANTDIV
			ODM_ReleaseTimer(pDM_Odm,&pDM_Odm->EVM_FastAntTrainingTimer);
		#endif
	}

}

#endif /*#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))*/

VOID
ODM_AntDivReset(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;

	if (pDM_Odm->AntDivType == S0S1_SW_ANTDIV)
	{
		#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY
		odm_S0S1_SWAntDiv_Reset(pDM_Odm);
		#endif
	}

}

VOID
odm_AntennaDiversityInit(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	
	#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
		ODM_AntDiv_Config(pDM_Odm);
		ODM_AntDivInit(pDM_Odm);
	#endif
}

VOID
odm_AntennaDiversity(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;

	#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
		ODM_AntDiv(pDM_Odm);
	#endif
}


