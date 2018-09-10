/*
  LzmaDecode.c
  LZMA Decoder (optimized for Speed version)

  LZMA SDK 4.22 Copyright (c) 1999-2005 Igor Pavlov (2005-06-10)
  http://www.7-zip.org/

  LZMA SDK is licensed under two licenses:
  1) GNU Lesser General Public License (GNU LGPL)
  2) Common Public License (CPL)
  It means that you can select one of these two licenses and
  follow rules of that license.

  SPECIAL EXCEPTION:
  Igor Pavlov, as the author of this Code, expressly permits you to
  statically or dynamically link your Code (or bind by name) to the
  interfaces of this file without subjecting your linked Code to the
  terms of the CPL or GNU LGPL. Any modifications or additions
  to this file, however, are subject to the LGPL or CPL terms.
*/

#include "LzmaDecode.h"

#ifndef Byte
#define Byte unsigned char
#endif

#define kNumTopBits 24
#define kTopValue ((UInt32)1 << kNumTopBits)

#define kNumBitModelTotalBits 11
#define kBitModelTotal (1 << kNumBitModelTotalBits)
#define kNumMoveBits 5

#define RC_READ_BYTE (*Buffer++)

#define RC_INIT2 Code = 0; Range = 0xFFFFFFFF; \
  { int i; for(i = 0; i < 5; i++) { RC_TEST; Code = (Code << 8) | RC_READ_BYTE; }}

#ifdef _LZMA_IN_CB

#define RC_TEST { if (Buffer == BufferLim) \
  { SizeT size; int result = InCallback->Read(InCallback, &Buffer, &size); if (result != LZMA_RESULT_OK) return result; \
  BufferLim = Buffer + size; if (size == 0) return LZMA_RESULT_DATA_ERROR; }}

#define RC_INIT Buffer = BufferLim = 0; RC_INIT2

#else

#define RC_TEST { if (Buffer == BufferLim) return LZMA_RESULT_DATA_ERROR; }

#define RC_INIT(buffer, bufferSize) Buffer = buffer; BufferLim = buffer + bufferSize; RC_INIT2

#endif

#define RC_NORMALIZE if (Range < kTopValue) { RC_TEST; Range <<= 8; Code = (Code << 8) | RC_READ_BYTE; }

#define IfBit0(p) RC_NORMALIZE; bound = (Range >> kNumBitModelTotalBits) * *(p); if (Code < bound)
#define UpdateBit0(p) Range = bound; *(p) += (kBitModelTotal - *(p)) >> kNumMoveBits;
#define UpdateBit1(p) Range -= bound; Code -= bound; *(p) -= (*(p)) >> kNumMoveBits;

#define RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } else \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; }

#define RC_GET_BIT(p, mi) RC_GET_BIT2(p, mi, ; , ;)

#define RangeDecoderBitTreeDecode(probs, numLevels, res) \
  { int i = numLevels; res = 1; \
  do { CProb *p = probs + res; RC_GET_BIT(p, res) } while(--i != 0); \
  res -= (1 << numLevels); }


#define kNumPosBitsMax 4
#define kNumPosStatesMax (1 << kNumPosBitsMax)

#define kLenNumLowBits 3
#define kLenNumLowSymbols (1 << kLenNumLowBits)
#define kLenNumMidBits 3
#define kLenNumMidSymbols (1 << kLenNumMidBits)
#define kLenNumHighBits 8
#define kLenNumHighSymbols (1 << kLenNumHighBits)

#define LenChoice 0
#define LenChoice2 (LenChoice + 1)
#define LenLow (LenChoice2 + 1)
#define LenMid (LenLow + (kNumPosStatesMax << kLenNumLowBits))
#define LenHigh (LenMid + (kNumPosStatesMax << kLenNumMidBits))
#define kNumLenProbs (LenHigh + kLenNumHighSymbols)


#define kNumStates 12
#define kNumLitStates 7

#define kStartPosModelIndex 4
#define kEndPosModelIndex 14
#define kNumFullDistances (1 << (kEndPosModelIndex >> 1))

#define kNumPosSlotBits 6
#define kNumLenToPosStates 4

#define kNumAlignBits 4
#define kAlignTableSize (1 << kNumAlignBits)

#define kMatchMinLen 2

#define IsMatch 0
#define IsRep (IsMatch + (kNumStates << kNumPosBitsMax))
#define IsRepG0 (IsRep + kNumStates)
#define IsRepG1 (IsRepG0 + kNumStates)
#define IsRepG2 (IsRepG1 + kNumStates)
#define IsRep0Long (IsRepG2 + kNumStates)
#define PosSlot (IsRep0Long + (kNumStates << kNumPosBitsMax))
#define SpecPos (PosSlot + (kNumLenToPosStates << kNumPosSlotBits))
#define Align (SpecPos + kNumFullDistances - kEndPosModelIndex)
#define LenCoder (Align + kAlignTableSize)
#define RepLenCoder (LenCoder + kNumLenProbs)
#define Literal (RepLenCoder + kNumLenProbs)

