/*
  LzmaDecode.c
  LZMA Decoder
  
  LZMA SDK 4.05 Copyright (c) 1999-2004 Igor Pavlov (2004-08-25)
  http://www.7-zip.org/

  LZMA SDK is licensed under two licenses:
  1) GNU Lesser General Public License (GNU LGPL)
  2) Common Public License (CPL)
  It means that you can select one of these two licenses and 
  follow rules of that license.

  SPECIAL EXCEPTION:
  Igor Pavlov, as the author of this code, expressly permits you to 
  statically or dynamically link your code (or bind by name) to the 
  interfaces of this file without subjecting your linked code to the 
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

typedef struct _CRangeDecoder
{
  Byte *Buffer;
  Byte *BufferLim;
  UInt32 Range;
  UInt32 Code;
  #ifdef _LZMA_IN_CB
  ILzmaInCallback *InCallback;
  int Result;
  #endif
  int ExtraBytes;
} CRangeDecoder;

Byte RangeDecoderReadByte(CRangeDecoder *rd)
{
  if (rd->Buffer == rd->BufferLim)
  {
    #ifdef _LZMA_IN_CB
    UInt32 size;
    rd->Result = rd->InCallback->Read(rd->InCallback, &rd->Buffer, &size);
    rd->BufferLim = rd->Buffer + size;
    if (size == 0)
    #endif
    {
      rd->ExtraBytes = 1;
      return 0xFF;
    }
  }
  return (*rd->Buffer++);
}

/* #define ReadByte (*rd->Buffer++) */
#define ReadByte (RangeDecoderReadByte(rd))

void RangeDecoderInit(CRangeDecoder *rd,
  #ifdef _LZMA_IN_CB
    ILzmaInCallback *inCallback
  #else
    Byte *stream, UInt32 bufferSize
  #endif
    )
{
  int i;
  #ifdef _LZMA_IN_CB
  rd->InCallback = inCallback;
  rd->Buffer = rd->BufferLim = 0;
  #else
  rd->Buffer = stream;
  rd->BufferLim = stream + bufferSize;
  #endif
  rd->ExtraBytes = 0;
  rd->Code = 0;
  rd->Range = (0xFFFFFFFF);
  for(i = 0; i < 5; i++)
    rd->Code = (rd->Code << 8) | ReadByte;
}

#define RC_INIT_VAR UInt32 range = rd->Range; UInt32 code = rd->Code;        
#define RC_FLUSH_VAR rd->Range = range; rd->Code = code;
#define RC_NORMALIZE if (range < kTopValue) { range <<= 8; code = (code << 8) | ReadByte; }

UInt32 RangeDecoderDecodeDirectBits(CRangeDecoder *rd, int numTotalBits)
{
  RC_INIT_VAR
  UInt32 result = 0;
  int i;
  for (i = numTotalBits; i > 0; i--)
  {
    /* UInt32 t; */
    range >>= 1;

    result <<= 1;
    if (code >= range)
    {
      code -= range;
      result |= 1;
    }
    /*
    t = (code - range) >> 31;
    t &= 1;
    code -= range & (t - 1);
    result = (result + result) | (1 - t);
    */
    RC_NORMALIZE
  }
  RC_FLUSH_VAR
  return result;
}

int RangeDecoderBitDecode(CProb *prob, CRangeDecoder *rd)
{
  UInt32 bound = (rd->Range >> kNumBitModelTotalBits) * *prob;
  if (rd->Code < bound)
  {
    rd->Range = bound;
    *prob += (kBitModelTotal - *prob) >> kNumMoveBits;
    if (rd->Range < kTopValue)
    {
      rd->Code = (rd->Code << 8) | ReadByte;
      rd->Range <<= 8;
    }
    return 0;
  }
  else
  {
    rd->Range -= bound;
    rd->Code -= bound;
    *prob -= (*prob) >> kNumMoveBits;
    if (rd->Range < kTopValue)
    {
      rd->Code = (rd->Code << 8) | ReadByte;
      rd->Range <<= 8;
    }
    return 1;
  }
}

#define RC_GET_BIT2(prob, mi, A0, A1) \
  UInt32 bound = (range >> kNumBitModelTotalBits) * *prob; \
  if (code < bound) \
    { A0; range = bound; *prob += (kBitModelTotal - *prob) >> kNumMoveBits; mi <<= 1; } \
  else \
    { A1; range -= bound; code -= bound; *prob -= (*prob) >> kNumMoveBits; mi = (mi + mi) + 1; } \
  RC_NORMALIZE

#define RC_GET_BIT(prob, mi) RC_GET_BIT2(prob, mi, ; , ;)               

