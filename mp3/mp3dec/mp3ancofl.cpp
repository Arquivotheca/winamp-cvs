/****************************************************************\
*
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
*                        All Rights Reserved
*
*   filename: mp3ancofl.cpp
*   project : MPEG Decoder
*   author  : Dieter Weninger
*   date    : 2003-05-14
*   contents: ancillary data and original file length
*
\****************************************************************/
#include "mp3ancofl.h"

//--------------------------------------------------------------*
//  function: constructor
//  info: only version 1 supported
//  return: -
//--------------------------------------------------------------*
CMp3AncOfl::CMp3AncOfl(CBitStream &__Db) : m_Db(__Db)
{
  Reset();
}

//--------------------------------------------------------------*
//  function: Reset
//  info: reset members
//  return: -
//--------------------------------------------------------------*
void CMp3AncOfl::Reset(void)
{
  int i;

  m_valid     = false;
  m_searched  = false;
  m_semaphor  = true;
  m_FhGAncChecked = false;
  m_collecting = false;
  m_vbrChecked = false;
  m_mp3pro = false;

  m_FhGAncBuf = 0;
  m_tmpAncBuf = 0;

  m_pFhGAncBuf = 0;
  m_FhGAncBufSize = 0;

  for(i=0; i < VERSION_1_LEN; i++) {
    oflArray[i]=0;
  }

  // default: read bytewise
  m_readBytes = 1;
}

//--------------------------------------------------------------*
//  function: destructor
//  info: -
//  return: -
//--------------------------------------------------------------*
CMp3AncOfl::~CMp3AncOfl()
{
  cleanUp();
}

//--------------------------------------------------------------*
//  function: isOfl
//  info:     reads sync and checks crc
//  return:   true if ofl found
//--------------------------------------------------------------*
bool CMp3AncOfl::isOfl(void)
{
  unsigned long crcReg = 0x000000ff;
  int i=0;

  if (oflArray[0] == 0xB0) {
    for( i=0; i < (VERSION_0_LEN - 1); i++) {
      crcOfl(0x0045, 0x0080, &crcReg, oflArray[i]);
    }
    if(oflArray[7] == (crcReg & 0x000000ff)) {
      // not supported, only for mp3PRO
      m_mp3pro = true;
      m_valid = false;
      return true;
    }
  }
  else if(oflArray[0] == 0xB4) {
    for( i=0; i < (VERSION_1_LEN - 1); i++) {
      crcOfl(0x0045, 0x0080, &crcReg, oflArray[i]);
    }
    if(oflArray[9] == (crcReg & 0x000000ff)) {
      // ok
      return (m_valid=true);
    }
  }
  return (m_valid=false);
}

//--------------------------------------------------------------*
//  function: getVersion
//  info: -
//  return: the version of ofl, or -1 on error
//--------------------------------------------------------------*
int CMp3AncOfl::getVersion(void)
{
  if ( validOfl() || m_mp3pro )
    return ( (oflArray[0]>>2) & 0x3 );
  else
    return -1;
}

//--------------------------------------------------------------*
//  function: getTotalLength
//  info: -
//  return: original length or 0
//--------------------------------------------------------------*
unsigned CMp3AncOfl::getTotalLength(void)
{
  unsigned tmp = 0;

  if ( validOfl() ) {
    tmp =  ((unsigned)oflArray[3]) << 24;
    tmp += ((unsigned)oflArray[4]) << 16;
    tmp += ((unsigned)oflArray[5]) << 8;
    tmp += ((unsigned)oflArray[6]);
  }
  return tmp;
}

//--------------------------------------------------------------*
//  function: getCodecDelay
//  info: -
//  return: codec delay or 0
//--------------------------------------------------------------*
unsigned CMp3AncOfl::getCodecDelay(void)
{
  unsigned tmp = 0;

  if ( validOfl() ) {
    tmp =  ((unsigned)oflArray[1]) << 8;
    tmp += ((unsigned)oflArray[2]);
  }
  return tmp;
}

//--------------------------------------------------------------*
//  function: getAddDelay
//  info: only with version 1 available, reserved for mp3PRO
//  return: additional codec delay or 0
//--------------------------------------------------------------*
unsigned CMp3AncOfl::getAddDelay(void)
{
  unsigned tmp = 0;

  if ( validOfl() ) {
    tmp = ((unsigned)oflArray[7]) << 8;
    tmp += ((unsigned)oflArray[8]);
  }
  return tmp;
}

//--------------------------------------------------------------*
//  function: justSearched
//  info: avoid second search, unless VBRI
//  return: true if ofl already searched
//--------------------------------------------------------------*
bool CMp3AncOfl::justSearched(void)
{
  return m_searched;
}

//--------------------------------------------------------------*
//  function: validOfl
//  info: -
//  return: true if ofl is valid (version 1)
//--------------------------------------------------------------*
bool CMp3AncOfl::validOfl(void)
{
  return m_valid;
}

