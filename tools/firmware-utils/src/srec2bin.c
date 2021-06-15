#include <stdio.h>
#include <ctype.h>
#include <string.h>

//Rev 0.1 Original
// 8 Jan 2001  MJH  Added code to write data to Binary file
//                  note: outputfile is name.bin, where name is first part
//                  of input file.  ie tmp.rec -> tmp.bin
//
//   srec2bin <input SREC file> <Output Binary File> <If Present, Big Endian>
//
//   TAG   
//        bit32u TAG_BIG     = 0xDEADBE42;
//        bit32u TAG_LITTLE  = 0xFEEDFA42;
//
//  File Structure
//
//  TAG    :   32 Bits
//  [DATA RECORDS]
//
//  Data Records Structure
//
//  LENGTH  :  32 Bits    <- Length of DATA, excludes ADDRESS and CHECKSUM
//  ADDRESS :  32 Bits
//  DATA    :  8 Bits * LENGTH
//  CHECKSUM:  32 Bits    <-  0 - (Sum of Length --> End of Data)
//
//  Note : If Length == 0, Address will be Program Start
//
//
//  
//
//

#define MajRevNum 0
#define MinRevNum 2


#define EndianSwitch(x) ((x >> 24) | (x << 24) | ((x << 8) & (0x00FF0000)) | ((x >> 8) & (0x0000FF00)) )

typedef unsigned char bit8u;
typedef unsigned int bit32u;
typedef int bit32;

#define FALSE 0
#define TRUE (!FALSE)


bit32u CheckSum;
int RecStart;
int debug;
int verbose;

FILE *OpenOutputFile( char *Name );
FILE *fOut;
bit32u RecLength=0;

bit32u AddressCurrent;

bit32u gh(char *cp,int nibs);

int BigEndian;

int inputline;

// char buf[16*1024];

char buffer[2048];
char *cur_ptr;
int cur_line=0;
int cur_len=0;

int s1s2s3_total=0;

bit32u PBVal;
int    PBValid;
bit32u PBAdr;


void dumpfTell(char *s, bit32u Value)
{
    int Length;
    Length = (int) RecLength;
    if (debug)
          printf("[%s  ] ftell()[0x%08lX] Length[0x%4X] Length[%4d] Value[0x%08x]\n",
                s, ftell(fOut), Length, Length, Value);
}

void DispHex(bit32u Hex)
{
//    printf("%X", Hex);
}

void WaitDisplay(void)
{
   static int Count=0;
   static int Index=0;
   char iline[]={"-\\|/"};

   Count++;
   if ((Count % 32)==0)
   {
     if (verbose)
        printf("%c%c",iline[Index++],8);
     Index &= 3;
   }
}


void binOut32 ( bit32u Data )
{
// On UNIX machine all 32bit writes need ENDIAN switched
//    Data = EndianSwitch(Data);
//    fwrite( &Data, sizeof(bit32u), 1, fOut);

   char sdat[4];
   int i;

   for(i=0;i<4;i++)
    sdat[i]=(char)(Data>>(i*8));
   fwrite( sdat, 1, 4, fOut);
   dumpfTell("Out32" , Data);
}

// Only update RecLength on Byte Writes
// All 32 bit writes will be for Length etc

void binOut8 ( bit8u Data )
{
    int n;
    dumpfTell("B4Data" , (bit32u) (Data & 0xFF) );
    n = fwrite( &Data, sizeof(bit8u), 1, fOut);
    if (n != 1)
        printf("Error in writing %X for Address 0x%8X\n", Data, AddressCurrent);
    RecLength += 1;
}

//  Currently ONLY used for outputting Program Start

void binRecStart(bit32u Address)
{
    RecLength      = 0;
    CheckSum       = Address;
    RecStart       = TRUE;

    if (debug)
          printf("[RecStart] CheckSum[0x%08X] Length[%4d] Address[0x%08X]\n",
                CheckSum, RecLength, Address);


    dumpfTell("RecLength", RecLength);
    binOut32( RecLength );
    dumpfTell("Address", Address);
    binOut32( Address );
}