#if Literal != LZMA_BASE_SIZE
StopCompilingDueBUG
#endif

#if 0
int LzmaDecodeProperties(CLzmaProperties *propsRes, const unsigned char *propsData, int size)
{
  unsigned char prop0;
  if (size < LZMA_PROPERTIES_SIZE)
    return LZMA_RESULT_DATA_ERROR;
  prop0 = propsData[0];
  if (prop0 >= (9 * 5 * 5))
    return LZMA_RESULT_DATA_ERROR;
  {
    for (propsRes->pb = 0; prop0 >= (9 * 5); propsRes->pb++, prop0 -= (9 * 5));
    for (propsRes->lp = 0; prop0 >= 9; propsRes->lp++, prop0 -= 9);
    propsRes->lc = prop0;
    /*
    unsigned char remainder = (unsigned char)(prop0 / 9);
    propsRes->lc = prop0 % 9;
    propsRes->pb = remainder / 5;
    propsRes->lp = remainder % 5;
    */
  }

  #ifdef _LZMA_OUT_READ
  {
    int i;
    propsRes->DictionarySize = 0;
    for (i = 0; i < 4; i++)
      propsRes->DictionarySize += (UInt32)(propsData[1 + i]) << (i * 8);
    if (propsRes->DictionarySize == 0)
      propsRes->DictionarySize = 1;
  }
  #endif
  return LZMA_RESULT_OK;
}
#endif

#define kLzmaStreamWasFinishedId (-1)