//--------------------------------------------------------------*
//  function: toSkip
//  info: skip ofl only one time from ancillary data
//  return: ancillary bytes to skip
//--------------------------------------------------------------*
int CMp3AncOfl::toSkip(void)
{
  if ( validOfl() || m_mp3pro ) {
    if( m_semaphor ) {
      m_semaphor = false;
      return ((getVersion() == 1) ? VERSION_1_LEN : VERSION_0_LEN);
    }
  }
  return 0;
}

//--------------------------------------------------------------*
//  function: cleanUp
//  info: clean buffers
//  return: -
//--------------------------------------------------------------*
void CMp3AncOfl::cleanUp(void)
{
  if ( m_tmpAncBuf ) {
    delete[] m_tmpAncBuf;
    m_tmpAncBuf = 0;
  }
  if ( m_FhGAncBuf ) {
    delete[] m_FhGAncBuf;
    m_FhGAncBuf = 0;
    m_FhGAncBufSize = 0;
  }
}

//--------------------------------------------------------------*
//  function: getOfl
//  info: search ofl after main data in first frame;
//        if VBRI or Xing is found then search in second frame
//  return: -
//--------------------------------------------------------------*
void CMp3AncOfl::getOfl(CBitStream &Db,
                        const int len)
{
  int nBytesRead = 0;
  unsigned char byteRead;
  int i;

  m_searched = true;

 findOFL:

  while( nBytesRead < len ) {
    byteRead = (unsigned char) Db.GetBits(8);
    nBytesRead++;
    if( (byteRead == 0xB0) || (byteRead == 0xB4) ) {
      oflArray[0] = byteRead;
      break;
    }
    else if( (char)byteRead == 'V' ) {
      if ( (unsigned char) Db.GetBits(8) == 'B' ) {
        if ( (unsigned char) Db.GetBits(8) == 'R' ) {
          if ( (unsigned char) Db.GetBits(8) == 'I' ) {
            // VBRI
            m_searched = false;
            return;
          }
          else {
            Db.Rewind(24);
          }
        }
        else {
          Db.Rewind(16);
        }
      }
      else {
        Db.Rewind(8);
      }
    }
    else  if( (char)byteRead == 'X' ) {
      if ( (unsigned char) Db.GetBits(8) == 'i' ) {
        if ( (unsigned char) Db.GetBits(8) == 'n' ) {
          if ( (unsigned char) Db.GetBits(8) == 'g' ) {
            // Xing
            m_searched = false;
            return;
          }
          else {
            Db.Rewind(24);
          }
        }
        else {
          Db.Rewind(16);
        }
      }
      else {
        Db.Rewind(8);
      }
    }
  }

  // ofl found ?
  if( nBytesRead < len ) {
    for( i=1; i < VERSION_1_LEN; i++ ) {
      oflArray[i] = Db.GetBits(8);
    }

    if ( !isOfl() ) {
      // not found
      Db.Rewind( (VERSION_1_LEN - 1) * 8);
      goto findOFL;
    }
  }

  return;
}

//----------------------------------------------------------------*
//  function: readOFL
//  info: be sure ancillary data is byte aligned;
//        note bitCnt from dynamic buffer
//  return: true if ofl is valid
//----------------------------------------------------------------*
bool CMp3AncOfl::readOfl(CBitStream &Db,
                         int beforeScf)
{
  int bitsLeftInBuffer = Db.GetValidBits();
  int bytesLeftInBuffer = bitsLeftInBuffer >> 3 ;
  int bitsConsumed = beforeScf - bitsLeftInBuffer ;
  int initCnt = Db.GetBitCnt();

  if (bytesLeftInBuffer) {
    // byte align
    int skip = (bitsConsumed & 0x07) ? 8 - (bitsConsumed & 0x07) : 0 ;
    Db.GetBits(skip);
  }

  getOfl( Db, bytesLeftInBuffer );
  Db.Rewind( Db.GetBitCnt() - initCnt );

  return validOfl();
}

//----------------------------------------------------------------*
//  function: crcOfl
//  info: for ofl we need 8bit crc with poly 0x0045
//  return: -
//----------------------------------------------------------------*
void CMp3AncOfl::crcOfl(unsigned short crcPoly,
                        unsigned short crcMask,
                        unsigned long *crc,
                        unsigned char byte)
{
  int i;
  for (i=0; i<8; i++) {
    unsigned short flag = (*crc) & crcMask ? 1:0;
    flag ^= (byte & 0x80 ? 1 : 0);
    (*crc)<<=1;
    byte <<= 1;
    if(flag)
      (*crc) ^= crcPoly;
  }
}

