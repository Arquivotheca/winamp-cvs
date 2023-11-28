/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2002-2007 Intel Corporation. All Rights Reserved.
//
*/

#include "ac3_dec.h"
#include "ac3_dec_own_fp.h"
#include "ac3_dec_tables.h"

/********************************************************************/

static const float dialnormTable[32] = {
  1.0000000000f, 0.0312500000f, 0.0350769386f, 0.0393725336f,
  0.0441941731f, 0.0496062823f, 0.0556811690f, 0.0625000000f,
  0.0701538771f, 0.0787450671f, 0.0883883461f, 0.0992125645f,
  0.1113623381f, 0.1250000000f, 0.1403077543f, 0.1574901342f,
  0.1767766923f, 0.1984251291f, 0.2227246761f, 0.2500000000f,
  0.2806155086f, 0.3149802685f, 0.3535533845f, 0.3968502581f,
  0.4454493523f, 0.5000000000f, 0.5612310171f, 0.6299605370f,
  0.7071067691f, 0.7937005162f, 0.8908987045f, 1.0000000000f
};

/********************************************************************/

int32_t ParseSyncInfo(AC3Dec *state,
                     sBitsreamBuffer *pBS)
{
  int32_t tmp;
  /* To avoid division by zero in the case of BAD_STREAM */
  state->syncinfo.SampleRate = 48000;
 /* Get crc1 - we don't actually use this data though */
  GET_BITS(pBS, tmp, 16, int32_t)

 /* Get the sampling rate */
  GET_BITS(pBS, (state->syncinfo.fscod), 2, int32_t)
  if (state->syncinfo.fscod == 3)
    return 1;

 /* Get the frame size code */
  GET_BITS(pBS, (state->syncinfo.frmsizecod), 6, int32_t)
  if (state->syncinfo.frmsizecod > 37)
    return -1;

 /* Parse information */
  state->syncinfo.SampleRate = SAMPLING_RATE[state->syncinfo.fscod];
  state->syncinfo.bit_rate = FRAMESIZECODE[state->syncinfo.frmsizecod].bit_rate;
  state->syncinfo.frame_size =
    FRAMESIZECODE[state->syncinfo.frmsizecod].frm_size[state->syncinfo.fscod];

  return 0;
}

/********************************************************************/

