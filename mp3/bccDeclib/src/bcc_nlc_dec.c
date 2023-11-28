/***************************************************************************\
 *                         Fraunhofer IIS 
 *                 (c) 1996 - 2008 Fraunhofer IIS
 *          (c) 2004 Fraunhofer IIS and Agere Systems Inc.
 *                       All Rights Reserved.
 *
 *
 *    This software and/or program is protected by copyright law and
 *    international treaties. Any reproduction or distribution of this
 *    software and/or program, or any portion of it, may result in severe
 *    civil and criminal penalties, and will be prosecuted to the maximum
 *    extent possible under law.
 *
\***************************************************************************/

#include "bcc_nlc_dec.h"
#include "huff_nodes.h"

#define min(a,b) (((a) < (b)) ? (a) : (b))


extern const HUFF_NODES huffNodes[2][2];


static int pcm_decode( Streamin_state* strm,
                        int*            out_data_1,
                        int*            out_data_2,
                        int             num_val,
                        int             num_levels )
{
  int pcm_chunk_size_5lev   [] = { 0,  3,  5,  7,  0,  0 };
  int pcm_chunk_size_9lev   [] = { 0,  4,  7, 10, 13, 16 };
  int pcm_chunk_size_13lev  [] = { 0,  4,  8, 12, 15,  0 };
  int pcm_chunk_size_17lev  [] = { 0,  5,  9, 13, 17, 21 };
  int pcm_chunk_size_default[] = { 0,  0,  0,  0,  0,  0 };

  int i = 0, j = 0, idx = 0;
  int grp_len = 0, max_grp_len = 0, next_val = 0, grp_val = 0;

  int (*pcm_chunk_size)[6] = NULL;


  switch( num_levels ) {
  case 5:
    pcm_chunk_size = &pcm_chunk_size_5lev;
    max_grp_len = 3;
    break;
  case 9:
    pcm_chunk_size = &pcm_chunk_size_9lev;
    max_grp_len = 5;
    break;
  case 13:
    pcm_chunk_size = &pcm_chunk_size_13lev;
    max_grp_len = 4;
    break;
  case 17:
    pcm_chunk_size = &pcm_chunk_size_17lev;
    max_grp_len = 5;
    break;
  default:
    pcm_chunk_size = &pcm_chunk_size_default;
    (*pcm_chunk_size)[1] = (int) ceil( log((float)num_levels)/log(2.0) );
    max_grp_len = 1;
  }


  for( i=0; i<2*num_val; i+=max_grp_len ) {
    grp_len = min( max_grp_len, 2*num_val-i );
    grp_val = getbits( strm, (*pcm_chunk_size)[grp_len] );
    if(grp_val == -1)
      return(0);

    for( j=0; j<grp_len; j++ ) {
      idx = i+(grp_len-j-1);
      next_val = grp_val%num_levels;
      if(idx%2) {
        out_data_2[idx/2] = next_val;
      }
      else {
        out_data_1[idx/2] = next_val;
      }
      grp_val = (grp_val-next_val)/num_levels;
    }
  }
  return(1);

}


static int huff_read( Streamin_state  *strm,
                      const int      (*nodeTab)[][2] )
{
  int node = 0;

  do {
    int b =  getbits(strm,1);
    if(b == -1)
      return(1);                /* 1 error code */
    node = (*nodeTab)[node][b];
  } while( node > 0 );

  return node;
}


static int huff_read_2D( Streamin_state  *strm,
                         const int       (*nodeTab)[][2],
                         int             out_data[2] )
{
  int huff_2D_8bit = 0, node = 0, escape = 0;

  node = huff_read( strm, nodeTab );

  if(node == 1)
    return(-1);   /* -1 error code */


  escape = (node == 0);

  if( escape ) {
    out_data[0] = 0;
    out_data[1] = 1;
  }
  else {
    huff_2D_8bit = -(node+1);
    out_data[0] = huff_2D_8bit >> 4;
    out_data[1] = huff_2D_8bit & 0xf;
  }

  return escape;
}