//----------------------------------------------------------------*
//  function: readAnc
//  info: consider ofl size if ancillary and ofl are wanted,
//        if Xing or VBRI no ancillary data available in this frame
//  return: number of ancillary bytes read
//----------------------------------------------------------------*
int CMp3AncOfl::readAnc(unsigned char *ancBytes,
                        CBitStream      &Db,
                        const int numAncBits)
{
  int cnt=0;
  int numAncBytes = (int)(numAncBits/8);
  int skip = toSkip();
  int ancSize = numAncBytes - skip;

  if( (ancBytes != 0) && (ancSize > 0) ) {

    int i=0;

    if ( m_readBytes ) {
      Db.Rewind( ancSize * 8 );
    }
    else {
      Db.Rewind( numAncBits );
      if ( numAncBits%8 )
        ancSize++;
    }

    m_tmpAncBuf = new unsigned char[ancSize];

    for (i=0; i < ancSize; i++ ) {
      m_tmpAncBuf[i] = (unsigned char)Db.GetBits(8);
    }

    if ( !m_readBytes && numAncBits%8 ) {
      int bits = numAncBits-8*(ancSize-1);
      m_tmpAncBuf[ancSize-1] = m_tmpAncBuf[ancSize-1] << (8-bits);
      Db.Rewind( 8-bits );
    }

    if ( !m_vbrChecked ) {
      m_vbrChecked = true;
      if ( ancSize >= 4) {
        for(i=0; i <= (ancSize-4); i++) {
          if((  m_tmpAncBuf[i] == 'V' &&
              m_tmpAncBuf[i+1] == 'B' &&
              m_tmpAncBuf[i+2] == 'R' &&
              m_tmpAncBuf[i+3] == 'I' ) ||
             (   m_tmpAncBuf[i] == 'X' &&
              m_tmpAncBuf[i+1]  == 'i' &&
              m_tmpAncBuf[i+2]  == 'n' &&
              m_tmpAncBuf[i+3]  == 'g' ) ) {
            // VBRI or Xing
            cleanUp();
            return 0;
          }
        }
      }
    }

    if( !m_FhGAncChecked && !m_collecting) {
      isFhGAnc(ancSize);
    }

    if (!m_collecting) {
      // raw
      for (cnt=0; cnt < ancSize; cnt++){
        ancBytes[cnt] = m_tmpAncBuf[cnt];
      }
      ancBytes[cnt]='\0';
    }
    else {
      for (i=0; (i < ancSize) && (m_pFhGAncBuf < m_FhGAncBufSize);
           m_pFhGAncBuf++, i++) {
        m_FhGAncBuf[m_pFhGAncBuf] = m_tmpAncBuf[i];
      }

      m_collecting = false;
      m_FhGAncChecked = false;

      // calc checksum
      unsigned long checksum = 0;
      for(i=0; i < (m_FhGAncBufSize-1); i++) {
        checksum += m_FhGAncBuf[i];
      }

      if( m_FhGAncBuf[m_FhGAncBufSize-1] != (0x000000ff & checksum) ) {
        // treat as raw
        for (cnt=0; cnt < m_FhGAncBufSize; cnt++){
          ancBytes[cnt] = m_FhGAncBuf[cnt];
        }
      }
      else {
        // FhG ancillary
        for (cnt=0, i=2; i < (m_FhGAncBufSize-1); cnt++, i++) {
          ancBytes[cnt] = m_FhGAncBuf[i];
        }
      }

      m_pFhGAncBuf = 0;
    }
    cleanUp();
  }
  return cnt;
}

//----------------------------------------------------------------*
//  function: isFhGAnc
//  info: number of fhg ancillary bytes should min be 4
//        SYNC | LEN | DATA | CHECKSUM
//	  SYNC = 0xA5
//	  LEN = total length including SYNC LEN and CHECKSUM in byte
//	  DATA = ancillary data (LEN-3 bytes)
//	  CHECKSUM = 8bit checksum
//  return: true if FhG ancillary mode
//----------------------------------------------------------------*
bool CMp3AncOfl::isFhGAnc(int nAncBytes)
{
  m_FhGAncChecked = true;

  if (nAncBytes >= 4) {
    if ( m_tmpAncBuf[0] == 0xA5 ) {
      // FhG ancillary
      m_FhGAncBufSize = (int)m_tmpAncBuf[1];
      m_FhGAncBuf = new unsigned char[ (int)m_tmpAncBuf[1] ];
      m_collecting = true;
      return true;
    }
  }

  return false; // raw ancillary
}

//----------------------------------------------------------------*
//  function: fetchOfl
//  info: if ofl is claimed search it, but deliver it only if ofl
//        volitional
//  return: -
//----------------------------------------------------------------*
void CMp3AncOfl::fetchOfl(int oflOn,
                          CBitStream &Db,
                          int beforeScf,
                          unsigned int* startDelay,
                          unsigned int* totalLength)
{
  if( oflOn == 1 ) {
    if( !justSearched() ) {
      if( readOfl(Db, beforeScf) ) {
        if( (totalLength != 0) && (startDelay != 0) ) {
          // ofl ok
          *totalLength = getTotalLength();
          *startDelay = getCodecDelay();
        }
      }
      else {
        // no ofl or error
        if( (totalLength != 0) && (startDelay != 0) ) {
          *totalLength = 0;
          *startDelay = 0;
        }
      }
    }
  }
}