int RangeDecoderBitTreeDecode(CProb *probs, int numLevels, CRangeDecoder *rd)
{
  int mi = 1;
  int i;
  #ifdef _LZMA_LOC_OPT
  RC_INIT_VAR
  #endif
  for(i = numLevels; i > 0; i--)
  {
    #ifdef _LZMA_LOC_OPT
    CProb *prob = probs + mi;
    RC_GET_BIT(prob, mi)
    #else
    mi = (mi + mi) + RangeDecoderBitDecode(probs + mi, rd);
    #endif
  }
  #ifdef _LZMA_LOC_OPT
  RC_FLUSH_VAR
  #endif
  return mi - (1 << numLevels);
}

int RangeDecoderReverseBitTreeDecode(CProb *probs, int numLevels, CRangeDecoder *rd)
{
  int mi = 1;
  int i;
  int symbol = 0;
  #ifdef _LZMA_LOC_OPT
  RC_INIT_VAR
  #endif
  for(i = 0; i < numLevels; i++)
  {
    #ifdef _LZMA_LOC_OPT
    CProb *prob = probs + mi;
    RC_GET_BIT2(prob, mi, ; , symbol |= (1 << i))
    #else
    int bit = RangeDecoderBitDecode(probs + mi, rd);
    mi = mi + mi + bit;
    symbol |= (bit << i);
    #endif
  }
  #ifdef _LZMA_LOC_OPT
  RC_FLUSH_VAR
  #endif
  return symbol;
}

Byte LzmaLiteralDecode(CProb *probs, CRangeDecoder *rd)
{ 
  int symbol = 1;
  #ifdef _LZMA_LOC_OPT
  RC_INIT_VAR
  #endif
  do
  {
    #ifdef _LZMA_LOC_OPT
    CProb *prob = probs + symbol;
    RC_GET_BIT(prob, symbol)
    #else
    symbol = (symbol + symbol) | RangeDecoderBitDecode(probs + symbol, rd);
    #endif
  }
  while (symbol < 0x100);
  #ifdef _LZMA_LOC_OPT
  RC_FLUSH_VAR
  #endif
  return symbol;
}

Byte LzmaLiteralDecodeMatch(CProb *probs, CRangeDecoder *rd, Byte matchByte)
{ 
  int symbol = 1;
  #ifdef _LZMA_LOC_OPT
  RC_INIT_VAR
  #endif
  do
  {
    int bit;
    int matchBit = (matchByte >> 7) & 1;
    matchByte <<= 1;
    #ifdef _LZMA_LOC_OPT
    {
      CProb *prob = probs + ((1 + matchBit) << 8) + symbol;
      RC_GET_BIT2(prob, symbol, bit = 0, bit = 1)
    }
    #else
    bit = RangeDecoderBitDecode(probs + ((1 + matchBit) << 8) + symbol, rd);
    symbol = (symbol << 1) | bit;
    #endif
    if (matchBit != bit)
    {
      while (symbol < 0x100)
      {
        #ifdef _LZMA_LOC_OPT
        CProb *prob = probs + symbol;
        RC_GET_BIT(prob, symbol)
        #else
        symbol = (symbol + symbol) | RangeDecoderBitDecode(probs + symbol, rd);
        #endif
      }
      break;
    }
  }
  while (symbol < 0x100);
  #ifdef _LZMA_LOC_OPT
  RC_FLUSH_VAR
  #endif
  return symbol;
}

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

int LzmaLenDecode(CProb *p, CRangeDecoder *rd, int posState)
{
  if(RangeDecoderBitDecode(p + LenChoice, rd) == 0)
    return RangeDecoderBitTreeDecode(p + LenLow +
        (posState << kLenNumLowBits), kLenNumLowBits, rd);
  if(RangeDecoderBitDecode(p + LenChoice2, rd) == 0)
    return kLenNumLowSymbols + RangeDecoderBitTreeDecode(p + LenMid +
        (posState << kLenNumMidBits), kLenNumMidBits, rd);
  return kLenNumLowSymbols + kLenNumMidSymbols + 
      RangeDecoderBitTreeDecode(p + LenHigh, kLenNumHighBits, rd);
}

#define kNumStates 12

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

#ifdef _LZMA_OUT_READ

typedef struct _LzmaVarState
{
  CRangeDecoder RangeDecoder;
  Byte *Dictionary;
  UInt32 DictionarySize;
  UInt32 DictionaryPos;
  UInt32 GlobalPos;
  UInt32 Reps[4];
  int lc;
  int lp;
  int pb;
  int State;
  int PreviousIsMatch;
  int RemainLen;
} LzmaVarState;