static int sym_restore( Streamin_state* strm,
                         int             lav,
                         int             data[2] )
{
  /*int tmp = 0; */

  int sum_val  = data[0]+data[1];
  int diff_val = data[0]-data[1];

  if( sum_val > lav ) {
    data[0] = - sum_val + (2*lav+1);
    data[1] = - diff_val;
  }
  else {
    data[0] = sum_val;
    data[1] = diff_val;
  }

  if( data[0]+data[1] != 0 ) {
    int tmp = getbits( strm, 1 );
    if(tmp == -1)
      return(0);
    if( tmp ) {
      data[0] = -data[0];
      data[1] = -data[1];
    }
  }

  if( data[0]-data[1] != 0 ) {
    int tmp = getbits( strm, 1 );
    if(tmp == -1)
      return(0);

    if( tmp ) {
      tmp     = data[0];
      data[0] = data[1];
      data[1] = tmp;
    }
  }
  return(1);
}


static int huff_dec( Streamin_state*   strm,
                      const HUFF_NODES* huffNodes,
                      int               lav,
                      int               out_data[][2],
                      int               num_val,
                      int               stride )
{
  int i = 0, escape = 0, escCntr = 0;
  int esc_data[2][MAXPART] = {{0}};
  int escIdx[MAXPART] = {0};

  for( i=0; i<num_val; i+=stride ) {

    switch( lav ) {
    case 2:
      escape = huff_read_2D( strm, (void *)&huffNodes->lav2, out_data[i] );
      break;
    case 4:
      escape = huff_read_2D( strm, (void *)&huffNodes->lav4, out_data[i] );
      break;
    case 6:
      escape = huff_read_2D( strm, (void *)&huffNodes->lav6, out_data[i] );
      break;
    case 8:
      escape = huff_read_2D( strm, (void *)&huffNodes->lav8, out_data[i] );
      break;
    }

    if(escape == -1)
      return(0);

    if( escape ) {
      escIdx[escCntr++] = i;
    }
    else {
      if (!sym_restore( strm, lav, out_data[i] ))
        return(0);
    }

  }

  if( escCntr > 0 ) {
    if(!pcm_decode( strm, esc_data[0], esc_data[1], escCntr, (2*lav+1) ))
      return(0);

    for( i=0; i<escCntr; i++ ) {
      out_data[escIdx[i]][0] = esc_data[0][i] - lav;
      out_data[escIdx[i]][1] = esc_data[1][i] - lav;
    }
  }
  return(1);

}


static int huff_decode( Streamin_state* strm,
                         int*            out_data_1,
                         int*            out_data_2,
                         NLC_DIFF_TYPE   diff_type_1,
                         NLC_DIFF_TYPE   diff_type_2,
                         int             num_val )
{
  CODING_SCHEME cdg_scheme;
  int lav = 0, i = 0;
  int pair_vec[MAXPART][2];

  if(num_val%2)
    cdg_scheme = HUFF_2D_TIME;
  else{
    cdg_scheme = getbits( strm, 1 );
    if(cdg_scheme == -1)
    return(0);
  }


  lav = getbits( strm, 2 );
  if(lav == -1)
    return(0);

  lav = 2 * ( lav + 1 );

  switch( cdg_scheme ) {
  case HUFF_2D_FREQ:
    if(!huff_dec( strm, &huffNodes[diff_type_1][HUFF_2D_FREQ], lav, pair_vec  , num_val, 2 /* stride */ ))
      return(0);
    if(!huff_dec( strm, &huffNodes[diff_type_2][HUFF_2D_FREQ], lav, pair_vec+1, num_val, 2 /* stride */ ))
      return(0);

    for( i=0; i<num_val; i++ ) {
      if(i%2){
        out_data_2[i-1] = pair_vec[i][0];
        out_data_2[i  ] = pair_vec[i][1];
      }
      else{
        out_data_1[i  ] = pair_vec[i][0];
        out_data_1[i+1] = pair_vec[i][1];
      }
    }

    break;

  case HUFF_2D_TIME:
    if(!huff_dec( strm, &huffNodes[diff_type_1|diff_type_2][HUFF_2D_TIME], lav, pair_vec, num_val, 1 /* stride */ ))
      return(0);

    for( i=0; i<num_val; i++ ) {
      out_data_1[i] = pair_vec[i][0];
      out_data_2[i] = pair_vec[i][1];
    }

    break;

  default:
    break;
  }

  return(1);

}

static void diff_freq_decode( int  start_val,
                              int* diff_data,
                              int* out_data,
                              int  num_val )
{
  int i = 0;
  int prev_val = start_val;

  for( i=0; i<num_val; i++ ) {
    out_data[i] = prev_val + diff_data[i];
    prev_val = out_data[i];
  }
}