int32_t ParseBsi(AC3Dec *state,
                sBitsreamBuffer *pBS)
{
  int32_t i, tmp;

 /* Check the AC-3 version number */
  GET_BITS(pBS, (state->bsi.bsid), 5, int32_t)
  if (state->bsi.bsid > 8)
    return -2;

 /* Get the audio service provided by the stream */
  GET_BITS(pBS, (state->bsi.bsmod), 3, int32_t)

 /* Get the audio coding mode (ie how many channels) */
  GET_BITS(pBS, (state->bsi.acmod), 3, int32_t)

  state->bsi.karaokeMode = 0;
  if ((state->bsi.bsmod == 7) && (state->bsi.acmod > 1)) {
    state->bsi.karaokeMode = 1;
  }

 /* Predecode the number of full bandwidth channels as we use this number a lot */
  state->bsi.nfchans = NFCHANS[state->bsi.acmod];

 /* If it is in use, get the centre channel mix level */
  if ((state->bsi.acmod & 0x1) && (state->bsi.acmod != 0x1)) {
    GET_BITS(pBS, (state->bsi.cmixlev), 2, int32_t)
  }

 /* If it is in use, get the surround channel mix level */
  if (state->bsi.acmod & 0x4) {
    GET_BITS(pBS, (state->bsi.surmixlev), 2, int32_t)
  }

 /* Get the dolby surround mode if in 2/0 mode */
  if (state->bsi.acmod == 0x2) {
    GET_BITS(pBS, (state->bsi.dsurmod), 2, int32_t)
  }

 /* Is the low frequency effects channel on? */
  GET_BITS(pBS, (state->bsi.lfeon), 1, int32_t)

  /* Set output parameters as input ones */
  if (state->as_input) {
    state->out_acmod = state->bsi.acmod;
    state->outlfeon = state->bsi.lfeon;

    state->nChannelOut = NFCHANS[state->out_acmod] +
                         state->outlfeon;
  }

 /* Get the dialogue normalization level */
  GET_BITS(pBS, (tmp), 5, int32_t)
  state->bsi.dialnorm = dialnormTable[tmp];

 /* Does compression gain exist? */
  GET_BITS(pBS, (state->bsi.compre), 1, int32_t)
  if (state->bsi.compre) {
 /* Get compression gain */
    GET_BITS(pBS, (tmp), 8, int32_t)
    state->bsi.compr = (tmp << 24) >> 8;
  }

 /* Does language code exist? */
  GET_BITS(pBS, (state->bsi.langcode), 1, int32_t)
  if (state->bsi.langcode) {
 /* Get langauge code */
    GET_BITS(pBS, (state->bsi.langcod), 8, int32_t)
  }

 /* Does audio production info exist? */
  GET_BITS(pBS, (state->bsi.audprodie), 1, int32_t)
  if (state->bsi.audprodie) {
 /* Get mix level */
    GET_BITS(pBS, (state->bsi.mixlevel), 5, int32_t)

 /* Get room type */
    GET_BITS(pBS, (state->bsi.roomtyp), 2, int32_t)
  }

 /* If we're in dual mono mode then get some extra info */
  if (state->bsi.acmod == 0) {
 /* Get the dialogue normalization level two */
    GET_BITS(pBS, (tmp), 5, int32_t)
    state->bsi.dialnorm2 = dialnormTable[tmp];

 /* Does compression gain two exist? */
    GET_BITS(pBS, (state->bsi.compr2e), 1, int32_t)
    if (state->bsi.compr2e) {
 /* Get compression gain two */
      GET_BITS(pBS, (tmp), 8, int32_t)
      state->bsi.compr2 = (tmp << 24) >> 8;
    }

 /* Does language code two exist? */
    GET_BITS(pBS, (state->bsi.langcod2e), 1, int32_t)
    if (state->bsi.langcod2e) {
 /* Get langauge code two */
      GET_BITS(pBS, (state->bsi.langcod2), 8, int32_t)
    }

 /* Does audio production info two exist? */
    GET_BITS(pBS, (state->bsi.audprodi2e), 1, int32_t)
    if (state->bsi.audprodi2e) {
 /* Get mix level two */
      GET_BITS(pBS, (state->bsi.mixlevel2), 5, int32_t)

 /* Get room type two */
      GET_BITS(pBS, (state->bsi.roomtyp2), 2, int32_t)
    }
  }

 /* Get the c o p y r i g h t  b i t */
  GET_BITS(pBS, (state->bsi.copyrightb), 1, int32_t)

 /* Get the original bit */
  GET_BITS(pBS, (state->bsi.origbs), 1, int32_t)

 /* Does timecode one exist? */
  GET_BITS(pBS, (state->bsi.timecod1e), 1, int32_t)

  if (state->bsi.timecod1e) {
    GET_BITS(pBS, (state->bsi.timecod1), 14, int32_t)
  }

 /* Does timecode two exist? */
  GET_BITS(pBS, (state->bsi.timecod2e), 1, int32_t)

  if (state->bsi.timecod2e) {
    GET_BITS(pBS, (state->bsi.timecod2), 14, int32_t)
  }

 /* Does addition info exist? */
  GET_BITS(pBS, (state->bsi.addbsie), 1, int32_t)

  if (state->bsi.addbsie) {
 /* Get how much info is there */
    GET_BITS(pBS, (state->bsi.addbsil), 6, int32_t)

 /* Get the additional info */
    for (i = 0; i < (int32_t)(state->bsi.addbsil + 1); i++) {
      GET_BITS(pBS, (state->bsi.addbsi[i]), 8, uint8_t)
    }
  }

  return 0;
}

/********************************************************************/

