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
  *******************************************************************************/
#define _USB_OPS_C_

#include <rtl8188f_hal.h>

static s32 pre_recv_entry(union recv_frame *precvframe, u8 *pphy_status)
{
	s32 ret = _SUCCESS;

	return ret;

}

#if 0
static s32 pre_recv_entry(union recv_frame *precvframe, u8 *pphy_status)
{
	s32 ret = _SUCCESS;

	return ret;

}
#endif

int recvbuf2recvframe(PADAPTER padapter, void *ptr)
{
	u8 *pbuf;
	u8 pkt_cnt = 0;
	u32 pkt_offset;
	s32 transfer_len;
	u8 *pdata, *pphy_status;
	union recv_frame *precvframe = NULL;
	struct rx_pkt_attrib *pattrib = NULL;
	PHAL_DATA_TYPE pHalData;
	struct recv_priv *precvpriv;
	_queue *pfree_recv_queue;
	_pkt *pskb;


	pHalData = GET_HAL_DATA(padapter);
	precvpriv = &padapter->recvpriv;
	pfree_recv_queue = &precvpriv->free_recv_queue;

#ifdef CONFIG_USE_USB_BUFFER_ALLOC_RX
	pskb = NULL;
	transfer_len = (s32)((struct recv_buf *)ptr)->transfer_len;
	pbuf = ((struct recv_buf *)ptr)->pbuf;
#else /* !CONFIG_USE_USB_BUFFER_ALLOC_RX */
	pskb = (_pkt *)ptr;
	transfer_len = (s32)pskb->len;
	pbuf = pskb->data;
#endif /* !CONFIG_USE_USB_BUFFER_ALLOC_RX */

#ifdef CONFIG_USB_RX_AGGREGATION
	pkt_cnt = GET_RX_STATUS_DESC_USB_AGG_PKTNUM_8188F(pbuf);
#endif

	do {
		precvframe = rtw_alloc_recvframe(pfree_recv_queue);
		if (precvframe == NULL) {
			DBG_8192C("%s: rtw_alloc_recvframe() failed! RX Drop!\n", __func__);
			goto _exit_recvbuf2recvframe;
		}

		if (transfer_len > 1500)
			_rtw_init_listhead(&precvframe->u.hdr.list);
		precvframe->u.hdr.precvbuf = NULL;	/*can't access the precvbuf for new arch. */
		precvframe->u.hdr.len = 0;

		rtl8188fu_rx_query_desc(precvframe, pbuf);

		pattrib = &precvframe->u.hdr.attrib;

		if (((pattrib->crc_err) || (pattrib->icv_err))) {
			DBG_8192C("%s: RX Warning! crc_err=%d icv_err=%d, skip!\n",
					  __func__, pattrib->crc_err, pattrib->icv_err);

			rtw_free_recvframe(precvframe, pfree_recv_queue);
			goto _exit_recvbuf2recvframe;
		}

		pkt_offset = RXDESC_SIZE + pattrib->drvinfo_sz + pattrib->shift_sz + pattrib->pkt_len;
		if ((pattrib->pkt_len <= 0) || (pkt_offset > transfer_len)) {
			DBG_8192C("%s: RX Error! pkt_len=%d pkt_offset=%d transfer_len=%d\n",
					  __func__, pattrib->pkt_len, pkt_offset, transfer_len);

			rtw_free_recvframe(precvframe, pfree_recv_queue);
			goto _exit_recvbuf2recvframe;
		}

		pdata = pbuf + RXDESC_SIZE + pattrib->drvinfo_sz + pattrib->shift_sz;
		if (rtw_os_alloc_recvframe(padapter, precvframe, pdata, pskb) == _FAIL) {
			DBG_8192C("%s: RX Error! rtw_os_alloc_recvframe FAIL!\n", __func__);

			rtw_free_recvframe(precvframe, pfree_recv_queue);
			goto _exit_recvbuf2recvframe;
		}

		recvframe_put(precvframe, pattrib->pkt_len);

		if (pattrib->pkt_rpt_type == NORMAL_RX) {
			if (pattrib->physt)
				pphy_status = pbuf + RXDESC_OFFSET;
			else
				pphy_status = NULL;

			if (pphy_status)
				rx_query_phy_status(precvframe, pphy_status);

			if (rtw_recv_entry(precvframe) != _SUCCESS) {
				/* Return fail except data frame */
				/*DBG_8192C("%s: RX Error! rtw_recv_entry FAIL!\n", __func__); */
			}
		} else {
#ifdef CONFIG_C2H_PACKET_EN
			if (pattrib->pkt_rpt_type == C2H_PACKET)
				rtl8188f_c2h_packet_handler(padapter, precvframe->u.hdr.rx_data, pattrib->pkt_len);
			else {
				DBG_8192C("%s: [WARNNING] RX type(%d) not be handled!\n",
						  __func__, pattrib->pkt_rpt_type);
			}
#endif /* CONFIG_C2H_PACKET_EN */
			rtw_free_recvframe(precvframe, pfree_recv_queue);
		}

#ifdef CONFIG_USB_RX_AGGREGATION
		/* jaguar 8-byte alignment */
		pkt_offset = (u16)_RND8(pkt_offset);
		pkt_cnt--;
		pbuf += pkt_offset;
#endif
		transfer_len -= pkt_offset;
		precvframe = NULL;
	} while (transfer_len > 0);

_exit_recvbuf2recvframe:

	return _SUCCESS;
}


void rtl8188fu_xmit_tasklet(void *priv)
{
	int ret = _FALSE;
	_adapter *padapter = (_adapter *)priv;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	while (1) {
		if (RTW_CANNOT_TX(padapter)) {
			DBG_8192C("xmit_tasklet => bDriverStopped or bSurpriseRemoved or bWritePortCancel\n");
			break;
		}

		if (rtw_xmit_ac_blocked(padapter) == _TRUE)
			break;

		ret = rtl8188fu_xmitframe_complete(padapter, pxmitpriv, NULL);

		if (ret == _FALSE)
			break;

	}

}



void rtl8188fu_set_intf_ops(struct _io_ops	*pops)
{

	_rtw_memset((u8 *)pops, 0, sizeof(struct _io_ops));

	pops->_read8 = &usb_read8;
	pops->_read16 = &usb_read16;
	pops->_read32 = &usb_read32;
	pops->_read_mem = &usb_read_mem;
	pops->_read_port = &usb_read_port;

	pops->_write8 = &usb_write8;
	pops->_write16 = &usb_write16;
	pops->_write32 = &usb_write32;
	pops->_writeN = &usb_writeN;

#ifdef CONFIG_USB_SUPPORT_ASYNC_VDN_REQ
	pops->_write8_async = &usb_async_write8;
	pops->_write16_async = &usb_async_write16;
	pops->_write32_async = &usb_async_write32;
#endif
	pops->_write_mem = &usb_write_mem;
	pops->_write_port = &usb_write_port;

	pops->_read_port_cancel = &usb_read_port_cancel;
	pops->_write_port_cancel = &usb_write_port_cancel;


}
void rtl8188fu_set_hw_type(struct dvobj_priv *pdvobj)
{
	pdvobj->HardwareType = HARDWARE_TYPE_RTL8188FU;
	DBG_871X("CHIP TYPE: RTL8188FU\n");
}