static void diff_time_decode_backwards( int* prev_data,
                                        int* diff_data,
                                        int* out_data,
                                        int  num_val )
{
  int i = 0;

  for( i=0; i<num_val; i++ ) {
    out_data[i] = prev_data[i] + diff_data[i];
  }
}


static void diff_time_decode_forwards( int* prev_data,
                                       int* diff_data,
                                       int* out_data,
                                       int  num_val )
{
  int i = 0;

  for( i=0; i<num_val; i++ ) {
    out_data[i] = prev_data[i] - diff_data[i];
  }
}


static int get_num_lsb( int* in_data,
                        int  center_val,
                        int  num_val )
{
  int i = 0, lsb_cnt = 0;

  for( i=0; i<num_val; i++ ) {
    if( in_data[i] != center_val ) {
      lsb_cnt++;
    }
  }

  return lsb_cnt;
}


static void attach_lsb( int*          in_out_data,
                        unsigned long lsb_bits,
                        int           num_lsb,
                        int           center_val,
                        int           num_val )
{
  int i = 0, val = 0, lsb = 0;
  int lsb_cnt = num_lsb-1;

  for( i=0; i<num_val; i++ ) {

    val = in_out_data[i]<<1;

    if( val != center_val ) {
      lsb = (lsb_bits>>lsb_cnt) & 0x0001;
      lsb_cnt--;

      val += (val<center_val) ? lsb : -lsb;
    }

    in_out_data[i] = val;
  }

}