int LzmaDecode(CLzmaDecoderState *vs,
    #ifdef _LZMA_IN_CB
    ILzmaInCallback *InCallback,
    #else
    const unsigned char *inStream, SizeT inSize, SizeT *inSizeProcessed,
    #endif
    unsigned char *outStream, SizeT outSize, SizeT *outSizeProcessed)
{
  CProb *p = vs->Probs;
  SizeT nowPos = 0;
  Byte previousByte = 0;
  UInt32 posStateMask = (1 << (vs->Properties.pb)) - 1;
  UInt32 literalPosMask = (1 << (vs->Properties.lp)) - 1;
  int lc = vs->Properties.lc;

  #ifdef _LZMA_OUT_READ

  UInt32 Range = vs->Range;
  UInt32 Code = vs->Code;
  #ifdef _LZMA_IN_CB
  const Byte *Buffer = vs->Buffer;
  const Byte *BufferLim = vs->BufferLim;
  #else
  const Byte *Buffer = inStream;
  const Byte *BufferLim = inStream + inSize;
  #endif
  int state = vs->State;
  UInt32 rep0 = vs->Reps[0], rep1 = vs->Reps[1], rep2 = vs->Reps[2], rep3 = vs->Reps[3];
  int len = vs->RemainLen;
  UInt32 globalPos = vs->GlobalPos;
  UInt32 distanceLimit = vs->DistanceLimit;

  Byte *dictionary = vs->Dictionary;
  UInt32 dictionarySize = vs->Properties.DictionarySize;
  UInt32 dictionaryPos = vs->DictionaryPos;

  Byte tempDictionary[4];

  #ifndef _LZMA_IN_CB
  *inSizeProcessed = 0;
  #endif
  *outSizeProcessed = 0;
  if (len == kLzmaStreamWasFinishedId)
    return LZMA_RESULT_OK;

  if (dictionarySize == 0)
  {
    dictionary = tempDictionary;
    dictionarySize = 1;
    tempDictionary[0] = vs->TempDictionary[0];
  }

  if (len == kLzmaNeedInitId)
  {
    {
      UInt32 numProbs = Literal + ((UInt32)LZMA_LIT_SIZE << (lc + vs->Properties.lp));
      UInt32 i;
      for (i = 0; i < numProbs; i++)
        p[i] = kBitModelTotal >> 1;
      rep0 = rep1 = rep2 = rep3 = 1;
      state = 0;
      globalPos = 0;
      distanceLimit = 0;
      dictionaryPos = 0;
      dictionary[dictionarySize - 1] = 0;
      #ifdef _LZMA_IN_CB
      RC_INIT;
      #else
      RC_INIT(inStream, inSize);
      #endif
    }
    len = 0;
  }
  while(len != 0 && nowPos < outSize)
  {
    UInt32 pos = dictionaryPos - rep0;
    if (pos >= dictionarySize)
      pos += dictionarySize;
    outStream[nowPos++] = dictionary[dictionaryPos] = dictionary[pos];
    if (++dictionaryPos == dictionarySize)
      dictionaryPos = 0;
    len--;
  }
  if (dictionaryPos == 0)
    previousByte = dictionary[dictionarySize - 1];
  else
    previousByte = dictionary[dictionaryPos - 1];

  #else /* if !_LZMA_OUT_READ */

  int state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  int len = 0;
  const Byte *Buffer;
  const Byte *BufferLim;
  UInt32 Range;
  UInt32 Code;

  #ifndef _LZMA_IN_CB
  *inSizeProcessed = 0;
  #endif
  *outSizeProcessed = 0;

  {
    UInt32 i;
    UInt32 numProbs = Literal + ((UInt32)LZMA_LIT_SIZE << (lc + vs->Properties.lp));
    for (i = 0; i < numProbs; i++)
      p[i] = kBitModelTotal >> 1;
  }

  #ifdef _LZMA_IN_CB
  RC_INIT;
  #else
  RC_INIT(inStream, inSize);
  #endif

  #endif /* _LZMA_OUT_READ */

  while(nowPos < outSize)
  {
    CProb *prob;
    UInt32 bound;
    int posState = (int)(
        (nowPos
        #ifdef _LZMA_OUT_READ
        + globalPos
        #endif
        )
        & posStateMask);

    prob = p + IsMatch + (state << kNumPosBitsMax) + posState;
    IfBit0(prob)
    {
      int symbol = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_SIZE *
        (((
        (nowPos
        #ifdef _LZMA_OUT_READ
        + globalPos
        #endif
        )
        & literalPosMask) << lc) + (previousByte >> (8 - lc))));

      if (state >= kNumLitStates)
      {
        int matchByte;
        #ifdef _LZMA_OUT_READ
        UInt32 pos = dictionaryPos - rep0;
        if (pos >= dictionarySize)
          pos += dictionarySize;
        matchByte = dictionary[pos];
        #else
        matchByte = outStream[nowPos - rep0];
        #endif
        do
        {
          int bit;
          CProb *probLit;
          matchByte <<= 1;
          bit = (matchByte & 0x100);
          probLit = prob + 0x100 + bit + symbol;
          RC_GET_BIT2(probLit, symbol, if (bit != 0) break, if (bit == 0) break)
        }
        while (symbol < 0x100);
      }
      while (symbol < 0x100)
      {
        CProb *probLit = prob + symbol;
        RC_GET_BIT(probLit, symbol)
      }
      previousByte = (Byte)symbol;

      outStream[nowPos++] = previousByte;
      #ifdef _LZMA_OUT_READ
      if (distanceLimit < dictionarySize)
        distanceLimit++;

      dictionary[dictionaryPos] = previousByte;
      if (++dictionaryPos == dictionarySize)
        dictionaryPos = 0;
      #endif
      if (state < 4) state = 0;
      else if (state < 10) state -= 3;
      else state -= 6;
    }
    else
    {
      UpdateBit1(prob);
      prob = p + IsRep + state;
      IfBit0(prob)
      {
        UpdateBit0(prob);
        rep3 = rep2;
        rep2 = rep1;
        rep1 = rep0;
        state = state < kNumLitStates ? 0 : 3;
        prob = p + LenCoder;
      }
      else
      {
        UpdateBit1(prob);
        prob = p + IsRepG0 + state;
        IfBit0(prob)
        {
          UpdateBit0(prob);
          prob = p + IsRep0Long + (state << kNumPosBitsMax) + posState;
          IfBit0(prob)
          {
            #ifdef _LZMA_OUT_READ
            UInt32 pos;
            #endif
            UpdateBit0(prob);

            #ifdef _LZMA_OUT_READ
            if (distanceLimit == 0)
            #else
            if (nowPos == 0)
            #endif
              return LZMA_RESULT_DATA_ERROR;

            state = state < kNumLitStates ? 9 : 11;
            #ifdef _LZMA_OUT_READ
            pos = dictionaryPos - rep0;
            if (pos >= dictionarySize)
              pos += dictionarySize;
            previousByte = dictionary[pos];
            dictionary[dictionaryPos] = previousByte;
            if (++dictionaryPos == dictionarySize)
              dictionaryPos = 0;
            #else
            previousByte = outStream[nowPos - rep0];
            #endif
            outStream[nowPos++] = previousByte;
            #ifdef _LZMA_OUT_READ
            if (distanceLimit < dictionarySize)
              distanceLimit++;
            #endif

            continue;
          }
          else
          {
            UpdateBit1(prob);
          }
        }
        else
        {
          UInt32 distance;
          UpdateBit1(prob);
          prob = p + IsRepG1 + state;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            distance = rep1;
          }
          else
          {
            UpdateBit1(prob);
            prob = p + IsRepG2 + state;
            IfBit0(prob)
            {
              UpdateBit0(prob);
              distance = rep2;
            }
            else
            {
              UpdateBit1(prob);
              distance = rep3;
              rep3 = rep2;
            }
            rep2 = rep1;
          }
          rep1 = rep0;
          rep0 = distance;
        }
        state = state < kNumLitStates ? 8 : 11;
        prob = p + RepLenCoder;
      }
      {
        int numBits, offset;
        CProb *probLen = prob + LenChoice;
        IfBit0(probLen)
        {
          UpdateBit0(probLen);
          probLen = prob + LenLow + (posState << kLenNumLowBits);
          offset = 0;
          numBits = kLenNumLowBits;
        }
        else
        {
          UpdateBit1(probLen);
          probLen = prob + LenChoice2;
          IfBit0(probLen)
          {
            UpdateBit0(probLen);
            probLen = prob + LenMid + (posState << kLenNumMidBits);
            offset = kLenNumLowSymbols;
            numBits = kLenNumMidBits;
          }
          else
          {
            UpdateBit1(probLen);
            probLen = prob + LenHigh;
            offset = kLenNumLowSymbols + kLenNumMidSymbols;
            numBits = kLenNumHighBits;
          }
        }
        RangeDecoderBitTreeDecode(probLen, numBits, len);
        len += offset;
      }

      if (state < 4)
      {
        int posSlot;
        state += kNumLitStates;
        prob = p + PosSlot +
            ((len < kNumLenToPosStates ? len : kNumLenToPosStates - 1) <<
            kNumPosSlotBits);
        RangeDecoderBitTreeDecode(prob, kNumPosSlotBits, posSlot);
        if (posSlot >= kStartPosModelIndex)
        {
          int numDirectBits = ((posSlot >> 1) - 1);
          rep0 = (2 | ((UInt32)posSlot & 1));
          if (posSlot < kEndPosModelIndex)
          {
            rep0 <<= numDirectBits;
            prob = p + SpecPos + rep0 - posSlot - 1;
          }
          else
          {
            numDirectBits -= kNumAlignBits;
            do
            {
              RC_NORMALIZE
              Range >>= 1;
              rep0 <<= 1;
              if (Code >= Range)
              {
                Code -= Range;
                rep0 |= 1;
              }
            }
            while (--numDirectBits != 0);
            prob = p + Align;
            rep0 <<= kNumAlignBits;
            numDirectBits = kNumAlignBits;
          }
          {
            int i = 1;
            int mi = 1;
            do
            {
              CProb *prob3 = prob + mi;
              RC_GET_BIT2(prob3, mi, ; , rep0 |= i);
              i <<= 1;
            }
            while(--numDirectBits != 0);
          }
        }
        else
          rep0 = posSlot;
        if (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = kLzmaStreamWasFinishedId;
          break;
        }
      }

      len += kMatchMinLen;
      #ifdef _LZMA_OUT_READ
      if (rep0 > distanceLimit)
      #else
      if (rep0 > nowPos)
      #endif
        return LZMA_RESULT_DATA_ERROR;

      #ifdef _LZMA_OUT_READ
      if (dictionarySize - distanceLimit > (UInt32)len)
        distanceLimit += len;
      else
        distanceLimit = dictionarySize;
      #endif

      do
      {
        #ifdef _LZMA_OUT_READ
        UInt32 pos = dictionaryPos - rep0;
        if (pos >= dictionarySize)
          pos += dictionarySize;
        previousByte = dictionary[pos];
        dictionary[dictionaryPos] = previousByte;
        if (++dictionaryPos == dictionarySize)
          dictionaryPos = 0;
        #else
        previousByte = outStream[nowPos - rep0];
        #endif
        len--;
        outStream[nowPos++] = previousByte;
      }
      while(len != 0 && nowPos < outSize);
    }
  }
  RC_NORMALIZE;

  #ifdef _LZMA_OUT_READ
  vs->Range = Range;
  vs->Code = Code;
  vs->DictionaryPos = dictionaryPos;
  vs->GlobalPos = globalPos + (UInt32)nowPos;
  vs->DistanceLimit = distanceLimit;
  vs->Reps[0] = rep0;
  vs->Reps[1] = rep1;
  vs->Reps[2] = rep2;
  vs->Reps[3] = rep3;
  vs->State = state;
  vs->RemainLen = len;
  vs->TempDictionary[0] = tempDictionary[0];
  #endif

  #ifdef _LZMA_IN_CB
  vs->Buffer = Buffer;
  vs->BufferLim = BufferLim;
  #else
  *inSizeProcessed = (SizeT)(Buffer - inStream);
  #endif
  *outSizeProcessed = nowPos;
  return LZMA_RESULT_OK;
}