int LzmaDecoderInit(
    unsigned char *buffer, UInt32 bufferSize,
    int lc, int lp, int pb,
    unsigned char *dictionary, UInt32 dictionarySize,
    #ifdef _LZMA_IN_CB
    ILzmaInCallback *inCallback
    #else
    unsigned char *inStream, UInt32 inSize
    #endif
    )
{
  LzmaVarState *vs = (LzmaVarState *)buffer;
  CProb *p = (CProb *)(buffer + sizeof(LzmaVarState));
  UInt32 numProbs = Literal + ((UInt32)LZMA_LIT_SIZE << (lc + lp));
  UInt32 i;
  if (bufferSize < numProbs * sizeof(CProb) + sizeof(LzmaVarState))
    return LZMA_RESULT_NOT_ENOUGH_MEM;
  vs->Dictionary = dictionary;
  vs->DictionarySize = dictionarySize;
  vs->DictionaryPos = 0;
  vs->GlobalPos = 0;
  vs->Reps[0] = vs->Reps[1] = vs->Reps[2] = vs->Reps[3] = 1;
  vs->lc = lc;
  vs->lp = lp;
  vs->pb = pb;
  vs->State = 0;
  vs->PreviousIsMatch = 0;
  vs->RemainLen = 0;
  dictionary[dictionarySize - 1] = 0;
  for (i = 0; i < numProbs; i++)
    p[i] = kBitModelTotal >> 1; 
  RangeDecoderInit(&vs->RangeDecoder, 
      #ifdef _LZMA_IN_CB
      inCallback
      #else
      inStream, inSize
      #endif
  );
  return LZMA_RESULT_OK;
}