BCC_STAT bcc_cues_nldec( Streamin_state* strm,
                         BCC_dparams*    params,
                         BCC_dstate*     state )
{
  int lfchanpos = 0, all_linear_coding = 0;
  int g = 0, c = 0, p = 0;
  int cue_levels = 0, lsb_trans_flag = 0;
  long lsb_bits = 0;
  int num_gran_lsb = 0;
  int indep = 0, invalid = 0;

  int num_lsb[MAXCUES], npart[MAXCUES];
  int linear_coding[MAX_GPF/2][MAXCUES];
  int* (*prev_ild)[MAXCUES];

  NLC_DIFF_TYPE diff_type[MAX_GPF][MAXCUES];
  DIRECTION     direction = BACKWARDS;


  lfchanpos = -1;

  if (params->lfchan >= 0) {
    if (params->refchan < params->lfchan) {
      lfchanpos = params->lfchan-1;
    }
    else {
      lfchanpos = params->lfchan;
    }
  }

  for( c=0; c<params->ncues; c++ ) {
    npart[c] = (c == lfchanpos) ? 1 : params->npart;
  }

  /* reduced number of cue levels before LSB attaching */
  cue_levels = (params->cuelevels-1)/2 + 1;


  /* read ioChannel active flags */
  for( g=0; g<params->granPerFrame; g++ ) {
    for( c=0; c<params->ncues+1; c++ ) {
      int ioChanActive;
      ioChanActive = getbits( strm, 1 );
      if(ioChanActive == -1)
        return(BCC_CUES);
      state->ioChanActive[g][c] =  ioChanActive;
    }
  }


  /* get independency flag */
  indep = getbits( strm, 1 );
  if(indep == -1)
    return(BCC_CUES);

  /* get coding scheme */
  all_linear_coding = 1;
  for( g=0; g<params->granPerFrame-1; g+=2 ) {
    for( c=0; c<params->ncues; c++ ) {
      linear_coding[g/2][c] = getbits( strm, 1 );
      if(linear_coding[g/2][c] == -1)
         return(BCC_CUES);
      all_linear_coding &= linear_coding[g/2][c];
    }
  }


  /* get direction of time difference coding */
  if( !all_linear_coding ) {
    direction = getbits( strm, 1 );
    if(direction == -1)
      return(BCC_CUES);

  }

  for( g=0; g<params->granPerFrame-1; g+=2 ) {

    for( c=0; c<params->ncues; c++ ) {

      if( linear_coding[g/2][c] ) {
        if(!pcm_decode( strm,
                    state->quant_ild[g  ][c],
                    state->quant_ild[g+1][c],
                    npart[c],
                    cue_levels ))
           return(BCC_CUES);

      }
      else {

        diff_type[g  ][c] = ((direction == BACKWARDS) && (g == 0) && indep            ) ? DIFF_FREQ : getbits( strm, 1 );
        if(diff_type[g][c] == -1)
          return(BCC_CUES);

        diff_type[g+1][c] = ((direction == FORWARDS ) && (g == params->granPerFrame-2)) ? DIFF_FREQ : getbits( strm, 1 );
        if(diff_type[g+1][c] == -1)
          return(BCC_CUES);


        if(!huff_decode( strm,
                     state->ild_diff[g  ][c],
                     state->ild_diff[g+1][c],
                     diff_type[g  ][c],
                     diff_type[g+1][c],
                     npart[c] ))
          return(BCC_CUES);
      }
    }
  }


  if( direction == BACKWARDS ) {

    for( g=0; g<params->granPerFrame; g++ ) {
      for( c=0; c<params->ncues; c++ ) {
        if( !linear_coding[g/2][c] ) {
          switch( diff_type[g][c] ) {
          case DIFF_FREQ:
            diff_freq_decode( (cue_levels-1)/2, /* start value */
                              state->ild_diff[g][c],
                              state->quant_ild[g][c],
                              npart[c] );
            break;
          case DIFF_TIME:
            prev_ild = (g == 0) ? &state->quant_ild_hist : &(state->quant_ild)[g-1];
            diff_time_decode_backwards( (*prev_ild)[c], /* previous values */
                                        state->ild_diff[g][c],
                                        state->quant_ild[g][c],
                                        npart[c] );
            break;
          default:
            break;

          }
        }
      }
    }

  } else {

    for( g=params->granPerFrame; g>0; g-- ) {
      for( c=0; c<params->ncues; c++ ) {
        if( !linear_coding[(g-1)/2][c] ) {
          switch( diff_type[g-1][c] ) {
          case DIFF_FREQ:
            diff_freq_decode( (cue_levels-1)/2, /* start value */
                              state->ild_diff[g-1][c],
                              state->quant_ild[g-1][c],
                              npart[c] );
            break;
          case DIFF_TIME:
            diff_time_decode_forwards( state->quant_ild[g][c], /* previous values */
                                       state->ild_diff[g-1][c],
                                       state->quant_ild[g-1][c],
                                       npart[c] );
            break;
          default:
            break;
          }
        }
      }
    }

  }


  /* history update */
  for( c=0; c<params->ncues; c++ ) {
    memcpy( state->quant_ild_hist[c], state->quant_ild[params->granPerFrame-1][c], sizeof(int)*npart[c] );
  }


  /* LSB data */
  for( g=0; g<params->granPerFrame; g++ ) {

    num_gran_lsb = 0;
    for( c=0; c<params->ncues; c++ ) {
      num_lsb[c] = get_num_lsb( state->quant_ild[g][c],
                                (cue_levels-1)/2,
                                npart[c] );
      num_gran_lsb += num_lsb[c];
    }

    lsb_trans_flag = (num_gran_lsb == 0) ? 0 : getbits( strm, 1 );
    if(lsb_trans_flag == -1)
       return(BCC_CUES);


    for( c=0; c<params->ncues; c++ ) {
      lsb_bits = lsb_trans_flag ? getbits( strm, num_lsb[c] ) : 0;
      if(lsb_bits == -1)
        return(BCC_CUES);

      attach_lsb( state->quant_ild[g][c],
                  lsb_bits,
                  num_lsb[c],
                  (params->cuelevels-1)/2,
                  npart[c] );
    }

  } /* loop over granules */


  /* Check whether decoded data is valid */
  invalid = 0;

  for( g=0; g<params->granPerFrame; g++ ) {
    for( c=0; c<params->ncues; c++ ) {
      for( p=0; p<npart[c]; p++ ) {
        if( (state->quant_ild[g][c][p] < 0) || (state->quant_ild[g][c][p] >= params->cuelevels) ) {
          invalid = 1;
          break;
        }
      }
      if( invalid ) break;
    }
    if( invalid ) break;
  }

  if( invalid ) return(BCC_CUES);


  // if(!bytealign(strm))
  //   return(BCC_BYTE_ALIGN);

  return(BCC_OK);
}

/*NEW*/
BCC_STAT bcc_info_dec( Streamin_state* strm,
                       BCC_dparams*    params,
                       int             num_info_bits )
{
  int sxUpmixFlag=0, i=0;

  /* sxUpmixFlag */
  sxUpmixFlag = getbits( strm, 1 );

  if(sxUpmixFlag == -1) {
    return(BCC_INFO);
  }
  else {
    params->infoChunk.sxFlag = sxUpmixFlag;
  }

  /* reserved bits */
  for( i=1; i<num_info_bits; i++ ) {
    if( getbits(strm, 1) == -1 ) {
      return(BCC_INFO);
    }
  }

  return(BCC_OK);
}