void binRecEnd(void)
{
    long RecEnd;

    if (!RecStart)   //  if no record started, do not end it
    {
        return;
    }

    RecStart = FALSE;


    RecEnd = ftell(fOut);         // Save Current position

    if (debug)
          printf("[RecEnd  ] CheckSum[0x%08X] Length[%4d] Length[0x%X] RecEnd[0x%08lX]\n",
                CheckSum, RecLength, RecLength, RecEnd);

    fseek( fOut, -((long) RecLength), SEEK_CUR);  // move back Start Of Data

    dumpfTell("Data   ", -1);

    fseek( fOut, -4, SEEK_CUR);  // move back Start Of Address

    dumpfTell("Address   ", -1);

    fseek( fOut, -4, SEEK_CUR);  // move back Start Of Length

    dumpfTell("Length   ", -1);

    binOut32( RecLength );

    fseek( fOut, RecEnd, SEEK_SET);  // move to end of Record

    CheckSum += RecLength;

    CheckSum =  ~CheckSum + 1;  // Two's complement

    binOut32( CheckSum );

    if (verbose)
        printf("[Created Record of %d Bytes with CheckSum [0x%8X]\n", RecLength, CheckSum);
}

void binRecOutProgramStart(bit32u Address)
{
    if (Address != (AddressCurrent+1))
    {
        binRecEnd();
        binRecStart(Address);
    }
    AddressCurrent = Address;
}
void binRecOutByte(bit32u Address, bit8u Data)
{
    //  If Address is one after Current Address, output Byte
    //  If not, close out last record, update Length, write checksum
    //  Then Start New Record, updating Current Address

    if (Address != (AddressCurrent+1))
    {
        binRecEnd();
        binRecStart(Address);
    }
    AddressCurrent = Address;
    CheckSum += Data;
    binOut8( Data );
}

//=============================================================================
//       SUPPORT FUNCTIONS
//=============================================================================
int readline(FILE *fil,char *buf,int len)
{
    int rlen;
    
    rlen=0;
    if (len==0)  return(0);
    while(1)
    {
      if (cur_len==0)
      {
        cur_len=fread(buffer, 1, sizeof(buffer), fil);
        if (cur_len==0)
        {
          if (rlen)
          {
            *buf=0;
            return(rlen);
          }
          return(-1);
        }
        cur_ptr=buffer;
      }
      if (cur_len)
      {
        if (*cur_ptr=='\n')
        {
          *buf=0;
          cur_ptr++;
          cur_len--;
          return(rlen);
        }
         else
         {
           if ((len>1)&&(*cur_ptr!='\r'))
           {
             *buf++=*cur_ptr++;
             len--;
           }
           else
             cur_ptr++;

           rlen++;
           cur_len--;
         }
      }
      else
      {
        *buf=0;
        cur_ptr++;
        cur_len--;
        return(rlen);
      }
    }
}


int SRLerrorout(char *c1,char *c2)
{
  printf("\nERROR: %s - '%s'.",c1,c2);
  return(FALSE);
}


int checksum(char *cp,int count)
{
  char *scp;
  int cksum;
  int dum;

  scp=cp;
  while(*scp)
  {
    if (!isxdigit(*scp++))
      return(SRLerrorout("Invalid hex digits",cp));
  }
  scp=cp;

  cksum=count;

  while(count)
  {
    cksum += gh(scp,2);
    if (count == 2)
        dum = ~cksum;
    scp += 2;
    count--;
  }
  cksum&=0x0ff; 
  //  printf("\nCk:%02x",cksum);
  return(cksum==0x0ff);
}

bit32u gh(char *cp,int nibs)
{
  int i;
  bit32u j;

  j=0;

  for(i=0;i<nibs;i++)
  {
    j<<=4;
    if ((*cp>='a')&&(*cp<='z')) *cp &= 0x5f;
    if ((*cp>='0')&&(*cp<='9')) 
      j += (*cp-0x30);
     else
      if ((*cp>='A')&&(*cp<='F'))
        j += (*cp-0x37);
       else
        SRLerrorout("Bad Hex char", cp);
    cp++;
  }
  return(j);
}


//=============================================================================
//       PROCESS SREC LINE
//=============================================================================

