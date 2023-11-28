/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggTheora SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE Theora SOURCE CODE IS COPYRIGHT (C) 2002-2009                *
 * by the Xiph.Org Foundation and contributors http://www.xiph.org/ *
 *                                                                  *
 ********************************************************************

  function:
    last mod: $Id: dequant.h,v 1.1 2010/04/12 04:10:09 audiodsp Exp $

 ********************************************************************/

#if !defined(_dequant_H)
# define _dequant_H (1)
# include "quant.h"
# include "bitpack.h"

int oc_quant_params_unpack(oc_pack_buf *_opb,
 th_quant_info *_qinfo);
void oc_quant_params_clear(th_quant_info *_qinfo);

#endif