int LzmaDecode(unsigned char *buffer, 
    unsigned char *outStream, UInt32 outSize,
    UInt32 *outSizeProcessed)
{
  LzmaVarState *vs = (LzmaVarState *)buffer;
  CProb *p = (CProb *)(buffer + sizeof(LzmaVarState));
  CRangeDecoder rd = vs->RangeDecoder;
  int state = vs->State;
  int previousIsMatch = vs->PreviousIsMatch;
  Byte previousByte;
  UInt32 rep0 = vs->Reps[0], rep1 = vs->Reps[1], rep2 = vs->Reps[2], rep3 = vs->Reps[3];
  UInt32 nowPos = 0;
  UInt32 posStateMask = (1 << (vs->pb)) - 1;
  UInt32 literalPosMask = (1 << (vs->lp)) - 1;
  int lc = vs->lc;
  int len = vs->RemainLen;
  UInt32 globalPos = vs->GlobalPos;

  Byte *dictionary = vs->Dictionary;
  UInt32 dictionarySize = vs->DictionarySize;
  UInt32 dictionaryPos = vs->DictionaryPos;

  if (len == -1)
  {
    *outSizeProcessed = 0;
    return LZMA_RESULT_OK;
  }

  while(len > 0 && nowPos < outSize)
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
#else

int LzmaDecode(
    Byte *buffer, UInt32 bufferSize,
    int lc, int lp, int pb,
    #ifdef _LZMA_IN_CB
    ILzmaInCallback *inCallback,
    #else
    unsigned char *inStream, UInt32 inSize,
    #endif
    unsigned char *outStream, UInt32 outSize,
    UInt32 *outSizeProcessed)
{
  UInt32 numProbs = Literal + ((UInt32)LZMA_LIT_SIZE << (lc + lp));
  CProb *p = (CProb *)buffer;
  CRangeDecoder rd;
  UInt32 i;
  int state = 0;
  int previousIsMatch = 0;
  Byte previousByte = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  UInt32 nowPos = 0;
  UInt32 posStateMask = (1 << pb) - 1;
  UInt32 literalPosMask = (1 << lp) - 1;
  int len = 0;
  if (bufferSize < numProbs * sizeof(CProb))
    return LZMA_RESULT_NOT_ENOUGH_MEM;
  for (i = 0; i < numProbs; i++)
    p[i] = kBitModelTotal >> 1; 
  RangeDecoderInit(&rd, 
      #ifdef _LZMA_IN_CB
      inCallback
      #else
      inStream, inSize
      #endif
      );
#endif

  *outSizeProcessed = 0;
  while(nowPos < outSize)
  {
    int posState = (int)(
        (nowPos 
        #ifdef _LZMA_OUT_READ
        + globalPos
        #endif
        )
        & posStateMask);
    #ifdef _LZMA_IN_CB
    if (rd.Result != LZMA_RESULT_OK)
      return rd.Result;
    #endif
    if (rd.ExtraBytes != 0)
      return LZMA_RESULT_DATA_ERROR;
    if (RangeDecoderBitDecode(p + IsMatch + (state << kNumPosBitsMax) + posState, &rd) == 0)
    {
      CProb *probs = p + Literal + (LZMA_LIT_SIZE * 
        (((
        (nowPos 
        #ifdef _LZMA_OUT_READ
        + globalPos
        #endif
        )
        & literalPosMask) << lc) + (previousByte >> (8 - lc))));

      if (state < 4) state = 0;
      else if (state < 10) state -= 3;
      else state -= 6;
      if (previousIsMatch)
      {
        Byte matchByte;
        #ifdef _LZMA_OUT_READ
        UInt32 pos = dictionaryPos - rep0;
        if (pos >= dictionarySize)
          pos += dictionarySize;
        matchByte = dictionary[pos];
        #else
        matchByte = outStream[nowPos - rep0];
        #endif
        previousByte = LzmaLiteralDecodeMatch(probs, &rd, matchByte);
        previousIsMatch = 0;
      }
      else
        previousByte = LzmaLiteralDecode(probs, &rd);
      outStream[nowPos++] = previousByte;
      #ifdef _LZMA_OUT_READ
      dictionary[dictionaryPos] = previousByte;
      if (++dictionaryPos == dictionarySize)
        dictionaryPos = 0;
      #endif
    }
    else             
    {
      previousIsMatch = 1;
      if (RangeDecoderBitDecode(p + IsRep + state, &rd) == 1)
      {
        if (RangeDecoderBitDecode(p + IsRepG0 + state, &rd) == 0)
        {
          if (RangeDecoderBitDecode(p + IsRep0Long + (state << kNumPosBitsMax) + posState, &rd) == 0)
          {
            #ifdef _LZMA_OUT_READ
            UInt32 pos;
            #endif
            if (
               (nowPos 
                #ifdef _LZMA_OUT_READ
                + globalPos
                #endif
               )
               == 0)
              return LZMA_RESULT_DATA_ERROR;
            state = state < 7 ? 9 : 11;
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
            continue;
          }
        }
        else
        {
          UInt32 distance;
          if(RangeDecoderBitDecode(p + IsRepG1 + state, &rd) == 0)
            distance = rep1;
          else 
          {
            if(RangeDecoderBitDecode(p + IsRepG2 + state, &rd) == 0)
              distance = rep2;
            else
            {
              distance = rep3;
              rep3 = rep2;
            }
            rep2 = rep1;
          }
          rep1 = rep0;
          rep0 = distance;
        }
        len = LzmaLenDecode(p + RepLenCoder, &rd, posState);
        state = state < 7 ? 8 : 11;
      }
      else
      {
        int posSlot;
        rep3 = rep2;
        rep2 = rep1;
        rep1 = rep0;
        state = state < 7 ? 7 : 10;
        len = LzmaLenDecode(p + LenCoder, &rd, posState);
        posSlot = RangeDecoderBitTreeDecode(p + PosSlot +
            ((len < kNumLenToPosStates ? len : kNumLenToPosStates - 1) << 
            kNumPosSlotBits), kNumPosSlotBits, &rd);
        if (posSlot >= kStartPosModelIndex)
        {
          int numDirectBits = ((posSlot >> 1) - 1);
          rep0 = ((2 | ((UInt32)posSlot & 1)) << numDirectBits);
          if (posSlot < kEndPosModelIndex)
          {
            rep0 += RangeDecoderReverseBitTreeDecode(
                p + SpecPos + rep0 - posSlot - 1, numDirectBits, &rd);
          }
          else
          {
            rep0 += RangeDecoderDecodeDirectBits(&rd, 
                numDirectBits - kNumAlignBits) << kNumAlignBits;
            rep0 += RangeDecoderReverseBitTreeDecode(p + Align, kNumAlignBits, &rd);
          }
        }
        else
          rep0 = posSlot;
        rep0++;
      }
      if (rep0 == (UInt32)(0))
      {
        /* it's for stream version */
        len = -1;
        break;
      }
      if (rep0 > nowPos 
        #ifdef _LZMA_OUT_READ
        + globalPos
        #endif
        )
      {
        return LZMA_RESULT_DATA_ERROR;
      }
      len += kMatchMinLen;
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
        outStream[nowPos++] = previousByte;
        len--;
      }
      while(len > 0 && nowPos < outSize);
    }
  }

  #ifdef _LZMA_OUT_READ
  vs->RangeDecoder = rd;
  vs->DictionaryPos = dictionaryPos;
  vs->GlobalPos = globalPos + nowPos;
  vs->Reps[0] = rep0;
  vs->Reps[1] = rep1;
  vs->Reps[2] = rep2;
  vs->Reps[3] = rep3;
  vs->State = state;
  vs->PreviousIsMatch = previousIsMatch;
  vs->RemainLen = len;
  #endif

  *outSizeProcessed = nowPos;
  return LZMA_RESULT_OK;
}