int srecLine(char *pSrecLine)
{
    char *scp,ch;
    int  itmp,count,dat;
    bit32u adr;
    static bit32u RecordCounter=0;

    cur_line++;
    scp=pSrecLine;
  
    if (*pSrecLine!='S')
      return(SRLerrorout("Not an Srecord file",scp));
    pSrecLine++;
    if (strlen(pSrecLine)<4)
      return(SRLerrorout("Srecord too short",scp));
  
    ch=*pSrecLine++;
  
    count=gh(pSrecLine,2);
  
    pSrecLine += 2;
  
  //  if(debug)
  //        printf("count %d, strlen(pSrecLine) = %d, pSrecLine =[%s]\n", count, strlen(pSrecLine), pSrecLine);
     RecordCounter++;
     DispHex(RecordCounter);
  
    if ((count*2) != strlen(pSrecLine)) return(SRLerrorout("Count field larger than record",scp));
  
    if (!checksum(pSrecLine, count)) return(SRLerrorout("Bad Checksum",scp));
  
    switch(ch)
    {
        case '0': if (count<3) return(SRLerrorout("Invalid Srecord count field",scp));
                  itmp=gh(pSrecLine,4); pSrecLine+=4; count-=2;
                  if (itmp) return(SRLerrorout("Srecord 1 address not zero",scp));
        break;
        case '1': if (count<3) return(SRLerrorout("Invalid Srecord count field",scp));
                  return(SRLerrorout("Srecord Not valid for MIPS",scp));
        break;
        case '2': if (count<4) return(SRLerrorout("Invalid Srecord count field",scp));
                  return(SRLerrorout("Srecord Not valid for MIPS",scp));
        break;
        case '3': if (count<5) return(SRLerrorout("Invalid Srecord count field",scp));
                  adr=gh(pSrecLine,8); pSrecLine+=8; count-=4;
                  count--;
                  while(count)
                  {
                    dat=gh(pSrecLine,2); pSrecLine+=2; count--;
                    binRecOutByte(adr, (char) (dat & 0xFF));
                    adr++;
                  }
                  s1s2s3_total++;
        break;
        case '4': return(SRLerrorout("Invalid Srecord type",scp));
        break;
        case '5': if (count<3) return(SRLerrorout("Invalid Srecord count field",scp));
                  itmp=gh(pSrecLine,4); pSrecLine+=4; count-=2;
                  if (itmp|=s1s2s3_total) return(SRLerrorout("Incorrect number of S3 Record processed",scp));
        break;
        case '6': return(SRLerrorout("Invalid Srecord type",scp));
        break;
        case '7': // PROGRAM START
                  if (count<5) return(SRLerrorout("Invalid Srecord count field",scp));
                  adr=gh(pSrecLine,8); pSrecLine+=8; count-=4;
                  if (count!=1) return(SRLerrorout("Invalid Srecord count field",scp));
                  binRecOutProgramStart(adr);
        break;
        case '8': if (count<4) return(SRLerrorout("Invalid Srecord count field",scp));
                  return(SRLerrorout("Srecord Not valid for MIPS",scp));
        break;
        case '9': if (count<3) return(SRLerrorout("Invalid Srecord count field",scp));
                  return(SRLerrorout("Srecord Not valid for MIPS",scp));
        break;
        default:
        break;
    }
    return(TRUE);
}
 

//=============================================================================
//       MAIN LOGIC, READS IN LINE AND OUTPUTS BINARY
//=============================================================================

int srec2bin(int argc,char *argv[],int verbose)
{
    int rlen,sts;
    FILE *fp;
    char buff[256];
    bit32u TAG_BIG     = 0xDEADBE42;
    bit32u TAG_LITTLE  = 0xFEEDFA42;

    bit32u Tag;

  
    if(argc < 3)
    {
      printf("\nError: <srec2bin <srec input file> <bin output file>\n\n");
      return(0);
    }
  
    if (argc > 3) BigEndian=TRUE; else BigEndian=FALSE;

    if (BigEndian)
        Tag = TAG_BIG;
    else
        Tag = TAG_LITTLE;

    if (verbose)
       printf("\nEndian: %s, Tag is 0x%8X\n",(BigEndian)?"BIG":"LITTLE", Tag);

    fp = fopen(argv[1],"rt");

    if (fp==NULL)
    {
      printf("\nError: Opening input file, %s.", argv[1]);
      return(0);
    }
  
    fOut = fopen( argv[2], "wb");
    
    if (fOut==NULL)
    {
      printf("\nError: Opening Output file, %s.", argv[2]);
      if(fp) fclose(fp);
      return(0);
    }
 
    RecStart = FALSE;

    AddressCurrent = 0xFFFFFFFFL;

    // Setup Tag 
  
    dumpfTell("Tag", Tag);

    binOut32(Tag);

  
    inputline=0;
    sts=TRUE;

    rlen = readline(fp,buff,sizeof buff);

    while( (sts) && (rlen != -1))
    {
        if (strlen(buff))
        {
            sts &= srecLine(buff);
            WaitDisplay();
        }
       rlen = readline(fp,buff,sizeof buff);
    }

  
  //  printf("PC: 0x%08X, Length 0x%08X, Tag 0x%08X\n", ProgramStart, RecLength, TAG_LITTLE);
  
    binRecEnd();

    if(fp) fclose(fp);
    if(fOut) fclose(fOut);

    return(1);
}

int main(int argc, char *argv[])
{
    debug = TRUE;
    debug = FALSE;
    verbose = FALSE;
    srec2bin(argc,argv,verbose);
    return 0;
}