int32_t ParseAudblk(int32_t nblk,
                   AC3Dec *state,
                   sBitsreamBuffer *pBS)
{
  int32_t ncplsubnd;
  int32_t tmp, bitAlloc;
  int32_t prevCplinu, prevCplbegf, prevCplendf;
  int32_t i, j;
  _AudBlk *audblk = &(state->audblk);

  if (nblk == 0) {      /* audioblock 0? If so, reset these parameters */
    int32_t bin;
    audblk->dynrng = 0;
    audblk->dynrng2 = 0;
    //audblk->cpldeltnseg = 0;
    audblk->cpldeltlastbin = -256;

    for (bin = 0; bin < 50; bin++) {
      state->cpldeltba[bin] = 0;
    }

    for (i = 0; i < state->bsi.nfchans; i++) {
      for (bin = 0; bin < 50; bin++) {
        state->deltba[i][bin] = 0;
      }
      //audblk->deltnseg[i] = 0;
      audblk->firstChincpl[i] = 1;
    }

    for (i = 0; i < 18; i++) {
      audblk->phscor[i] = 0;
    }

    audblk->phsoutmod = 7;
    audblk->firstCplinu = 1;
    prevCplinu = 0;
    prevCplbegf = 0;
    prevCplendf = 0;
  } else {
    prevCplinu = audblk->cplinu;
    prevCplbegf = audblk->cplbegf;
    prevCplendf = audblk->cplendf;
  }


  for (i = 0; i < 6; i++) {
    audblk->bitAllocation[i] = 0;
  }
  audblk->CplBitAllocation = 0;
  audblk->LfeBitAllocation = 0;
  bitAlloc = 0;

  /* Is this channels are interleaved ? */
  /* Should we dither this channel? */
  GET_BITS(pBS, (audblk->blksw), 2*(state->bsi.nfchans), int32_t)
  audblk->dithflag = audblk->blksw & ((1 << state->bsi.nfchans) - 1);
  audblk->blksw = audblk->blksw >> state->bsi.nfchans;

 /* Does dynamic range control exist? */
  GET_BITS(pBS, (audblk->dynrnge), 1, int32_t)
  if (audblk->dynrnge) {
 /* Get dynamic range info */
    GET_BITS(pBS, (tmp), 8, int32_t)
    audblk->dynrng = (tmp << 24) >> 9;
  }

 /* If we're in dual mono mode then get the second channel DR info */
  if (state->bsi.acmod == 0) {
 /* Does dynamic range control two exist? */
    GET_BITS(pBS, (audblk->dynrng2e), 1, int32_t)
    if (audblk->dynrng2e) {
 /* Get dynamic range info */
      GET_BITS(pBS, (tmp), 8, int32_t)
      audblk->dynrng2 = (tmp << 24) >> 9;
    }
  }

 /* Does coupling strategy exist? */
  GET_BITS(pBS, (audblk->cplstre), 1, int32_t)

  if ((nblk == 0) && (audblk->cplstre == 0)) {
    return 0;
  }

  if (audblk->cplstre) {
 /* Is coupling turned on? */
    GET_BITS(pBS, (audblk->cplinu), 1, int32_t)
    if (audblk->cplinu) {
      GET_BITS(pBS, tmp, state->bsi.nfchans, int32_t)

      /* don't sure if this error is critical */
      //if (tmp == 0) {
      //  return 0;
      //}

      for (i = state->bsi.nfchans - 1; i >= 0; i--) {
        audblk->chincpl[i] = tmp & 1;
        tmp >>= 1;
      }

      if (state->bsi.acmod == 0x2) {
        GET_BITS(pBS, (audblk->phsflginu), 1, int32_t)
      } else audblk->phsflginu = 0;

      GET_BITS(pBS, (audblk->cplbegf), 4, int32_t)
      GET_BITS(pBS, (audblk->cplendf), 4, int32_t)

      ncplsubnd = (int32_t)audblk->cplendf - audblk->cplbegf + 3;
      if (ncplsubnd <= 0) {
        return 0;
      }

 /* Calculate the start and end bins of the coupling channel */
      audblk->cplstrtmant = ((audblk->cplbegf * 12) + 37);
      audblk->cplendmant = (((audblk->cplendf + 3) * 12) + 37);

 /* The number of combined subbands is ncplsubnd minus each combined band */
      audblk->ncplbnd = ncplsubnd;
      audblk->cplbndstrc[0] = 0;
      GET_BITS(pBS, tmp, ncplsubnd - 1, int32_t)
      for (i = ncplsubnd - 1; i >= 1; i--) {
        audblk->cplbndstrc[i] = tmp & 1;
        tmp >>= 1;
      }

#ifndef REF_DECODER_COMPATIBLE
      for (i = 1; i < ncplsubnd; i++) {
        audblk->ncplbnd -= (int32_t)audblk->cplbndstrc[i];
      }
#endif

    } else {
      for (i = 0; i < state->bsi.nfchans; i++)
        audblk->chincpl[i] = 0;

      audblk->phsflginu = 0;
    }
  }

  if (audblk->cplinu) {
 /* Loop through all the channels and get their coupling co-ords */
    for (i = 0; i < state->bsi.nfchans; i++) {
      if (!audblk->chincpl[i])
        continue;

 /* Is there new coupling co-ordinate info? */
      GET_BITS(pBS, (audblk->cplcoe[i]), 1, int32_t)

      if ((audblk->firstChincpl[i]) && ((audblk->cplcoe[i]) == 0)) {
        return 0;
      }

      audblk->firstChincpl[i] = 0;

      if (audblk->cplcoe[i]) {
        GET_BITS(pBS, (audblk->mstrcplco[i]), 2, int32_t)
        for (j = 0; j < audblk->ncplbnd; j++) {
          int32_t tmp0, tmp1;
#ifdef REF_DECODER_COMPATIBLE
          if (audblk->cplbndstrc[j] == 0) {
#endif
            GET_BITS(pBS, (tmp0), 4, int32_t)
            GET_BITS(pBS, (tmp1), 4, int32_t)

            if (tmp0 == 15) {
              tmp0 = tmp0 + 4 + 3 * audblk->mstrcplco[i];
              audblk->cplcoord[i][j] = (float)tmp1 / (float)(1 << tmp0);
            } else {
              tmp0 = tmp0 + 5 + 3 * audblk->mstrcplco[i];
              audblk->cplcoord[i][j] = (float)(tmp1 + 16) / (float)(1 << tmp0);
            }

            audblk->cplcoord[i][j] *= 8;
#ifdef REF_DECODER_COMPATIBLE
          } else {
            audblk->cplcoord[i][j] = audblk->cplcoord[i][j-1];
          }
#endif
        }
      }
    }

 /* If we're in 2/0 (stereo) mode, there's going to be some phase info */
    if ((state->bsi.acmod == 0x2) && audblk->phsflginu &&
        ((audblk->cplcoe[0] || audblk->cplcoe[1]))) {
#ifdef REF_DECODER_COMPATIBLE
      int32_t phsflg = 0;
#endif
      for (j = 0; j < audblk->ncplbnd; j++) {
#ifndef REF_DECODER_COMPATIBLE
        GET_BITS(pBS, (audblk->phsflg[j]), 1, int32_t)
#else
        if (audblk->cplbndstrc[j] == 0) {
          GET_BITS(pBS, (audblk->phsflg[j]), 1, int32_t)
          phsflg = audblk->phsflg[j];
        }
        if (phsflg) {
          if (audblk->cplcoord[1][j] > 0) {
            audblk->cplcoord[1][j] = -audblk->cplcoord[1][j];
          }
        } else {
          if (audblk->cplcoord[1][j] < 0) {
            audblk->cplcoord[1][j] = -audblk->cplcoord[1][j];
          }
        }
#endif
      }
    }
  }

 /* If we're in 2/0 (stereo) mode, there may be a rematrix strategy */
  if (state->bsi.acmod == 0x2) {
    GET_BITS(pBS, (audblk->rematstr), 1, int32_t)

    //if ((nblk == 0) || (audblk->rematstr == 0)) {
    //  return 0;
    //}

    if (audblk->rematstr) {
      if ((audblk->cplinu == 0) ||
          ((audblk->cplbegf > 2) && audblk->cplinu)) {
        audblk->nrematbnds = 4;
      } else if ((audblk->cplbegf == 0) && audblk->cplinu) {
        audblk->nrematbnds = 2;
      } else {
        audblk->nrematbnds = 3;
      }

      GET_BITS(pBS, (audblk->rematflg), audblk->nrematbnds, int32_t)
    }
  }

  audblk->cplexpstr = 0;
  if (audblk->cplinu) {
 /* Get the coupling channel exponent strategy */
    GET_BITS(pBS, (audblk->cplexpstr), 2, int32_t)

    if ((audblk->firstCplinu) && (audblk->cplexpstr == EXP_REUSE)) {
      return 0;
    }

    if (((audblk->cplbegf != prevCplbegf) ||
         (audblk->cplendf != prevCplendf)) &&
         (audblk->cplexpstr == EXP_REUSE)) {
      return 0;
    }

    audblk->ncplgrps = 0;
    if (audblk->cplexpstr != EXP_REUSE) {
      audblk->CplBitAllocation = 1;
      audblk->ncplgrps = ((audblk->cplendmant - audblk->cplstrtmant) /
                          (3 << (audblk->cplexpstr - 1)));
    }
  }

  for (i = 0; i < state->bsi.nfchans; i++) {
    GET_BITS(pBS, (audblk->chexpstr[i]), 2, int32_t)
    if ((nblk == 0) && (audblk->chexpstr[i] == EXP_REUSE)) {
      return 0;
    }

    if ((audblk->cplinu == 1) &&
        (audblk->cplbegf != prevCplbegf) &&
        (audblk->chincpl[i] == 1) &&
        (audblk->chexpstr[i] == EXP_REUSE)) {
      return 0;
    }
  }

 /* Get the exponent strategy for lfe channel */
  audblk->lfeexpstr = 0;
  if (state->bsi.lfeon) {
    GET_BITS(pBS, (audblk->lfeexpstr), 1, int32_t)
    if ((nblk == 0) && (audblk->lfeexpstr == EXP_REUSE)) {
      return 0;
    }
  }

 /* Determine the bandwidths of all the fbw channels */
  for (i = 0; i < state->bsi.nfchans; i++) {
    int32_t grp_size;

    if (audblk->chexpstr[i] != EXP_REUSE) {
      audblk->bitAllocation[i] = 1;
      if (audblk->cplinu && audblk->chincpl[i]) {
        audblk->endmant[i] = audblk->cplstrtmant;
      } else {
        GET_BITS(pBS, (audblk->chbwcod[i]), 6, int32_t)
        if (audblk->chbwcod[i] > 60)
          audblk->chbwcod[i] = 60;
        audblk->endmant[i] = (((audblk->chbwcod[i] + 12) * 3) + 37);
      }

 /* Calculate the number of exponent groups to fetch */
      grp_size = (3 * (1 << (audblk->chexpstr[i] - 1)));
      audblk->nchgrps[i] = ((audblk->endmant[i] - 1 + (grp_size - 3)) / grp_size);
    }
  }

 /* Get the coupling exponents if they exist */
  if (audblk->cplinu && (audblk->cplexpstr != EXP_REUSE)) {
    GET_BITS(pBS, (audblk->cplabsexp), 4, int32_t)
    for (i = 0; i < audblk->ncplgrps; i++) {
      GET_BITS(pBS, (audblk->cplexps[i]), 7, int32_t)
    }
  }

 /* Get the fwb channel exponents */
  for (i = 0; i < state->bsi.nfchans; i++) {
    if (audblk->chexpstr[i] != EXP_REUSE) {
      GET_BITS(pBS, (audblk->exps[i][0]), 4, int32_t)
      for (j = 1; j <= audblk->nchgrps[i]; j++) {
        GET_BITS(pBS, (audblk->exps[i][j]), 7, int32_t)
      }
      GET_BITS(pBS, (audblk->gainrng[i]), 2, int32_t)
    }
  }

 /* Get the lfe channel exponents */
  if (state->bsi.lfeon && (audblk->lfeexpstr != EXP_REUSE)) {
    audblk->LfeBitAllocation = 1;
    GET_BITS(pBS, (audblk->lfeexps[0]), 4, int32_t)
    GET_BITS(pBS, (audblk->lfeexps[1]), 7, int32_t)
    GET_BITS(pBS, (audblk->lfeexps[2]), 7, int32_t)
  }

 /* Get the parametric bit allocation parameters */
  GET_BITS(pBS, (audblk->baie), 1, int32_t)

  if ((nblk == 0) && (audblk->baie == 0)) {
    return 0;
  }

  if (audblk->baie) {
    bitAlloc = 1;
    GET_BITS(pBS, (audblk->sdcycod), 2, int32_t)
    GET_BITS(pBS, (audblk->fdcycod), 2, int32_t)
    GET_BITS(pBS, (audblk->sgaincod), 2, int32_t)
    GET_BITS(pBS, (audblk->dbpbcod), 2, int32_t)
    GET_BITS(pBS, (audblk->floorcod), 3, int32_t)
  }

 /* Get the SNR off set info if it exists */
  GET_BITS(pBS, (audblk->snroffste), 1, int32_t)

  if ((nblk == 0) && (audblk->snroffste == 0)) {
    return 0;
  }

  if (audblk->snroffste) {
    bitAlloc = 1;
    GET_BITS(pBS, (audblk->csnroffst), 6, int32_t)

    if (audblk->cplinu) {
      GET_BITS(pBS, tmp, 7, int32_t)
      audblk->cplfsnroffst = tmp >> 3;
      audblk->cplfgaincod = tmp & 7;
    }

    for (i = 0; i < state->bsi.nfchans; i++) {
      GET_BITS(pBS, tmp, 7, int32_t)
      audblk->fsnroffst[i] = tmp >> 3;
      audblk->fgaincod[i] = tmp & 7;
    }
    if (state->bsi.lfeon) {
      GET_BITS(pBS, tmp, 7, int32_t)
      audblk->lfefsnroffst = tmp >> 3;
      audblk->lfefgaincod = tmp & 7;
    }
  }

 /* Get coupling leakage info if it exists */
  if (audblk->cplinu) {
    GET_BITS(pBS, (audblk->cplleake), 1, int32_t)

    if ((nblk == 0) && (audblk->cplleake == 0)) {
      return 0;
    }

    if (audblk->cplleake) {
      audblk->CplBitAllocation = 1;

      GET_BITS(pBS, tmp, 6, int32_t)
      audblk->cplfleak = tmp >> 3;
      audblk->cplsleak = tmp & 7;
    }
  }

 /* Get the delta bit alloaction info */
  GET_BITS(pBS, (audblk->deltbaie), 1, int32_t)

  if (audblk->deltbaie) {
    bitAlloc = 1;
    if (audblk->cplinu) {
      GET_BITS(pBS, (audblk->cpldeltbae), 2, int32_t)
    }

    for (i = 0; i < state->bsi.nfchans; i++) {
      GET_BITS(pBS, (audblk->deltbae[i]), 2, int32_t)
    }

    if (audblk->cplinu) {
      if (audblk->cpldeltbae == 1) {
        int32_t deltnseg, deltoffst, deltlen, deltba;
        int32_t bin;

        GET_BITS(pBS, deltnseg, 3, int32_t)
        deltnseg += 1;
        bin = 0;
        audblk->cpldeltlastbin = -256;
        for (j = 0; j < deltnseg; j++) {
          GET_BITS(pBS, tmp, 12, int32_t)
          deltoffst = tmp >> 7;
          deltlen = (tmp >> 3) & 0xF;
          deltba = tmp & 7;

          while (deltoffst--) {
            state->cpldeltba[bin] = 0;
            bin++;
          }

          if ((bin + deltlen) > 50) {
            return 0;
          }

          deltba -= (deltba >= 4) ? 3 : 4;
          deltba <<= 7;
          while (deltlen--) {
            state->cpldeltba[bin] = deltba;
            bin++;
            audblk->cpldeltlastbin = bin;
          }
        }

        for (; bin < 50; bin++) {
          state->cpldeltba[bin] = 0;
        }

      } else if (audblk->cpldeltbae >= 2) {
        int32_t bin;

        for (bin = 0; bin < 50; bin++) {
          state->cpldeltba[bin] = 0;
        }
        audblk->cpldeltlastbin = -256;
      }

      if ((audblk->cpldeltlastbin + MASKTAB[audblk->cplstrtmant]) > 50) {
        return 0;
      }
    }

    for (i = 0; i < state->bsi.nfchans; i++) {
      if (audblk->deltbae[i] == 1) {
        int32_t deltnseg, deltoffst, deltlen, deltba;
        int32_t bin;

        GET_BITS(pBS, deltnseg, 3, int32_t)
        bin = 0;
        for (j = 0; j <= deltnseg; j++) {
          GET_BITS(pBS, tmp, 12, int32_t)
          deltoffst = tmp >> 7;
          deltlen = (tmp >> 3) & 0xF;
          deltba = tmp & 7;

          while (deltoffst--) {
            state->deltba[i][bin] = 0;
            bin++;
          }

          if ((bin + deltlen) > 50) {
            return 0;
          }

          deltba -= (deltba >= 4) ? 3 : 4;
          deltba <<= 7;
          while (deltlen--) {
            state->deltba[i][bin] = deltba;
            bin++;
          }
        }
        for (; bin < 50; bin++) {
          state->deltba[i][bin] = 0;
        }
      } else if (audblk->deltbae[i] >= 2) {
        int32_t bin;

        for (bin = 0; bin < 50; bin++) {
          state->deltba[i][bin] = 0;
        }
      }
    }
  }

 /* Check to see if there's any dummy info to get */
  GET_BITS(pBS, (audblk->skiple), 1, int32_t)
  if (audblk->skiple) {
    int32_t skip_data;
    int32_t numReadedBits = 0;
    int32_t numSkipBits;
    int32_t tmp;

    GET_BITS(pBS, (audblk->skipl), 9, int32_t)

    if ((audblk->skipl > 0) && (audblk->cplinu)) {
      GET_BITS(pBS, (tmp), 1, int32_t)
      numReadedBits++;

      if (tmp) {
        GET_BITS(pBS, (audblk->phsoutmod), 3, int32_t)
        GET_BITS(pBS, (tmp), 1, int32_t)
        numReadedBits += 4;

        if (tmp) {
          for (j = 0; j < audblk->ncplbnd; j++) {
#ifdef REF_DECODER_COMPATIBLE
            if (audblk->cplbndstrc[j] == 0) {
#endif
              GET_BITS(pBS, (audblk->phscor[j]), 4, int32_t)
              numReadedBits += 4;
#ifdef REF_DECODER_COMPATIBLE
            } else {
              audblk->phscor[j] = audblk->phscor[j - 1];
            }
#endif
          }
        } else {
          GET_BITS(pBS, (audblk->phscor[0]), 4, int32_t)
          numReadedBits += 4;

          for (j = 1; j < audblk->ncplbnd; j++) {
            audblk->phscor[j] = audblk->phscor[0];
          }
        }
      }
    }

    numSkipBits = audblk->skipl * 8 - numReadedBits;

    for (i = 0; i < numSkipBits >> 3; i++) {
      GET_BITS(pBS, (skip_data), 8, int32_t)
    }

    numSkipBits &= 7;
    GET_BITS(pBS, (skip_data), numSkipBits, int32_t)
  }

  if (bitAlloc == 1) {
    for (i = 0; i < 6; i++) {
      audblk->bitAllocation[i] = 1;
    }
    audblk->CplBitAllocation = 1;
    audblk->LfeBitAllocation = 1;
  }

  if (audblk->cplinu) {
    audblk->firstCplinu = 0;
  }

  return 1;
}

/********************************************************************/

int32_t ParseAuxdata(AC3Dec *state,
                    sBitsreamBuffer *pBS,
                    int32_t start_bits)
{
  int32_t i, tmp;
  int32_t numSkipBits;

  GET_BITS_COUNT(pBS, tmp)
  tmp -= start_bits;
  numSkipBits = state->syncinfo.frame_size * 16 - tmp - 18;
  numSkipBits += 18; /* 1 bit - auxdata exists, 1 bit - CRC reserved bit, 16 bit - CRC */

  for (i = 0; i < numSkipBits >> 3; i++) {
    GET_BITS(pBS, tmp, 8, int32_t)
  }

  numSkipBits &= 7;
  GET_BITS(pBS, tmp, numSkipBits, int32_t)

  return 1;
}

/********************************************************************/

