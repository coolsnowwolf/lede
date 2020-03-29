/*
   Copyright (c) 2000-2012, by Zhuo Meng (zxm8@case.edu).
   All rights reserved.

   Distributed under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/* For calendar in Encapsulated Postscript form with PS characters */

char PSjieqi[24][9] = {"j0b j16 ", "j11 j16 ", "j00 j01 ", "j02 j03 ",
                       "j04 j05 ", "j01 j06 ", "j07 j08 ", "j09 j02 ",
                       "j00 j0a ", "j0b j0c ", "j0d j0e ", "j0a j0f ",
                       "j0b j10 ", "j11 j10 ", "j00 j12 ", "j13 j10 ",
                       "j14 j15 ", "j12 j06 ", "j16 j15 ", "j17 j18 ",
                       "j00 j19 ", "j0b j1a ", "j11 j1a ", "j19 j0f "
};

char PSmiscchar[17][5] = {"m00 ", "n01 ", "n02 ", "n03 ", "n04 ", "n05 ",
                          "n06 ", "n07 ", "n08 ", "n09 ", "n0a ", "m01 ",
                          "m02 ", "m03 ", "m04 ", "m05 "
};

char PSbigmchar[22][5] = {"    ", "n11 ", "n12 ", "n13 ", "n14 ", "n15 ",
                          "n16 ", "n17 ", "n18 ", "n19 ", "n1a ", "    ",
                          "m12 ", "m13 ", "m14 ", "m15 ", "m16 ", "m17 ",
                          "m18 ", "m19 ", "m1a ", "m1b "
};

char PStiangan[10][5] = {"t00 ", "t01 ", "t02 ", "t03 ", "t04 ",
                         "t05 ", "t06 ", "t07 ", "t08 ", "t09 "
};

char PSdizhi[12][5] = {"d00 ", "d01 ", "d02 ", "d03 ", "d04 ", "d05 ",
                       "d06 ", "d07 ", "d08 ", "d09 ", "d0a ", "d0b "
};

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "psmonth.h"
#include "verstr.h"

/* Input:
   month, month number with .5 indicating leap month
   nstart, day of calendar month that starts this lunar month
   ndays, number of days in this lunar month
   bYear, whether to be used in yearly or monthly calendar
   Output:
   monname, C string containing the PS characters for the month.
            monname should be allocated at least 44 bytes before the call
*/
void Number2MonthPS(double month, int nstart, int ndays, bool bYear, char *monname)
{
    monname[0] = 0; /* Clear */
    int nmonth = int(month);
    if (month - nmonth == 0.5) /* Leap month */
        strcat(monname, (bYear) ? PSmiscchar[13] : PSbigmchar[13]);
    if (nmonth > 10)
    {
        strcat(monname, (bYear) ? PSmiscchar[10] : PSbigmchar[10]);
        nmonth -= 10;
    }
    if (nmonth == 1 && strlen(monname) == 0) /* Zheng month */
        strcat(monname, (bYear) ? PSmiscchar[12] : PSbigmchar[12]);
    else
        strcat(monname, (bYear) ? PSmiscchar[nmonth] : PSbigmchar[nmonth]);
    strcat(monname, (bYear) ? PSmiscchar[14] : PSbigmchar[14]);
    if (!bYear)
    {
        if (ndays == 30)
            strcat(monname, PSbigmchar[17]);
        else
            strcat(monname, PSbigmchar[18]);
        char buf[30];
        sprintf(buf, "1234 SFC (%d) SC ct ", nstart);
        strcat(monname, buf);
        strcat(monname, PSbigmchar[15]);
        strcat(monname, PSbigmchar[21]);
    }
}

/* Input:
   nday, day number (1 - 30)
   Output:
   dayname, C string containing the PS characters for the day.
            dayname should be allocated at least 9 bytes before the call
*/
void Number2DayPS(int nday, char *dayname)
{
    dayname[0] = 0; /* Clear */
    if (nday <= 10)
        strcat(dayname, PSmiscchar[0]);
    if (nday > 10 && nday < 20)
    {
        strcat(dayname, PSmiscchar[10]);
        nday -= 10;
    }
    if (nday == 20)
    {
        strcat(dayname, PSmiscchar[2]);
        nday -= 10;
    }
    if (nday > 20 && nday < 30)
    {
        strcat(dayname, PSmiscchar[11]);
        nday -= 20;
    }
    if (nday == 30)
    {
        strcat(dayname, PSmiscchar[3]);
        nday -= 20;
    }
    strcat(dayname, PSmiscchar[nday]);
}

/* Input:
   titlestr, string to be included in the title, normally either
             Year XXXX or Month Year
   bIsSim, true for simplified characters, false for traditional characters
   bNeedsRun: true if character Run is needed, false otherwise
*/
void PrintHeaderPS(char *titlestr, bool bIsSim, bool bNeedsRun)
{
    bool bYear = false;
    if (strncmp(titlestr, "Year", 4) == 0)
        bYear = true;
    printf("%%!PS-Adobe-3.0 EPSF-3.0\n%%%%Title: Chinese Calendar for %s\n",
           titlestr);
    printf("%%%%Creator: ccal-%s by Zhuo Meng, http://thunder.cwru.edu/ccal/\n", versionstr);
    time_t now = time(NULL);
    printf("%%%%CreationDate: %s", ctime(&now));
    if (bYear)
    {
        printf("%%%%BoundingBox: 24 297 583 738\n");
        printf("%%%%DocumentNeededResources: font Times-Bold Bookman-Light\n");
    }
    else
    {
        printf("%%%%BoundingBox: 47 397 554 714\n");
        printf("%%%%DocumentNeededResources: font Times-Bold Bookman-Demi\n");
    }
    printf("%%%%Pages: 1\n");
    printf("%%%%EndComments\n");
    printf("%%%%BeginSetup\n");
    printf("/B {bind def} def\n/dpif {72 600 div} B\n");
    printf("/xdf {exch def} B\n");
    printf("/m {dpif mul exch dpif mul exch moveto} B\n");
    if (bYear)
        printf("/SF {/b xdf /s xdf b 0 gt {/Times-Bold}{/Bookman-Light} ifelse\n");
    else
    {
        printf("/SFC {/s xdf /Times-Bold findfont s scalefont setfont} def\n");
        printf("/SC {100 128 rmoveto show 0 -128 rmoveto} def\n");
        printf("/SF {/b xdf /s xdf b 0 gt {/Times-Bold}{/Bookman-Demi} ifelse\n");
    }
    printf("findfont s scalefont setfont} def\n/S {show} B\n");
    printf("/K {setrgbcolor} B\n/lpts {dpif div} B\n/ppts {dpif mul} B\n");
    if (bYear)
    {
        printf("/fsc 7.5 def\n");
        printf("/Ymh {gsave currentpoint translate\n");
        printf("gsave 1 0 0 K 2 0 m ptc m05 grestore\n");
        printf("21 lpts 0 m gsave ptc n01 grestore\n");
        printf("40 lpts 0 m gsave ptc n02 grestore\n");
        printf("59 lpts 0 m gsave ptc n03 grestore\n");
        printf("78 lpts 0 m gsave ptc n04 grestore\n");
        printf("97 lpts 0 m gsave ptc n05 grestore\n");
        printf("gsave 0 0.8 0 K 116 lpts 0 m ptc n06 grestore\ngrestore} B\n");
    }
    else
    {
        printf("/rpts {2} B\n/RR {/h xdf /w xdf /y xdf /x xdf\n");
        printf("gsave newpath x y m currentpoint translate\n");
        printf("newpath rpts 0 moveto w ppts 4 sub rpts sub dup dup 0 lineto rpts rpts -90 0 arc\n");
        printf("w ppts 4 sub h ppts 4 sub rpts sub lineto currentpoint exch pop rpts 0 90 arc\n");
        printf("rpts h ppts 4 sub lineto rpts h ppts 4 sub rpts sub rpts 90 180 arc\n");
        printf("0 rpts lineto rpts rpts rpts 180 270 arc closepath} def\n");
        printf("/Fcolor {1 0.9 1 K} B\n");
        printf("/F {RR Fcolor fill grestore} def\n");
        printf("/Lcolor {0 0 0 K} B\n");
        printf("/L {RR 0.1 setlinewidth Lcolor stroke grestore} def\n");
    }
    printf("/U {ufill} B\n");
    printf("/ct {currentpoint translate} B\n");
    printf("/ptc {fsc 1234 div dup scale currentpoint 128 sub moveto ct} B\n");
    printf("/NC {960 0 rmoveto ct} B\n");
    printf("/NNC {-960 0 rmoveto ct} B\n");
    printf("/WNC {1010 0 rmoveto ct} B\n");
    printf("%% Number characters\n");
    printf("/n01 {{<~Pop&Bz\"Q0F@!&Oa6!)s\"6!,M]V!/:Ot!2KZ=\"4.)l\",I\"t\"%%WJM!&Oa6~>\n");
    printf("<~!!*0*,QeA~>}U NC} B\n");
    printf("/n02 {{<~Pop&Tz\"Q0F@!1X,s!5&Bs!7V)>!:Bp\\!=T&%%\")n>6\"!dt6!pg!p!1X,s!&su6!*B66\n");
    printf("!,qqV!/^ct!2on=\"3:J`\"*=P`\"#L#5!&su6~>\n");
    printf("<~!!*0*,Qe&L\"XF0W~>}U NC} B\n");
    printf("/n03 {{<~Pop&fz\"Q0F@!-A<*!0dR*!3?8J!6,*h!9=51\".00=\"%%WN=!seuk!-A<*!3cM%%!71c%%\n");
    printf("!9aIE!<N;c!?_F,\"%%WIV!qZOV!k8:/!3cM%%!$hQC!(6gC!*fMc!-S@,!0dJJ\"4R=8\"+1+4\"$?R^\n");
    printf("!$hQC~>\n");
    printf("<~!!*0*,Qe&L\"XF0N!sKqP$3~>}U NC} B\n");
    printf("/n04 {{<~Pop'Uz\"Q0F@\"*=P0\"*=WU\"-`n@\"&oB0\"#pCA!6>8$!/L`a!/^kh!/q!,!/psV!/pq*\n");
    printf("!/^c:!/LV3!5nk2!5nl-\"$?RR\"$?Q?\"*af:\"*FU4\"*=Oj\"*=P0!5nta!IOuo!Ib+&!I\"TK!G2B3\n");
    printf("!E9)o!?M8<!5nmD!5nta!OMrR!_<E<!_<?j!_!-1!aYn0!gE^f!pBYe!uM&=\"\"s[[\"\"jUj\"\"XJ#\n");
    printf("\"![i(!tkX#!rr@r!pfrZ!n770!iuE]!fR/:!dt*C!e:=$!e:At\"$?[1\"$?Rf!5nlA!5nm,!CQrW\n");
    printf("!K7&M!M0>e!NuQ&!O`'X!OMrR~>\n");
    printf("<~!!+DK+pB7p\"T\\]L\"TncL!sJcO\"TnoS\"!RmU~>}U NC} B\n");
    printf("/n05 {{<~Pop''z\"Q0F@!rrB6\"\"44%%!oO,j!k\\Rk!RLm)!W3%%%%\"-`mi\"%%36e!sf!F!-A<Z!0dRZ\n");
    printf("!3?9%%!6,+C!9=5a!Pee>!LNpF!2orU!6>3U!8mnu!;Za>!>kk\\!L*X.!E]>L!&st+!*B5+!,qpK\n");
    printf("!/^bi!2om2\"4R<a\",$Ze\"%%WE>!q62;!rrB6!k85X!K[;/!R(Tf!ltEc!k85X~>\n");
    printf("<~!!+PO\"XF0R,Qe2r!s'YL$3~>}U NC} B\n");
    printf("/n06 {{<~Pop'Ez\"Q0F@!&+JU!)N`U!,)Fu!.k9>!2'C\\\"4RC>\",$aB\"%%33l!&+JU!Ls2S!QY<B\n");
    printf("!GDO&!DNU_!@Io:!;6F_!5nm.!.Y'<!%%7h4!&O[4!5SZ*!BC04!Ls2S!_<A0!f@#t!l\"b_!p0ME\n");
    printf("!t52)\"\"=5`\"$?R>\"&8ho\"(D7\"\"*afT\"-!;0\"-NY]\",Hs2\"+:1[!u:ob!`T4H!_<A0!Ls89!K[E!\n");
    printf("!Peec!T!oF!U9au!VQTN!XJkV![7^:!]pJq!^ZuA!]C-Q!\\+:a!VQVX!Ls89~>\n");
    printf("<~!!*0*,Qe&k\"!RmN\"T]nQ!s&KL\"UY~>}U NC} B\n");
    printf("/n07 {{<~Pop'1z\"Q0F@!OMnj!%%7m'!*fQ?!-A7g!0@6:!3cLb!OMnR!OMjB!OMiQ!SIHT![IbJ\n");
    printf("\"&&\\s\".T?n\"3LU\\\"5!U@\"2+]/\"0DR7\"/l4V\"/5et\"/#ZO\"/#[:\",Hu\"\",-bD\"+^Ib\"+1+(\"*O[A\n");
    printf("\"(VCj\"%%3-J!^HaE!X\\pd!Up)c!Up*A!Up.I\"2Fu:\"(VJk\"#p@T!Up.a!Up2I!Yba4!O)[Q!O;fp\n");
    printf("!OMq@!OMnj~>\n");
    printf("<~!!+8G\"X4$P\"![sO+pA&P-j'V[~>}U NC} B\n");
    printf("/n08 {{<~Pop'5z\"Q0F@!R(XB!Ghjc!GMW:!FGno!DW\\Y!B^DA!?_E\"!;Z^Q!7Lr)!0R>=!&ss8\n");
    printf("!(6f8!2KU6!:0^\"!?;+Q!DEN+!GheH!Ib(W!KR:d!M0A*!NZAS!R(XB!_<DY!bqd5!g3S_!l+h/\n");
    printf("!pp!R\"\"F;=\"*afF\",[(u\"/l3K\"4.$q\"4.%%,\"-NYX\"'GWJ\"!dmU!q$(_!lP,8!i?#5!f$i1!d+S>\n");
    printf("!cS6\\!g!M?!^Hjd!_<DY~>\n");
    printf("<~!!*0H\"TotQ!s'VM\"![so!t#~>}U NC} B\n");
    printf("/n09 {{<~Pop'Mz\"Q0F@!H\\CQ!*B<4!.4j4!1<nY!4`0(!8IXK!H\\C9!HA/H!EK5]!@.\\&!:^'B\n");
    printf("!29IM!&ssH!'gN@!4W#K!>5CG!DWY4!K$nu!N?+n!NZ?u!dk'F!dk!D!djua!gisa!mgpD\"+1)F\n");
    printf("\"/Z&r\"2t7R\"5!U8\"2t8-\"1\\E/\"1/'@\"0MXO\"0)A!\"/l5_\"-`gK\"-`fm\"-<N>\",[)k\"+pTA\"*=Ns\n");
    printf("\"'bhV!qZJC!m(Fm!jhrp!jhsK!ji#n!n7:U!g!KA!d\"LR!NZ@4!NZBR!RLq=!H8.^!HJ:0!H\\E,\n");
    printf("!H\\CQ~>\n");
    printf("<~!!+8G\"ToqP\"!Rmm!sJcO\"ToqP!sL(T\"UY~>}U NC} B\n");
    printf("/n0a {{<~Pop&fz\"Q0F@!Z1pI!Z1u(\"4.*7\",$`;\"%%WJm!Z1u<!Z2#i!^$RX!S@Lu!SRXA!Sdct\n");
    printf("!Sdcb!Sd`U!%%\\1N!)*GN!+Z-n!.Fu7!1X*U!Sd`A!Sd[V!Sd[;!SRNc!S@B'!ZV22!Z:ua!Z1p1\n");
    printf("!Z1pI~>\n");
    printf("<~!!+JM\"X=*Q+p.uN\"UY~>}U NC} B\n");
    printf("/m00 {{<~Pop(tz\"Q0F@!QY@*!U'V*!Xo/Z!a#OQ!`]<3!_req!^Zqa!]C(P!ZhA?!Vug.!S.7q\n");
    printf("!M0:K!E9%%i!F,Ue!O2WE!V-5p!Zh@<!_NJ\\!bVOu!d4V4!e^VF!f[8c!g!L4\")J&f\"&o;\\\"%%!!t\n");
    printf("\"#^.Z\"\"F;?\"!7N,!u:m\"!t50k!nmX>!e:9D!e:90!lt@e!qZIt!sAT\\\"#0dA\"&],&\"(hO^\"*ssA\n");
    printf("\"-*D2\"/H#5\"2k9q\"-<Uq\")%%d!!QY@*!7V,'!6bPh!9jTb!;lqP!<`L2!=T&h!>knf!@e1.!BUBI\n");
    printf("!Cd/g!D3H1!DW`O!@@oK!7V,'!A\"9D!A\":_!CR!D!Fc,8!Jgg<!O)Y#!H\\E#!Dik0!&Och!)s$h\n");
    printf("!,M`3!/:RQ!2K\\o!Dijq!@S#P!;ch2!6P?m!13fQ!+>o5!$hSm!%%\\.i!-A74!4W'^!;HU<!;HQl\n");
    printf("!;HQ>!;6DM!;$7F!AFLE!A+:r!A\"5D!A\"5d!A\"90!F,ZE!Ib'H!L*V:!N?*+!P&50!Q5\"J!R:^b\n");
    printf("!RUq#!R(S5!QG/E!Moh<!GDMl!It4L!LNp(!O)VT!QY=+!T!lO!V?Fm!OMoe!Mfd2!JCM0!E9*`\n");
    printf("!Cd+T!BC2K!A\"9D~>\n");
    printf("<~!!+;H,6\\/o\"TotQ\"![sp!s&KL\"T\\]/,m+;Q+pA&P!sJc1\"\"\"0R+pA;~>}U NC} B\n");
    printf("/m01 {{<~Pop'3z\"Q0F@!H\\BV!f-nk!f-ip!H\\=[!H\\BV!&t%%-!*B;-!,VdJ!/:Pg!2K[0!B^Es\n");
    printf("!B^Ag!B^A$!BC.-!Ajd-!H\\<0!H\\=G!f-i\\!f-hA!lt@H!l=r%%!l+fa!l+gR!l+kN\"3^g[\",$`[\n");
    printf("\"&&cD!l+kb!l+nC!pB`6!e^ZO!epf!!f-qA!f-pY!f-o*!H\\Bj!H\\ES!LNt>!B:1[!BL=*!B^HL\n");
    printf("!B^Gi!B^F2!&t%%-~>\n");
    printf("<~!!+>I!<WKL!sKtQ\"XjHV,m+;p!t#~>}U NC} B\n");
    printf("/m04 {{<~Pop'Kz\"Q0F@!GDRW!pg#2!pfue!GDP5!GDRW!GDP!!pfuQ!pfs3!Fu5T!G2BF!GDO6\n");
    printf("!GDP!!]0nU!\\aV=!g!D0!keS=!k84e!pTbR!sns-!u(`K\"!.Gg\"!dl+\"!dl>\"!dtR\"&K)A!sA_5\n");
    printf("!n[U2!IP!*!A\">W!AOZ[!AX_.!AFR%%!A+>p!@7br!>ki,!=Ai9!:p3D!7CkK!3cHP!/LVU!*B4X\n");
    printf("!+Z'X!2]`m!8[^+!=St<!BC/L!EK4i!Fu5@!pfrt!pfp*!pfoW!oF!7!lb5!!iuB^!dk!8!]0nU\n");
    printf("~>\n");
    printf("<~!!+>I!@%%[M!<XPM-3GLs\"!Rmm\"!RmU~>}U NC} B\n");
    printf("/m05 {{<~Pop&nz\"Q0F@!>GWT!q6;2!q67R!>GSt!>GWT!>GS`!q67>!q632!>GOT!>GS`!8%%96\n");
    printf("!>GNE!>GO@!q62s!q61\\\"\"XFc\"\"=5K\"\"40;\"\"414\"\"47R\"&K)=!t5:5!osH:!?_Jt!8%%CT!87M]\n");
    printf("!8IX!!8IVG!8ITm!87G.!8%%96~>\n");
    printf("<~!!+>I!@.aJ,m+;t\"!RmU~>}U NC} B\n");
    if (!bYear)
    {
        printf("/bf {dpif 1234 fsc div mul neg dup neg rmoveto ct} B\n");
        printf("/xfac {fsc 6 div} B\n");
        printf("/nR {xfac 2 gt {xfac}{2} ifelse} B\n");
    }
    if (bYear)
    {
        PrintHeaderMonthPS(NULL, 0, false, bIsSim, bNeedsRun, 6);
        printf("gsave 30 300 moveto currentpoint translate\n");
        printf("%% Grid lines\ngsave 0.9 0.9 1 K 3 ppts setlinewidth\n");
        printf("newpath 0 2236 m 546 0 rlineto stroke\n");
        printf("newpath 0 1086 m 546 0 rlineto stroke\n");
        printf("newpath 132 399 moveto 0 -398 rlineto stroke\n");
        printf("newpath 272 399 moveto 0 -398 rlineto stroke\n");
        printf("newpath 412 399 moveto 0 -398 rlineto stroke\n");
        printf("grestore\n");
        printf("%% Year heading\n28 1 SF\n2030 3450 m (%s) S\n", &titlestr[5]);
    }
}

/* Inputs:
   monthhead, string header for the monthly calendar if bSingle is
              true, don't care otherwise
   month, month number for the monthly calendar if bSingle is
          true, don't care otherwise
   bSingle, true for single month, false for whole year
   bIsSim, true for simplified characters, false for traditional characters
   bNeedsRun: true if character Run is needed, false otherwise
   nWeeks: number of week lines in month
*/
void PrintHeaderMonthPS(char *monthhead, short int month, bool bSingle,
                        bool bIsSim, bool bNeedsRun, int nWeeks)
{
    if (!bSingle || strstr(monthhead, "m12 ") != NULL ||
        month == 2 || month == 3)
    {
        printf("/m02 {{<~Pop&tz\"Q0F@!9aJ(!9aE!!'gO#!+5e#!-eKC!0R=a!3cH*\"3^aI\"+1*U\"$?R\"![Ib^\n");
        printf("![Ig1\")n;)\"\"445!qZNk![IgE![Ik1\"0;Se\"'bqq!uqD>!.4lF!1X-F!42hf!6t[/!:0eM!UKnN\n");
        printf("!UKf&!?_AY!?_G'!C-]_!9=3/!9O>X!9aJ:!9aJ(~>\n");
        printf("<~!!+;H\"YTr]-3FDY~>}U NC} B\n");
        if (bSingle)
            printf("/m12 {gsave 1 1 nR {m02 NNC bf} for grestore WNC} B\n");
    }
    if (bNeedsRun)
    {
        if (bIsSim)
        {
            printf("/m03 {{<~Pop(2z\"Q0F@!;lo>!?;0>!Ajk^!DW^'!GhhE!TX<h!TX:^!@.^H!CQtH!F,Zh!HnM1\n");
            printf("!L*WO!TX:J!TX8$!8%%;l!;HQl!>#87!@e*U!D!4s\"$?Si!q64e!k\\OR!ZV4\\!ZV7-!rrAg!l+jg\n");
            printf("!g!H`!ZV7A!ZV9K!uqBL!n[SL!iQ1A!;lo>!6bPh!5J]P!9sZD!<rX3!>GVq!?hOX!AFTc!C?l>\n");
            printf("!E0(l!E9/1!Cd07!B1+<!=f4/!6bPh!-eJt!4W#&!4W+.!8IYj!-eT:!.\"_f!.4j=!.4gg!.4e;\n");
            printf("!.\"W[!-eJt!K[D*!O)Z*!QY@J!TF2h!WW=1\"&&f)\"&&]n\"&&]V\"$lp?\"\"XG,!u:ll!q62K!k85p\n");
            printf("!k85`!qcP8!uq;A\"\"XF'\"&&\\Q\"(M=!\"*+B@\"+UA^\",Hr%%\",Hr=\",I%%M\"/l<4\")%%e0\"$cs1!K[D*\n");
            printf("~>\n");
            printf("<~!!*0*,Qe2q!sL:Z!<XSN!@.ak\"T\\]/+p0\"m\"![sq!t#~>}U NC} B\n");
        }
        else
        {
            printf("/m03 {{<~Pop(Zz\"Q0F@!@.^h!CQth!F,[3!HnMQ!L*Wo!TX:j!TX9G!CQsQ!Fu4Q!IOoq!L<b:\n");
            printf("!OMlX!TX93!TX7M!<<,h!?_Bh!B:)3!E&pQ!H8%%o!tYIf!n75f!iuCk!Z1q,!Z1rg!lP+i!fR/i\n");
            printf("!b_V!!Z1s&!Z1tI!pfss!jD_s!fR1'!@.^h!/(=t!5JS&!5JZ+!It6A!It5r!Or2m!OW!?!OMq5\n");
            printf("!OMrN!Rq41!MBP1!I+^6!5nu,!/(He!/:T$!/L^9!/L[L!/LX_!/:Jk!/(=t!5J\\i!It9*!It7t\n");
            printf("!5J[^!5J\\i!e:@a\"$cr\"\"$cpl!e:?V!e:@a!_<B/!e:?*!e:?B\"$cpX\"$cjV\"$cjF\"$?R7\"#L\"*\n");
            printf("\"\"XFq!rW+F!iQ*T!iQ*D!p'Dq!t>6)\"!@Rl\"%%<2I\"'kmn\")%%[2\"*+BI\"*afb\"*ag%%\"*aoe\".TIL\n");
            printf("\"(25L\"#L+I!f-rG!_<F+!_NQd!_`]/!_`\\3!_`[6!_NNQ!_<B/!e:B+\"$csA\"$cr6!e:@u!e:B+\n");
            printf("!5J[J!It7`!It6U!5JZ?!5J[J~>\n");
            printf("<~!!*0*,Qe2q!sL:Z!@7gO,m,=p!@.aJ,Qe&m\"!RmN,6]Cr+p@um!s'YL$3~>}U NC} B\n");
        }
        if (bSingle && strstr(monthhead, "m13 ") != NULL)
            printf("/m13 {gsave 1 1 nR {m03 NNC bf} for grestore WNC} B\n");
    }
    if (bSingle)
    {
        printf("/m14 {gsave 1 1 nR {m04 NNC bf} for grestore WNC} B\n");
        printf("/m15 {gsave 1 1 nR {m05 NNC bf} for grestore WNC} B\n");
        printf("/m06 {{<~Pop'Sz\"Q0F@!>kl#!X&Qe!X&O;!>kiN!>kl#!@S'\"!;Ze0!6bNX!1X,A!,M_*!(I$,\n");
        printf("!%%\\1J!&OaF!,2LP!0[JK!3u[5!71es!:Bp^!=T&M!X&TF!X&R$!>kl7!8IWt!8[c>!8mn;!8mlk\n");
        printf("!%%\\.a!)*Da!+Z+,!.FrJ!1X'h!X&O'!X&MU!X&M/!Wi@F!WW3G!^m#R!^Qfq!^HaC!^Haq!^Hcc\n");
        printf("\"4R?N\",HuR\"&&`/!^Hd\"!^HfL\")J$$\"!dr,!osDZ!^Hf`!^Hi-\"0_kI\")%%dQ\"\"47*!>knm!A+CM\n");
        printf("!C?m+!E]G_!IP!>!@S'\"~>\n");
        printf("<~!!+>I!?q[L+pB4o\"X4$P+p.uN\"YTr]+p//~>}U NC} B\n");
        printf("/m16 {gsave 1 1 nR {m06 NNC bf} for grestore WNC} B\n");
    }
    if (!bSingle || strstr(monthhead, "m17 ") != NULL || month == 1 ||
        month == 7 || month == 12)
    {
        printf("/m07 {{<~Pop';z\"Q0F@!Rq1\\!'gUq!+5kq!/(EL!Rq1H!R:a<!P\\[-!N#mn!K7%%Y!FPp;!?_Bh\n");
        printf("!8mj?!0I7h!&O[8!&ss,!4i/.!?qN)!H8%%q!PJLb!Up,d!XJiu![7[)!`/oK!g!F.!mgqe!uq;h\n");
        printf("\"*=N6\",6eh\"/GpB\"3^am\"3^b(\"&&]t!pBXT!ho]r!a>]:!\\+6l!Y>E`!Y>F/\"4.+F\",$aF\"&&d+\n");
        printf("!Y>FC!Y>Hi!\\a_T!RLqu!R_(G!Rq3?!Rq1\\~>\n");
        printf("<~!!+>I,6\\/q\"ToqP-j'V[~>}U NC} B\n");
        if (bSingle && strstr(monthhead, "m17 ") != NULL)
            printf("/m17 {gsave 1 1 nR {m07 NNC bf} for grestore WNC} B\n");
    }
    if (!bSingle || strstr(monthhead, "m18 ") != NULL || month == 1 ||
        month == 5 || month == 7 || month == 11)
    {
        printf("/m08 {{<~Pop'Wz\"Q0F@!UKmo!UKg1!UKf`!T*mA!QG,*!NZ9f!J(6@!CQp`!CQpP!LWq8!QG+B\n");
        printf("!R(Np!Xo&m!\\+1d![n&U![n/$!_`]h!U'X0!U9cW!UKnl!UKmo!Ajkr!A\";J!?M;i!=Am%%!;6I6\n");
        printf("!87J=!4Dp:!0RA7!,Va:!([,E!*B7I!1a-s!7V%%:!<*#I!@IpV!CQuG!E9+q!Fu7E!HnNi!K7)5\n");
        printf("!Ajkr!gj&%%!fR2b!m(LY!r)gK!uM(6\"#p=u\"&K#[\"(2.=\")n8s\"+UD#\"-N[O\"/>m$\"0)BF\"/Z*`\n");
        printf("\"/5h$\"-!?3\")%%`9\"%%!&=!rW17!gj&%%~>\n");
        printf("<~!!*0G\"ToqP,6J)M,6\\/o\"Tnc--3X_~>}U NC} B\n");
        if (bSingle && strstr(monthhead, "m18 ") != NULL)
            printf("/m18 {gsave 1 1 nR {m08 NNC bf} for grestore WNC} B\n");
    }
    if (bSingle)
    {
        if (strstr(monthhead, "m19 ") != NULL)
        {
            printf("/m09 {{<~Pop'%%z\"Q0F@!8ISH!5nm0!3uV\"!2onu!1a,r!1!Wu!0R@*!0.(3!0%%\"?!0@4N!0R@\\\n");
            printf("!1X'r!3?39!5&>U!71al!9aH*!<<.<!>#9?!>ki2!?_D$!?M7\\!>GP1!=8bY!;6E&!87FA!58G[\n");
            printf("!2T[1!0@1o!.\"WV!-eKY!/LVu!13b<!2fgZ!4Dm'!5nlG!7:eo!8ISH~>\n");
            printf("<~!!+VS$3~>}U NC} B\n");
            printf("/m19 {gsave 1 1 nR {m09 NNC bf} for grestore WNC} B\n");
        }
        printf("/m1a {WNC} B\n");
        printf("/m0b {{<~Pop)'z\"N1G]!^Hdj\"%%WHs\"%%WEf!^Ha]!^Hdj!fR62!c.s^!_NP?![n-,!X/Xl!TX<&\n");
        printf("!Q5%%Y!U'S-!Vc^M![.V/!b;@)!i?$\"!s&I3\"*=T\\\"+C;P\",?qE\"-*F8\"-`j*\"/#]6\"0_hZ\"2Ft)\n");
        printf("\"2b1D\"1ePY\"0_il\"*Oai!t58O!rrE7\"\"aT4\"&T-&\")J$d!lP.`!_`Z9!X&RD!\\\"2<!`/sH!dXqk\n");
        printf("!i#j7!lb>1!osHZ!fR62\"+UEB\".TD%%\"'blj\"$cn*!^m(-!X&Pf!X8\\5!XJgA!XJf2!XJe#!X8X*\n");
        printf("!X&KK!^H`J!^HaI\"%%WER\"%%WD?\",$YB\"+^Gi\"+UC1\"+UEB!:U'.!H\\C]!EfIL!BC1X!>GQ.!:0_k\n");
        printf("!5JVO!0@5/!5JXR!8mp5!:U'.!D!<c!:0g3!9OBR!7h6E!5&Bc!%%\\3(!)*I(!+Z/H!.G!f!1X,/\n");
        printf("!4W*K!1Esa!.4h(!*fPH!/U_e!5JV,!<<-G!7Ckp!0$u1!&+C4!&ss0!13ap!9jKb!A\"6[!Drjh\n");
        printf("!H8%%m!K$lm!M]Xk!O;^$!Or-D!PJKb!Or-s!NH/#!Lj*&!I4\\l!CQrJ!Ghe\"!KI3m!NZ@,!QY>`\n");
        printf("!JggP!H8+m!;$?F!=8iX!?)&O!@S&+!D!<c~>\n");
        printf("<~!!+>I!?q[L-3XJs\"TncO\"!Rmp!sJ]-,6\\)M\"X4$P\"![sO-3YXs\"To)~>}U NC} B\n");
        printf("/m1b {gsave 1 1 nR {m0b NNC bf} for grestore WNC} B\n");
        printf("/n11 {gsave 1 1 nR {n01 NNC bf} for grestore WNC} B\n");
        printf("/n12 {gsave 1 1 nR {n02 NNC bf} for grestore WNC} B\n");
        printf("/n13 {gsave 1 1 nR {n03 NNC bf} for grestore WNC} B\n");
        printf("/n14 {gsave 1 1 nR {n04 NNC bf} for grestore WNC} B\n");
        printf("/n15 {gsave 1 1 nR {n05 NNC bf} for grestore WNC} B\n");
        printf("/n16 {gsave 1 1 nR {n06 NNC bf} for grestore WNC} B\n");
        if (strstr(monthhead, "n17 ") != NULL)
            printf("/n17 {gsave 1 1 nR {n07 NNC bf} for grestore WNC} B\n");
        if (strstr(monthhead, "n18 ") != NULL)
            printf("/n18 {gsave 1 1 nR {n08 NNC bf} for grestore WNC} B\n");
        if (strstr(monthhead, "n19 ") != NULL)
            printf("/n19 {gsave 1 1 nR {n09 NNC bf} for grestore WNC} B\n");
        if (strstr(monthhead, "n1a ") != NULL)
            printf("/n1a {gsave 1 1 nR {n0a NNC bf} for grestore WNC} B\n");
        if (strstr(monthhead, "t00 ") != NULL)
        {
            printf("/t00i {{<~Pop';z\"Q0F@![%%NR\"!doW\"!dnd\"(2.g\"'P`<\"'>V&\"'>Y%%\",I%%m\"%%WNm\"!dtr!:0f8\n");
            printf("!3cQu!3u\\l!42ga!42fT!42eF!3uX6!3cK'!9aH*!9aHn!UKis!UKi3!U9[Y!U'M?![IbB![.Pl\n");
            printf("![%%L9![%%NR!9aMq!UKo!!UKlX!9aKS!9aMq![%%SU\"!dtZ\"!dr<![%%Q7![%%SU!9aK?!UKlD!UKj2\n");
            printf("!9aI-!9aK?![%%Q#\"!dr(\"!dok![%%Nf![%%Q#~>\n");
            printf("<~!!+;H\"XO6p\"X=*Q!sJ]N!s'YL!@.aJ,QeA~>}U NC} B\n");
            printf("/t00 {gsave 1 1 nR {t00i NNC bf} for grestore WNC} B\n");
        }
        if (strstr(monthhead, "t01 ") != NULL)
        {
            printf("/t01i {{<~Pop';z\"Q0F@!0dRn!42hn!6bO9!9OAW!<`Ku!al+(!GDO&!8%%=B!3uW)!/gjd!/(@(\n");
            printf("!1j2!!4Mrm!8RX.!>#6a\"\"XG*\")\\*n\".T@_\"1/'P\"-rr:\",$[>\"+C7\\\"*Xc$\"*+ES\")n:B\"'>T*\n");
            printf("\"'#@n\"&K\")\"%%iR[\"%%*(6\"\"jSe!t51>!@RrX!;uo-!9+!r!7h/(!6P<2!87Gn!<rR2!AX\\J!J1@Q\n");
            printf("!Vc_H!c8#>!kJJ#!oO/O!gEeO!aGh8!0dRn~>\n");
            printf("<~!!*0*\"![sO,6\\/n\"Tp\"R,6J8~>}U NC} B\n");
            printf("/t01 {gsave 1 1 nR {t01i NNC bf} for grestore WNC} B\n");
        }
        if (strstr(monthhead, "t02 ") != NULL)
        {
            printf("/t02i {{<~Pop'cz\"Q0F@!Z2!C\"\"XMX\"\"XG^\"\"XGF\"!R`0!tG<q!r;n]!mL_4!f-iL!f-i8!mgpp\n");
            printf("!rW+*!tYGh\"\"4.5\"$?QV\"&&\\u\"'bh>\"(VC^\"(VD)\"(VJ#\",m;g\"&K'g\"!drd!Z2!W!ZV;Y\"2\"_P\n");
            printf("\"*=XP\"$cs=!'C@6!*fV6!-A<V!0..t!3?9=!TX?!!T4$t!8IXo!13iG!1Etc!1X*M!1X(W!1X&a\n");
            printf("!1En:!13a;!7V!B!7V(S!T4$`!SRSN!K$nh!:U!X!;HQP!I+W!!R1Y)!Vc]j!^Zq$!d4Tq!g3RP\n");
            printf("!j2P.!lG$7!n%%)l!oO)J!n[Nh!k88o!gj\"u!a5W`!WW91!Y#2n!YkcW!Z2!C~>\n");
            printf("<~!!+8G+pA&n\"Y0ZY,Qf4o,6J)O-3X_~>}U NC} B\n");
            printf("/t02 {gsave 1 1 nR {t02i NNC bf} for grestore WNC} B\n");
        }
        if (strstr(monthhead, "t03 ") != NULL)
        {
            printf("/t03i {{<~Pop&hz\"Q0F@!B:(P!B:(<!Jg`)!OMi7!PACd!SdZ7!V?@\\!X8X,!Z(iO![%%Jn![%%K1\n");
            printf("![%%SM\"3:RL\"*apL\"$?[)!&t(\"!*B>\"!,r$B!/^k`!2p!)!TX>f!TX6r!TX6W!S.7<!PAE\"!MKL\\\n");
            printf("!HeC2!B:(P~>\n");
            printf("<~!!*0H\"XX<T+p0\"m$3~>}U NC} B\n");
            printf("/t03 {gsave 1 1 nR {t03i NNC bf} for grestore WNC} B\n");
        }
        if (strstr(monthhead, "t04 ") != NULL)
        {
            printf("/t04i {{<~Pop(8z\"Q0F@\"*=O!!tG=5!keTq!e:;*!m1P'!sAYm\"\"XLQ\"'>V<!q68m!pTi<!o*iR\n");
            printf("!lb9[!j;Xb!fR/W!aGb:!\\OMd!Yths!Yb^g\"1SEb\"(VK^\"\"XN?!Yb_&!YGMX!Y>H6!Y>Hi!\\a_P\n");
            printf("!R(Ym!R:eG!R_'T!S@J?!;$?Z!3cPB!4;ls!4)_E!3-'a!2'?&!-nR4!&O[4!'C60!1!V:!6YB)\n");
            printf("!87HQ!9aI\"!:^+W!;$?F!S@J+!T4\"j!WE+Q!]0p7!R(Pf!EB,K!71^>!7V!2!F>b4!T!gM!`T1'\n");
            printf("!iZ0n!s/I4\"(2+$\"2+[g\"5s54\"3^a5\"1A24\"0;KQ\"0;L4\"0;Ll\"0MYN\"1/)2\"/Gs&\"-rs*\",6g(\n");
            printf("\"*=O!!d\"OO!c.t;!gWqG!k&2E!m1U3!o=\"u!q6:!!s/Q9!ttbO\"!%%If\"!@\\*\"!RhA!u(iI!ql_B\n");
            printf("!nRO9!iuL\"!d\"OO~>\n");
            printf("<~!!+8I+p0%%n,Qe2o!sKkN+pA&m\"ToqP\"!n*Q\"T\\]O\"UY~>}U NC} B\n");
            printf("/t04 {gsave 1 1 nR {t04i NNC bf} for grestore WNC} B\n");
        }
        if (strstr(monthhead, "t05 ") != NULL)
        {
            printf("/t05i {{<~Pop'9z\"Q0F@!4W$m!3uTi!71^_!>#6M\"&&]6\"'GVC\")S$Y\",Hr#\"/5dA\"1\\Dn\"3^bT\n");
            printf("\"/,_6\",[*b\",Hu.\"*=Qo\"*=Pq\")n83\")7ha\"(M>8\"&],m\"#p:V!AFML!<iJ!!:Tuu!:U!H!:U%%L\n");
            printf("!q67N!q66_\"#'cf\"\"F@5\"\"45E\"\"47>\"%%WN!!tYR!!osGs!0@:r!3cPr!6>7=!9+)[!<<4$!q6:k\n");
            printf("!q67b!:U%%`!42fH!4Dqd!4W'8!4W$m~>\n");
            printf("<~!!*6*,6\\/n\"TnoR!sKtQ\"XF0R$3~>}U NC} B\n");
            printf("/t05 {gsave 1 1 nR {t05i NNC bf} for grestore WNC} B\n");
        }
        if (strstr(monthhead, "t06 ") != NULL)
        {
            printf("/t06i {{<~Pop(\\z\"Q0F@!0dS)!1!^0!13i6!13h:!13g=!0RB4!/LZ!!.=ka!*fN(!$hOu!&O[(\n");
            printf("!-S?u!2'>R!3uVj!5ei+!6kQ4!71d.!7Cq'!7V)(!7V*1\"1/.i\")J'i\"#pBV!ZV;9!\\jdc!]^@+\n");
            printf("!]1\"<!\\OSK!Y#7B!RLqu!QYAe!Smj\\!UBiS!V->I!Vcb=!WE10!X&U!!8IZM!0dS)!V?Gl!=/b*\n");
            printf("!@S#*!DEQZ!V?GX!V?FA!8IW(!;lm(!?_FX!V?F-!V?E\\!V$34!UKi_!<`G!!@.]!!D!6Q!U'QG\n");
            printf("!R1W6!E]>L!/LV3!/pn'!Ib#\\!X/S^![%%N*!\\aY:!bDAn!n@9j\"+UA.\",[(U\"/5d)\"3:IU\"3:Ia\n");
            printf("!tkTm!g!EI!^HdJ!rr@`!rr?u\"$?Tt\"$$CF\"#p>*\"#p>r\"5!Yh\".0-h\")J#e\"#p?1\"#p@8\"'>Vp\n");
            printf("\"!@Zp!r)hq!\\=DO!\\=E6!^m+j!Up1:!V-<i!V?HA!V?Gl!\\=D;!rrCe!rrBN!\\=C$!\\=D;!\\=Be\n");
            printf("!rrB:!rr@t![IfB!\\\"/i!\\=B=!\\=Be~>\n");
            printf("<~!!+;J\"![sp\"!RmN+pB.m!@\\*S,Qe2P\"ToqP!sKnO\"Y9`Z!@.aJ,6J)V~>}U NC} B\n");
            printf("/t06 {gsave 1 1 nR {t06i NNC bf} for grestore WNC} B\n");
        }
        if (strstr(monthhead, "t07 ") != NULL)
        {
            printf("/t07i {{<~Pop(2z\"Q0F@!AFTU!?_I9!C?k-!E]Dl!FPtH!GDO$!I\"T!!KI4B!Mfca!O)W)!O)WC\n");
            printf("!O)W\\!Mfdi!KI5k!I\"Ul!E]Ee!AFTU!OMsY!NZCA!QG5@!SIR<!TX?3!U^&)!VQUl!W3$R!.Y/*\n");
            printf("!2'E*!4W+J!7Crh!:U(1\"+UIn\"#L*r!r)jO!Y>Gf![Rq0!\\sjO!]C-k!]gF2!\\OS;!YPU2!VQW(\n");
            printf("!S.@m!OMsY!fR3m!e^XE!dFdh!bqe0!a>_K!_*5Z!\\=B]!%%\\0[!)*F[!+Z-&!.FtD!1X)b!S@GJ\n");
            printf("!S@E<!,qs(!0@4(!2ooH!5\\af!8ml/!S@E(!S@CW!S.6R!Rq)p!Z1ns!Yk];!YbX6!YbYd\")n8l\n");
            printf("\"!@Vl!oO)E!YbZ#!Yb\\1\"4RAH\",I\"L\"%%WJ%%!`/q,!c7us!f-nc!h]UN!k8<9!mh\"i!pB^4!fR3m\n");
            printf("~>\n");
            printf("<~!!*0J\"T\\]L\"X4$P,Qf7p!?q[k!sKqP\"Tno1\"XsNt\"To)~>}U NC} B\n");
            printf("/t07 {gsave 1 1 nR {t07i NNC bf} for grestore WNC} B\n");
        }
        if (strstr(monthhead, "t08 ") != NULL)
        {
            printf("/t08i {{<~Pop''z\"Q0F@!uM-U!pB`Z!gWq?![7_[!NlN!!A4I7!2K]J!2K]6!AO[#!MKTI!V?IR\n");
            printf("!V?Er!)N_\"!,qu\"!/L[B!29M`!5JX)!V?E^!V?AF!/LW.!2om.!5JSN!87El!;HP5\",m5!\"$ck!\n");
            printf("!rr=S!\\aV-!\\aZE\"2\"[(\")n<,\"#'cZ!\\aZY!\\a^E!n%%0W\"#pBt\"(24E!uM-U~>\n");
            printf("<~!!+8I!sKnO\"XF0R.K]hV$3~>}U NC} B\n");
            printf("/t08 {gsave 1 1 nR {t08i NNC bf} for grestore WNC} B\n");
        }
        if (strstr(monthhead, "t09 ") != NULL)
        {
            printf("/t09i {{<~Pop)5z\"Q0F@!.Y.s!-eS_!2'Dd!5&BY!6tYA!8dj'!:U&2!<N=c!>>O=!=]+S!:U'N\n");
            printf("!7CrH!3H>8!.Y.s!]1##![%%T`!]:'d!`/t0!ce@n!g<\\U!jr)J!n[QL!r;sL\"#0i\\\"+1.%%\"-`iW\n");
            printf("\"0hn1\"4RA\\\"4RAh\"-`j5\"'u$g\"#L'W!so$E!p'K<!ltG=!r)i5\"\"XN0\"(VK.\",m<^\"&K(b\"#'fi\n");
            printf("!r`8^!k\\TA!hKJC!epdL!d\"M]!j2Vg!oX5e!t59V\"#L+5!qZT1!nII8!iZ9-!c.re!`9&'!^6^J\n");
            printf("!]1##!S@KJ!WW=5!Q5)5!Ls7:!/L`%%!2p!%%!6bOU!Ls7&!HnP2!C['M!=/a!!6P?I!.Y*q!%%7lD\n");
            printf("!&+G<!13f*!9F7P!>kk\\!B:,`!E9+7!Sd_j!SIM,!Rq.H!R(Rh!/(At!2KWt!5&>?!7h0]!;$;&\n");
            printf("!QY:P!OVqR!K[<I!EoK7!@%%T#!6tRM!*fL`!+5dT!5\\_!!>YZ?!EoJV!M0:m!R:]%%!UKh,![[pH\n");
            printf("!a>Z_!f?un!k86'!o<p)!rN%%!!uV(l\"\"aL'\"#p9Q\"%%!!$\"#U(7!tYI4!pTd/!fR-Q!V?CD!XJg;\n");
            printf("\"+UD_\"$?U_!t53T!Xo*S!Yb[8!Z;$q!ZV7U!pft&!iuH&!e:>#!?_Fl!GDOJ!Mok>!S@KJ~>\n");
            printf("<~!!*0I\"T\\]N\"TotQ+p0%%n+p0\"m!@Isn\"TnoQ!sKkN\"ToqP\"\"+6u!sKqP\"UY~>}U NC} B\n");
            printf("/t09 {gsave 1 1 nR {t09i NNC bf} for grestore WNC} B\n");
        }
        if (strstr(monthhead, "d00 ") != NULL)
        {
            printf("/d00i {{<~Pop';z\"Q0F@!0@;=!3cQ=!6>7]!9+*&!<<4D!rN.B![%%QS!U'U?!U9`^!UKl&!UKkA\n");
            printf("!&Oa2!)s\"2!,M]R!/:Op!2KZ9!UKk-!UKg-!UKf_!T3sB!QkD,!ODcj!JCH@!B^@X!B^@D!KdA)\n");
            printf("!PJJ2!Pe[`!X/Qe![n%%X![n&9![n*i\"3^fd\")n<`\"#'d1![n+(![n+t!`T5c!\\a\\W!f-pB!mh#c\n");
            printf("!seue\"$crf\")%%dF\"+UJY\"#'he!r)kR!0@;=~>\n");
            printf("<~!!*0*,6J)n!sKkN+pA&m\"XjHs\"X=*V~>}U NC} B\n");
            printf("/d00 {gsave 1 1 nR {d00i NNC bf} for grestore WNC} B\n");
        }
        if (strstr(monthhead, "d01 ") != NULL)
        {
            printf("/d01i {{<~Pop&tz\"Q0F@!N6&:!pB\\&!mCY/!K[;G!N6&:!/L_j!2ouj!6bOE!It8O!Ghfg!0dO9\n");
            printf("!42e9!8%%>i!GhfS!DEKX!&+D;!)NZ;!,)@[!.k3$!2'=B\"4.$u\",Hru\"&o8b!t50o\"$?ZJ\"'bq1\n");
            printf("\"!du1!rN.6!/L_j!Pee:!rN.\"!pB\\:!NZ>R!Pee:~>\n");
            printf("<~!!+>I!@n6U.0BSr!t#~>}U NC} B\n");
            printf("/d01 {gsave 1 1 nR {d01i NNC bf} for grestore WNC} B\n");
        }
        if (strstr(monthhead, "d02 ") != NULL)
        {
            printf("/d02i {{<~Pop(pz\"Q0F@!<<0P!Q5$j!Q5#G!<</-!<<0P!W3!M!ltEo!ltDL!W2u*!W3!M!W2tk\n");
            printf("!ltD8!ltBf!W2sD!W2tk!<<.n!Q5#3!Q5!a!<<-G!<<.n!2ot;!6>5;!8mp[!;Zc$!>kmB!Q5&D\n");
            printf("!Q5%%)!<<0d!5nqH!6,'l!6>3,!6>20!6>13!6,$H!5nlq!<<,h!<<-3!ltBR!ltB&!sAW%%!s&EO\n");
            printf("!rr?u!rr@@!rrBB\"!@Y)!o*ij!l+k&!W3!a!W3#'\"\"XMX!osFX!iQ1=!2ot;!I+VF!AFMU!6tRX\n");
            printf("!)rqP!*fLL!1X$I!8dcU!@@eo!Ghb3!M0;\"!PAE>!I+VF!]0oP!\\=?<!h97l!oO''!r;mn!ttY_\n");
            printf("\"\"\"!h\"#9j5\"$Q]V\"$cin\"$-F*\"#Bq9!uM$4!pBWo!k86T!dk\"-!]0oP!uM*l\"!drp\"'u':\"-E[3\n");
            printf("\"2\"^Y\")n?]\"%%WMb!WW<n!ZD/K![@ek!ZD0$!Y>I0!U^'&!OMsY!N6+E!QkM9!Smiu!T4&N!0dRV\n");
            printf("!0@;-!.Y/r!,Vfl!*B='!($bN!%%S,t!%%\\2f!(I%%&!+,f9!-A:T!/:R!!1*cB!1X,i!0dRB\"&K(V\n");
            printf("!uM*l~>\n");
            printf("<~!!+>I!@.aJ,Qe&m!s&K-,Qf4o,m+<#!s&Q-+pA&L\"!n*O!sKnO+pA&P,6K1p+p//~>}U NC} B\n");
            printf("/d02 {gsave 1 1 nR {d02i NNC bf} for grestore WNC} B\n");
        }
        if (strstr(monthhead, "d03 ") != NULL)
        {
            printf("/d03i {{<~Pop(,z\"Q0F@!2'Ct!2'AF!29Lm!1X(I!/pr/!5JV#!6kOH!>,?k!K7'7!JCJs!Ghc`\n");
            printf("!Cd(S!?V<E!6kLc!)NYX!*B4T!;->'!EfDt!JCI<!NlGW!Q5\"r!Q5$6!Q5&P!U'U7!Jgg\\!K$r[\n");
            printf("!K7(W!K7'O!B^D.!<</-!8%%=J!8%%B1!C-`\\!K$tc!O`)H!TF3,!X8aX![Il$!Sddu!R(YO!N#sf\n");
            printf("!Ghjg!AO[f!<*'u!7V*A!1X.)!1j9B!2'D\\!2'Ct!dk)(\"&&eN\"&&`+\"&&_e\"%%E;Q\"$?TF\"#0g9\n");
            printf("!s/Ke!jD]!!iuD^!s&EF\"\"XHS\"\"XH1\"(hQ+\",$[n\",$\\O\",$b!\"/H#Y\"(24M\"%%35Z!dk)<!^Hiu\n");
            printf("!^Zu9!^m+P!^m*c!^m%%\\!^m%%<!^ZmL!^H`:!e:8A!dt&^!dk!5!dk!p!dk)(~>\n");
            printf("<~!!*0*\"![sO+pB1n+pA&m\"ToqP!sJ]L\"!RmN+pB:q\"Tno1\"To)~>}U NC} B\n");
            printf("/d03 {gsave 1 1 nR {d03i NNC bf} for grestore WNC} B\n");
        }
        if (strstr(monthhead, "d04 ") != NULL)
        {
            printf("/d04i {{<~Pop(6z\"Q0F@!;lq(\"/H$(\"'br(\"\"47j!<<4@!5J]$!5\\h-!5ns8!6,)E!65.P!5ejB\n");
            printf("!4i2q!3cJI!.b-O!%%\\+,!&ss,!.\"W^!3#t(!5nm2!8[`;!:Bl@!;$<C!;Q[D!;lnt!;lq(!D!:9\n");
            printf("!GDP9!It6Y!La)\"!Or3@\"&K'/!t58/!o*k$!D!:9!H8$`!MBEL!Pe\\7!X\\p^!e:9p!dk\"'!OMig\n");
            printf("!OMn:!Xo,=!ZV6b!\\se:!_rbq!bq`R!gE]I!mCYU!sAUa\"$?R\"\"*=NB\",6et\"/GpM\"3^au\"3^b0\n");
            printf("\"%%3-j!nmY6!g!FB!m1OG!qcS5!tYKb\"\"F>8\"%%30\\\"'bl\"!uq@*!sAYG!pfrh!nIC:!l\"b`!i,j-\n");
            printf("!e^SJ!au+W!^Qk\"![IgU\"0_h8\")%%a8\"#L'%%!>GSX!AjiX!DEP#!G)<@!It4[!It4[!It0S!It0@\n");
            printf("!I=a'!H8$`~>\n");
            printf("<~!!+AJ,6\\/o\"T\\]/,Qe&L\"XF0q\"TotQ\"![sq!sKkN\"UY~>}U NC} B\n");
            printf("/d04 {gsave 1 1 nR {d04i NNC bf} for grestore WNC} B\n");
        }
        if (strstr(monthhead, "d05 ") != NULL)
        {
            printf("/d05i {{<~Pop'/z\"Q0F@!;lpq!o*lS!o*iR!;lmp!;lpq!5J\\m!5\\h?!5nqY!5nle!58Gq!8RWp\n");
            printf("!?_Aa\"#L\"\"\")IsW\"-`e?\"0;L0\",$ZZ\"*\">:\"*=R&\"(2.g\"'kpc\"'5L!\"&8jK\"%%3-s\"\"jSM!t51.\n");
            printf("!C-X`!=]%%'!;6E#!;liT!;lm\\!o*i>!o*hK!uM(N!u1kp!u(g)!u(i\"\"#L*Z!qZSR!mh$[!<<44\n");
            printf("!5J\\m~>\n");
            printf("<~!!+>I!?q[L+pA&m\"TnoR!sKtQ$3~>}U NC} B\n");
            printf("/d05 {gsave 1 1 nR {d05i NNC bf} for grestore WNC} B\n");
        }
        if (strstr(monthhead, "d06 ") != NULL)
        {
            printf("/d06i {{<~Pop'-z\"Q0F@!ZV3Q!ZV6j\"2k5i\"*akq\"$?VJ!ZV7)!ZV:J\")J&F\"!dtN!pB_+!?;1]\n");
            printf("!Cd0-!GMY'!JCQJ!AFW*!>#?\\!:BqC!6bN6!3$%%(!.Fug!)*GJ!*B:J!2'C0!8[de!>GVA!T4%%c\n");
            printf("!T4\"B!&+H;!)N^;!,)D[!.k7$!2'AB!T4\".!T3sZ!T3s?!T!ff!SdZ'![%%J6!Z_8e!ZV36!ZV3Q\n");
            printf("~>\n");
            printf("<~!!+PO\"ToqP!sKqP\"X4$P!sK#~>}U NC} B\n");
            printf("/d06 {gsave 1 1 nR {d06i NNC bf} for grestore WNC} B\n");
        }
        if (strstr(monthhead, "d07 ") != NULL)
        {
            printf("/d07i {{<~Pop'Gz\"Q0F@!SdZ'![%%J6!Z_9(!ZV4p!ZV7a!\\=Bq!au+E!mgr4\"*ag=\",-`o\"/,_G\n");
            printf("\"3^bp\"3^c+\"!.ID!h'.)!^Hf0\"1/+P\")%%aP\"\"XL1!ZV7u!ZV:B\")%%c:!uqD:!pB_#!ZV:V!ZV;q\n");
            printf("!^Hjd!Sde,!T!p#!T4&:!T4%%o!1X-*!5&C*!7V)J!:Bph!=T&1!T4%%[!T4#=!)N_V!,quV!/L\\!\n");
            printf("!29N?!5JX]!PAIZ!Fu4a!9+\"X!&stC!'C77!:Bj<!I=c@!T4\"B!T3u6!T!gI!SdZ'~>\n");
            printf("<~!!*0*\"!RmN\"Y9`Z+p.uo!sJc1\"!RmU~>}U NC} B\n");
            printf("/d07 {gsave 1 1 nR {d07i NNC bf} for grestore WNC} B\n");
        }
        if (strstr(monthhead, "d08 ") != NULL)
        {
            printf("/d08i {{<~Pop'Iz\"Q0F@!4W+n!4i6o!5&An!5&@i!5&?c!4i2U!4W%%@!;$:;!;$;2!V?D3!V?C(\n");
            printf("!V-5`!Up(3!\\aU6!\\FC`!\\=>r!\\=@k\"\"41h\"\"41$\"(VF#\"(;4]\"(20?\"(22s\",$a^\"%%WM^!uqC[\n");
            printf("!\\=Ej!\\=G8!_`^#!Up3@!V->Y!V?Ir!V?I2!;HX5!4W+n!;$?r!V?Hs!V?Fi!;$=h!;$?r!\\=EV\n");
            printf("\"\"46S\"\"44I!\\=CL!\\=EV!;$=T!V?FU!V?DG!;$;F!;$=T!\\=C8\"\"445\"\"42'!\\=A*!\\=C8~>\n");
            printf("<~!!+8I,6J)O\"X=*Q-NaMr!s'YL!@.aJ,Qe&m!t#~>}U NC} B\n");
            printf("/d08 {gsave 1 1 nR {d08i NNC bf} for grestore WNC} B\n");
        }
        if (strstr(monthhead, "d09 ") != NULL)
        {
            printf("/d09i {{<~Pop'kz\"Q0F@!'gX2!+5n6!/q\"n!It8c!It6u!:U':!42h&!4Ds?!4W)\"!4W'\"!4W%%!\n");
            printf("!4DlU!42_k!:0\\b!:0]Q!tYI\"!tYGl\"%%WDg\"%%<4*\"%%30H\"%%33l\")%%b[\"!dsK!rN,H!d\"Lj!d\"NX\n");
            printf("\"3:RX\"*apX\"$cs5!'gX2!OMrB!^Hj$!^Hh6!OMpT!OMrB!:0d\"!It6a!IY#-!DigA!;HSF!<<.B\n");
            printf("!I=db!O`%%a!OMp@!^Hh\"!^Hem!]gA/!_rd'!dk$U!o*g`!r2l-!s8SE!r;rS!q66_!oa7W!mgu;\n");
            printf("!keWr!iQ.X!g3TD!dat.!c\\81!d\"JL!d\"LV!tYOH!tYK,!:0_[!:0d\"!:0_C!tYJi!tYI6!:0]e\n");
            printf("!:0_C~>\n");
            printf("<~!!+DK+pB7p\"XsNS,Qe&L\"TnoP!sJcP\"XO6O,QeA~>}U NC} B\n");
            printf("/d09 {gsave 1 1 nR {d09i NNC bf} for grestore WNC} B\n");
        }
        if (strstr(monthhead, "d0a ") != NULL)
        {
            printf("/d0ai {{<~Pop(Hz\"Q0F@!k8?J!iuL2!o=$A!s/RC!u:u6\"\"FC(\"$-N+\"%%WM@\"'#FS\"'bpj\"'u(/\n");
            printf("\"().H\"&T/N\"#L+C!u;!6!pp)q!k8?J\"+UB9\"#'_`!pTdR!jD\\f!o3m!!sSe3\"\"F?F\"&8nY\")S*G\n");
            printf("\",I\"h\"!drH!uqAj!t#*&!q67(!n@>(!jr'&!g!G!!b_V3!`/qB!_<CN\"2Fur\")n>n\"#L)O!_<Cb\n");
            printf("!_<DB!_<Dq!_<ED!cS73!X&VT!X8b+!XSs8!Xo/&!<<2n!4W+N!4i6b!4r;^!4i4A!4W'\"!3H8e\n");
            printf("!13c]!.k3T!*oSD!%%7h,!&O[0!+Z((!07,1!4DmM!8ISg!:^),!;$<E!W2uN!Or1R!JCL?!;HT]\n");
            printf("!;HWR!Xo.g!Y,8P!\\FG8!b_Ts!Z_:*!N6!e!=/[%%!=Srn!N?'>![[oW!e^Rc!mUeO\"!.G`\"*+B>\n");
            printf("\"3(<p\"6KS:\"4@0E\"24bO\"0qok\"0_dD\"0DRq\"0DSN\"0_f2\"/#[\"\"+UB9~>\n");
            printf("<~!!*0J\"T]hO\"![sp!sKkN\"X4$n\"ToqP-3GFq\"!n*p!t#~>}U NC} B\n");
            printf("/d0a {gsave 1 1 nR {d0ai NNC bf} for grestore WNC} B\n");
        }
        if (strstr(monthhead, "d0b ") != NULL)
        {
            printf("/d0bi {{<~Pop(8z\"Q0F@!NZCA!R:e@!U'W6!Vcb!!XJl`!Z(qf![\\\"3!]1!S!]gEn!]C.,!\\sk>\n");
            printf("!X8b/!OMsU!NZCA!'C?3!*fU3!-A;S!0.-q!3?8:!O)Y?!Jgg!!F>hh!Ajjm!=8fp!9aJ:!71cu\n");
            printf("!;HTQ!>5G#!BU>X!HJ5E!N6&0!T=(q!ZV7]!MfaN!>,>Q!+Z)f!,)AZ!D!4p!Y#0u!k8;j!oO-M\n");
            printf("!gEc=!dOjD!`];=![n+(!>kkh!DNVh!KI57!Sdb+!W3#[!PedO\"3^i]\"+UJ]\"%%WMB!'C?3!r)gR\n");
            printf("!n[PQ!iuF@!d4U!!^?]U!Vc['!MTSB!D<E[!8.?`!)*AP!)*A@!?hGF!S%%1P!b_T_!giuo!lY0#\n");
            printf("!q63(!u_0+\"$$@'\"&&\\o\"'tsa\")\\)i\"+1)2\",R\"O\",d.i\"+gN-\"*agD\"!.I%%!d\"H&!j2QE!p'Ha\n");
            printf("!uM(\"\"$cnZ!r)gR~>\n");
            printf("<~!!+>K!s&K-\"!RmN,6\\/Q+p.uN\"XX<P,6\\/s\"X4$U~>}U NC} B\n");
            printf("/d0b {gsave 1 1 nR {d0bi NNC bf} for grestore WNC} B\n");
        }
    }
    char jiestr[20];
    if (!bSingle)
        month = 1;
    strcpy(jiestr, PSjieqi[(month - 1) * 2]);
    strcat(jiestr, PSjieqi[(month - 1) * 2 + 1]);
    if (!bSingle || strstr(jiestr, "j00 ") != NULL)
    {
        printf("/j00 {{<~Pop'Uz\"Q0F@!NZCI!MBP5!QkM,!TjJn!V-=N!WE0.!Y5A8![\\!n!^$QM!^6]l!\\+:s\n");
        printf("!Ytm$!UKok!NZCI!,)H3!/L^3!2'DS!4i6q!8%%A:\"/H\"^\"&o@^!uM+;!,)H3!>klk!=/aS!@S\"-\n");
        printf("!C$VT!DWZs!F,Y;!G)9g!GDKL!GVW/!H\\>.!JUUJ!LEfd!M]Z.!NH/S!O)T!!NH0B!LNn`!JLR(\n");
        printf("!EoOG!>klk!pB\\n!tYN]!i,n-!gs*I!c.mj!ZV39!%%7iC!([*C!+5ec!.\"X,!13bJ\"5!U8\",Hs<\n");
        printf("\"&&]j!]U1U!cS/c!ic:2!pB\\n~>\n");
        printf("<~!!*0I\"T\\]/,Qe&L-3XDo!sKkN\"XF0R$3~>}U NC} B\n");
    }
    if (!bSingle || strstr(jiestr, "j01 ") != NULL)
    {
        printf("/j01 {{<~Pop(Vz\"Q0F@!PecP\"$?Y#!r)j#!ltGm!QY>l!R_&A!Smhm!U'VF\"-`mI\"&&fI!uM,6\n");
        printf("!UKn^!VlhN!X8b&!Yba<!PAN]!PAN2!O`)U!NZAs!+Z1*!/(G*!2ouZ!N6)[!MBN0!L<fZ!K7*0\n");
        printf("!/L^+!2ot+!6bM[!JCNi!HA1(!FPtJ!Dii\"!%%\\1R!)*GR!,r!-!CQuW!<E5c!20F]!$hSE!%%7k9\n");
        printf("!,;O@!2opJ!9+$V!?2'`!Drn!!JCMB!e:>?!h]St!ltDf!r;ql\"\"OCq\"'bk-\",m7K\"/,a+\"1eMT\n");
        printf("\"5!Wr\"5!X-\"0DT\\\",Hu?\")%%_*\"%%WHi\"!doY!rN)O!n78D!jMeE!gEaS\"3:O3\"+103\"%%32m!K7(^\n");
        printf("!Ls41!NcEd!PecP!F,Z$!f-lQ!f-k\"!F,XJ!F,Z$!F,X6!f-jc!f-i4!F,V\\!F,X6!l+hq!osBT\n");
        printf("!i,kP!e:<]!FPr<!?_Eq!?qQ+!@.\\8!@.[C!@.ZM!?qMY!?_@j!F,Uq!F,VH!f-hu!f-h)!lP(4\n");
        printf("!l4kV!l+fk!l+hq~>\n");
        printf("<~!!+>I\"XF0R!sKqP\"XF0R,Qe2P+pA&n\"TotQ,Qe2N,Qe&m!s'\\M+pB7p\"UY~>}U NC} B\n");
    }
    if (!bSingle || strstr(jiestr, "j02 ") != NULL)
    {
        printf("/j02 {{<~Pop(`z\"Q0F@!dk!@!dk!,!l4ka!pK\\k!qZIL\"!.G1\"$$?\\\"%%3-$\"&8i?\"&o8_\"&o9-\n");
        printf("\"&o>h\"*amS\"#pAW!t57L!Xo.K!Xo05\"3:RH\"+UKP\"&&f5!&t's!*B=s!,r$>!/^k\\!2p!%%!S@KV\n");
        printf("!S@Il!7V(g!13iS!1Et_!1X*A!1X(M!1X&X!1En:!13aK!71^N!71eO!S@IX!S@Fd!S.8@!Rq*C\n");
        printf("!Y>?J!Y#/-!Xo+C!Xo.7!uqBH!uq<V!uq<6!t#$j!pBWK!lY/*!hfU_!dk!@!;HV7!@7e6!CQu,\n");
        printf("!E9*l!Fu5V!H\\@c!JCL=!L*Wk!K[@,!I=f,!Fl1*!BL9m!<<1K!;HV7!;lk^!@@h`!CHlW!E&qB\n");
        printf("!FPp,!H8&8!It1f!K[=>!KI1U!I=cW!G2@X!BgID!<<.r!;lk^!^$Nt!b;?n!e:=c!g!HR!h]S@\n");
        printf("!jD^O!l+j)!mguW!mC]l!k&.i!hTNd!dOiR!^m*3!^$Nt!^HdB!bqa<!ep_/!g3Qo!hKDY!iuCe\n");
        printf("!kn[>!m^lk!mUg-!kJD/!i?!0!e:<!!_<?V!^HdB~>\n");
        printf("<~!!*0H\"Y0ZY,Qf4o,6J)O\"X4$m\"T]kP!s'YN!s'YN!s'YN!t#~>}U NC} B\n");
    }
    if (!bSingle || strstr(jiestr, "j03 ") != NULL)
    {
        printf("/j03 {{<~Pop'[z\"Q0F@!S@Lu!SRXL!Sdct!SdcB!Sd[r!Sd[L!RLh.!P/8o!M]XY!I4[4!B^@T\n");
        printf("!B^@@!K7#+!O_u5!PAC`!Vl^U!Z1oJ!Z1p=!Z2!k!c7t<!oX.C\"*=O)\"+1*Q\".B5+\"4.%%d\"4.%%p\n");
        printf("\"&]-g!nmZh!b_Wt!jVlF!pfuS!u:sF\"$Zk7\"(;8i\"+112\"#pB2!u1n1!mLeK!al(+!_!0J!\\shj\n");
        printf("![\\!4!ZD.R!Ytl/!ZV;u!]U:X!S@Lu!'C7/!7V#0!C?gI!Jgf)!O)We!H8+i!C-^Z!([0a!,)Fa\n");
        printf("!.Y-,!1EtJ!4W)h!D!9N!?_Ee!5\\aZ!&+D/!'C7/~>\n");
        printf("<~!!*6*+pA&m\"ToqP\"![sO,6]7n!<jbO\"Tno1$3~>}U NC} B\n");
    }
    if (!bSingle || strstr(jiestr, "j04 ") != NULL)
    {
        if (bIsSim)
        {
            printf("/j04 {{<~Pop)Yz\"N1G]!D!;@!GDQ@!It7`!La*)!Or4G\"4RDe\"-`mq\"(23Z!D!;@!Z1ql!RLjl\n");
            printf("!Q>(A!NZ;S!JUUN!FGiH!B:(F!>#6I!>kfE!E&oW!JLNa!O)Rb!SRPa!W<$I!Z1ql!n76M!tG>T\n");
            printf("\"#g5J\"&8j/\"(VCg\"*jlf\",m5+\".fLC\"/Z'^\"/Gq&\"/,_A\"-!<L\")%%]F\"%%!#>!tkW+!n[Na!n76M\n");
            printf("!Sda4!uqAu!uq?s!Sd_2!Sda4!Rq+.!RLgo!Yk]O!]g<V!^H`2!e:85!h]O,!h]Ol!h]S8!uq?_\n");
            printf("!uq?'\"'>T&\"'#BM\"&o=9\"&o><\"*am'\"$?Y#!uM*0!TX<P!MBM0!MTXQ!Mfct!Mfc@!Mfba!MTV)\n");
            printf("!MBID!Sd^C!Sd^s!b_VU!b_S8!bD@j!a,MN!^m$;!\\OJ&!XSj[!Rq+.!6>0<!6>/V!6,\"b!5njc\n");
            printf("!<`Bn!<E1N!<<,,!<<,\\!<<3-!?)%%!!A\";f!AjkN!B^F6!Cd-/!E9,<!FZ%%G!G_aY!HJ6r!I+[4\n");
            printf("!HnOH!GV\\W!F>ie!BL;]!<<3A!<<4X!@.c?!5nud!6,,+!6>7G!6>6b!6>0<!0dQg!/(FW!.4jn\n");
            printf("!,;S4!)<TQ!&=Un!%%J%%O!&amJ!($`D!)N_N!+5jf!,r!)!.FuI!/^hq!1!\\D!1<o$!0dQg!!!*T\n");
            printf("![%%T\\!]U:T!_<EE!`B,2!a>ar!bVTp!d4Z,!e^Y;!fd@P!g3Xj!gWq.!g!M<!e:B?!cS7A!`9'5\n");
            printf("![Ilp![%%T\\~>\n");
            printf("<~!!*0*,Qe&L+pA&m\"T]nQ!s'YL!<XML,Qe2r\"!Rmo\"!RmL\"Tno1,m>Lq\"Tnc-,m>In\n");
            printf(",m=AX~>}U NC} B\n");
            printf("/j05 {{<~Pop*fz\"Q0F@!]U9A!Q5's!UKn&!Z2\"^!]U9-!]:&\\!\\a]3![n,[!YPRO!VulB!T4%%3\n");
            printf("!S@It!VHN)!Y#41![%%Q7!X]!N!U^\"a!R(Tp!N?,)!JLR=!FPrX!GheX!N#n]!Rq/T!VQR=!Z(o%%\n");
            printf("!\\sg_!_<BC!ce?J!fmCJ!hKHA!iuG7!kA@D!lP-i!mUj7!mLdK!l+kN!jVlP!g!JC!a#N&!b)5X\n");
            printf("!bh`3!c.ra!pg\";!q$-G!qZPf!rN+A!sAZp!uD\"\\\"#^2Z\"&o<V\",6ir\"3^eY\"4R@q\"24fp\"1&%%*\n");
            printf("\"0qtH\"0_hd\"1/,A\"2\"]2\"0;R&\",$^e\"'u$Q\"%%32[\"#^4.\"\"+/T\"!Ia0\"!dsk\"%%WMR!rrF>!oO/G\n");
            printf("!c.s$!cS6p!gj(_!]U;/!]gFc!]gF0!]U9A!Z1on!Z1q(!rN&b!rN&B\"#L#A\"#0f^\"#'a4\"#'al\n");
            printf("\"'>SS!uM'S!pfrP!Z1s&!Z1sV!^HeE!T4\"j!TF.A!TX9o!TX9G!=/^j!6bJN!6tV*!71a[!71a5\n");
            printf("!71`c!6tT6!6bGY!<`DT!<`Dh!TX7I!TX6:!=8aS!/(>t!)rrG!-eJl!0$t<!8.?A!F,V&!T!f_\n");
            printf("!d\"EW!uq;c\"$?Qm\"'#=r\")%%Zq\"*sqo\",Hq,\"-NXT\".K:%%\"-W_4\"*ss+\"(2,!!uq<N!iuC[!h98?\n");
            printf("!mCYT!q62a!semg!o3j<!f[2>!Z1on!<`FR!TX93!TX7]!<`E'!<`FR!Z1rg!rN(L!rN'!!Z1q<\n");
            printf("!Z1rg!/pt%%!/L[f!6>3>!9sUG!:U$-!>#:Z!@@j(!AX]B!BpP[!CQtr!CQu3!CR!b!Q5&d!Pecp\n");
            printf("!CR\")!CR#0!RLog!K[Cg!Fu9d!CR#D!CR$7!Ghk.!=T(S!=f4*!>#?M!>#>e!([2G!,)HG!.Y.g\n");
            printf("!1F!0!4W+N!>#>Q!>#=B!2'Cd!)`l]!%%7o1!*B:j!+5k/!-SEU!1X+4!5S_f!9aKD!>#=\"!>#;l\n");
            printf("!>#;N!<iN6!:U%%$!87Je!4`.J!/pt%%~>\n");
            printf("<~!!+>I+pA&n\"Tp+U\"![sO+pB.m+pB=r\"T]kN\"XaBU+p0\"m,Qe2P-3XJq\"T]kN!@.aJ\n");
            printf("\"![t#!sKkN\"X4$P\"!RmN+pA;~>}U NC} B\n");
        }
        else
        {
            printf("/j04 {{<~Pop,`z\"Q0F@!:0d*!D!:1!D!9N!:0cG!:0d*!dk),!f-qL\"!@\\:!tYPL!qZQV!mC_Y\n");
            printf("!j2UX!gEcf!dk),!:U(1!@.b(!@.bT!JCP_!JCOl!Or4_!OW\"n!OMr*!OMr:!`/u,!Yba0!UKo1\n");
            printf("!OMrN!OMs!!RLqQ!It:%%!J1EV!JCQ8!JCPs!@.bh!@.c7!B^Ic!:U*;!:g6%%!;$A]!;$A8!'gX.\n");
            printf("!+5n.!-eTN!0RFl!3cQ5!;$A$!;$@c!:g4K!:U(1!g!N:!f@)c!dOm#!aGh\"!^6\\t![7^,!XJkG\n");
            printf("!Yb^G!]^>4!a#O!!cS5a!eLL;!gj&'!jDa%%!f@&,!^m)i!Sd`1!T4#)!]gB>!f6tX!mC_!!rN+#\n");
            printf("\"#p?>\"+UFq\",-e;\".]Kb\"3:O;\"3:OG\")S*C\"!dqg!pfu]!u(gR\"#pAY\"'>Xr\"2\"_,\"+13(\"&oA-\n");
            printf("!gEdl!ji&o!mCbL!o*mZ!g!N:!42i)!2]i@!.4k'!&Ob1!'C=1!+l;&!0I?&!5&C2!Q5';!Pni5\n");
            printf("!P/>Q!NlK;!MTX$!KmLd!It5T!GqmB!E&u)!AFR_!AFRS!GDO+!JLS7!Jge\"!O)VR!QtO(!SRTO\n");
            printf("!U'Su!V$5]!V?H[!Yb_:!Sdc2!PedK!6>6R!7Cri!8mr+!:U(A!42i)!I+[U!K[B(!F,]q!CR\"A\n");
            printf("!:0d>!5&C\"!5&Ba!5&BI!5&B-!5&Af!4i5K!4W)4!:0c'!:0c3!D!9:!D!9*!IOrj!I4a$!I+[:\n");
            printf("!I+[U!6>04!<<-'!<<-?\"%%3/8\"$$AD\"\"aM`\"!@T6!tkT_!rr=?!pBW)!mgpg!i,gA!b;:a!al\"M\n");
            printf("!k85:!os>L!os>,!t5/Y\"\"O@7\"$uuq\"'>PT\")ItM\"+1+X\"/#Z7\"(2.3\"$clH![%%M#![%%M[\"$cm'\n");
            printf("!seq'!osB4![%%Mo![%%NR\"#p=k!rrAo!o*h#![%%Nf![%%OM\"(VH=\"!dq9!rrBB!<`HD!6>3u!6P?O\n");
            printf("!6bJt!6bJ8!6bIQ!6P<k!6>04!8%%;D!5JTV!2BOq!.k3B!+5ef!)`fJ!*0)A!*TA7!+c.;!-eKM\n");
            printf("!/^b]!1j1'!42`T!6G5*!87Fa!9aFP!8%%;D!@.[3!BpM'!DWWl!E&oT!EK2<!F>b8!GhaL!I4Z^\n");
            printf("!J1;p!JUT-!K$l>!J^ZJ!Ib$S!H\\=Z!EfEV!A\"6G!@.[3!Q5!=!T=%%7!V6<*!Vuel!WW4W!Xf!X\n");
            printf("!ZD&n![n&-!\\aV@!\\sbQ!]'h`!\\FDj![%%Km!YPLo!VQNe!R(QQ!Q5!=!<<0,!UKjn!UKj2!<</E\n");
            printf("!<<0,!<</1!UKis!UKi;!<<.N!<</1!<<.:!UKi'!UKhD!<<-S!<<.:!d\"GK!g*KM!i,hE!j2O3\n");
            printf("!k//u!l=r!!mgq9!o3jO!os?c!os?s!os@.!o*e4!mUf2!l\"a/!i,i\"!dk\"_!d\"GK~>\n");
            printf("<~!!+>I!?qUi\"T]nO\"XaBU,Qe2o!sJc1!?q[L,6\\/o\"ToqP,Qe2P!<iQ/\"![sO,6]=p\n");
            printf("\"TncO\"!Rmp!sJ]M\"![sO,6]n++p@un\"TncO\"TncO\"TncN!s'YL!@.aJ,m=AX~>}U NC} B\n");
            printf("/j05 {{<~Pop+Wz\"Q0F@!=T'H!*fVB!.4lB!0dRb!3QE+!6bOI!=T'4!=T&9!&+K\\!)Na\\!,)H'\n");
            printf("!.k:E!2'Dc!Xo/&!R(X&!Ls5t!C-_m!C-`h!V?Ir!OMrr!Jgho!C-a'!C-aG!FQ#*!=/eW!=Aq>\n");
            printf("!=T'r!=T'H!4W*s!3?7_!4`0Y!5\\fS!6>5M!6kSF!7Cq9!8%%@'!)s#M!-A9M!/ptm!2]g6!5nqT\n");
            printf("!=T$G!=T#P!'C<*!*fR*!-A8J!0.*h!3?51!=T#<!=T\"c!=Ak7!=/^b!CQsm!C6b*!C-\\H!C-\\p\n");
            printf("!X&Q5!Q5%%5!L*X.!C-]/!C-^&!V?G0!Or30!Jgf-!E9,N!GMV&!I+[E!J1BY!K.#k!LNr*!N6(@\n");
            printf("!Ghi,!GMV^!EoQ)!C-^:!:0c;!<E7b!<rV$!;lo*!:^-/!87M*!4W*s\",I!E\")7lA\"'>UU\"&K&(\n");
            printf("\"%%WKP\"%%<:.\"%%WLo\")J&R\"#'gJ!t58W!ji%%T!ji&_!o*mJ!dk*o!e(6C!e:B%%!e:Af!e:AR!e:A:\n");
            printf("!e:@u!X&TN![IjN!^$Pn!`fC7!d\"MU!e:@a!e:@>!dk'h!d\"L6!_rfr!\\FJ]!Y>FK!XJk7!]U7O\n");
            printf("!a#M\\!b_X_!a5Y+!^m)F![[sY!XAbk!SRRn!MBId!NZ<d!TjEi!Z(me!^6YY!b;?K!eCD6!gEao\n");
            printf("!jMet!ltEs!n[Pm!pB[f!qZNs!rN*@!sAZa!s/Nr!ql[t!pThu!mUji!i,mR!j2U9!ji$m!ji%%@\n");
            printf("!u(hK!u(gP!uD#h\"!.M?\"!dpj\"#^2S\"&o<P\"*\"@K\"-rtY\"2k5%%\"3:M9\"1SB>\"0_gL\"0_gc\"0_h$\n");
            printf("\"181K\"2Ft/\"0_i#\",I!E!)NZ;!-A2h!0.%%=!:p1[!MTQl!`/l'!n[Kb\"#p9s\"&],%%\")%%[(\"*sr&\n");
            printf("\",d.#\".9-5\"/Go\\\"0MW-\"/Z':\",m5/\"*\"=\"\"#'_P!ltAc!k\\NK!ndRX!r2hd!uq;o!Z1ob!Z1pe\n");
            printf("!rN&J!rN&.\"#L#-\"#0fO\"#'`q\"#'a<\"'>S#!uM&p!qZM(!Z1rK!Z1rW!]U4:!T4!c!TF-?!TX9'\n");
            printf("!TX8l!<`F6!6>1k!6P=B!6bHt!6bHT!6bH4!6P;c!6>/9!<<,8!<<,L!TX71!TX6.!>YZY!0@2&\n");
            printf("!)NZ;!<<-s!TX8X!TX7E!<<,`!<<-s!Z1r7!rN'q!rN&^!Z1q$!Z1r7~>\n");
            printf("<~!!+8G\"XF0R/-?%%V\"!Rmm!sKqP\"Tno1\"XsNt\"Tno1+p@uk\"XaBr\"X4$P\"!RmN,6\\/s\n");
            printf("\"TotQ\"!Rmm!s&KN\"TnoS!sL%%S\"X4$m\"XF0R!@.aJ,QeA~>}U NC} B\n");
        }
    }
    if (!bSingle || strstr(jiestr, "j06 ") != NULL)
    {
        printf("/j06 {{<~Pop'sz\"Q0F@!I+^R!Diki!?hO4!:0c]!4Dr0!-nV[!&O`3!'C;/!0I<a!7q98!=f0]\n");
        printf("!CR\"+!HnPT!Mfg.!QY@f!I+^R!g!M_!^m.3!`fD/!dFeK!ic=1!o!ck\"!I^p\"+1.A\"-*Ep\"0;PI\n");
        printf("\"4RAt\"4RB+\",m:E\"%%E?#!sA[q!l4rh!g*R6!d\"O/!g!M_!8IW0!;lm0!?_F`!Ls32!L*W!!IFiZ\n");
        printf("!E&q2!@Rr^!8%%9o!+5dd!,)?\\!:Ttt!DEKk!IOn@!NZ;i!R(SA!Sd_r!iuGC!hfWg!gWi:!fR,h\n");
        printf("!eC\?\?!ce:!!aYka!_NHK!Zh?$!Sd[B!Sd[2!Zh>a!^m#h!_`SF!eC>8!i>s(!k\\Mj!mq\"V!o<r8\n");
        printf("!osCg!sAZJ!mC^J!i,lO!8IW0~>\n");
        printf("<~!!+8I\"!Rmm!s&KK\"ToqP!s'VK+pA&m\"TotQ\"![sp!t#~>}U NC} B\n");
    }
    if (!bSingle || strstr(jiestr, "j07 ") != NULL)
    {
        printf("/j07 {{<~Pop)Kz\"Q0F@!OMmg!uM(\"!uM''!OMll!OMmg!OMlX!uM&h!uM%%Y!OMkI!OMlX!IOqo\n");
        printf("!Ib(C!It3F!It2!!It0P!Ib#N!IOkq!OMhp!OMk5!uM%%E!uM$.!uM#p!t,*Z!qH>A!n[L'!jr#[\n");
        printf("!f-i4!f-i$!o3ik!sns(!t5/X\"#^-:\"&&\\q\"&&]R\"&&aJ\"*=S5\"#'d)!sAZ\"!PAI.!IOqo!^$Q]\n");
        printf("!B^H\\!F,^\\!H\\E'!KI7E!NZAc!^$QI!^$PF!H8,$!K[B$!N6(D!Q\"ob!T4%%+!^$P2!^$Nt!?_GW\n");
        printf("!C-]W!E]D\"!HJ6@!K[@^\"3^gG\"+10C\"$cp$!cS3S!cS4f\"(VJ[\"\"46_!rN,\\!cS5%%!cS6(\".00Q\n");
        printf("\"&oAY\"!@\\B!cS6<!cS7+!g!Mg!]U;3!]gFt!^$RJ!^$Q]!A\"<-!?;1!!8[ba!4W&o!3-'K!1O\"%%\n");
        printf("!0.(a!.k5T!-SBF!*B8*!%%7kU!%%7kE!)N\\b!,MZq!.4es!/ppt!/ppQ!.4d`!,MXn!-A3U!0dIk\n");
        printf("!42`+!5ee>!5\\_P!5JS`!5AN,!58H[!5&=4!5JU`!6P=7!7Lsa!;$<^!A\"<-!%%\\2q!$hW]!)WfW\n");
        printf("!,_jI!.\"]1!/:On!0[Hn!29N3!3cMK!4W(h!4W)2!4W)P!/UcI!%%\\2q!/L`a!.Y0M!2p!E!5\\h5\n");
        printf("!6tZr!87MZ!9aL_!;Zd/!=JuR!=o8m!<rX*!;lq:!7Ct0!/L`a~>\n");
        printf("<~!!+>I!@.aJ+pB4o+pA&m\"XX<P+p.uo!sKqP\"Yg)_!<XPM\"\"+6Q\"!e$N\"!e$W~>}U NC} B\n");
    }
    if (!bSingle || strstr(jiestr, "j08 ") != NULL)
    {
        printf("/j08 {{<~Pop(.z\"Q0F@!MBJ_!MBNK!Pee2!JCQ&!GDR;!5J\\=!/L`)!/^k0!/puu!/ptM!/ps$\n");
        printf("!/^ed!/LXe!5JUh!5JVg!Ghdi!Ghcn!Mf`q!MKO;!MBIn!MBJ_!5J\\)!Ghj+!Ghgb!5JY`!5J\\)\n");
        printf("!5JYL!GhgN!Ghe(!5JW&!5JYL!=Srb!>GM^!Fu1!!NH-?!TjBd![7X3!^m&T!_`Wr\"$?Uc\"$?S!\n");
        printf("\"$?RX\"\"aM;!tYHs!qH>U!lb5+!fR,L!fR,8!os?(!tYH5!tYG`\"&]+b\"*+BV\")n7=\")n?]\"-<V@\n");
        printf("\"&oB4\"#L+A!`T8`!ZV<P!ZhG,!ZhE\\!ZV86!Z;$c!YPN]!X8[#!Vug=!T3tH!P/9F!L!MB!EfD/\n");
        printf("!=Srb!`T8L\"$?[5\"$?X\\!`T5s!`T8L!`T5_\"$?XH\"$?V\"!`/p5!`B(\"!`T4i!`T5_~>\n");
        printf("<~!!+DK+pB7p\"T]kN!@.aJ\"!Rmm\"!RmN+pB:q,R\"2o!s'VK\"UY~>}U NC} B\n");
    }
    if (!bSingle || strstr(jiestr, "j09 ") != NULL)
    {
        if (bIsSim)
        {
            printf("/j09 {{<~Pop(*z\"Q0F@!Fu;B!DW`[!@S%%Z!:U(=!4W*t!.4jY!'C=A!(6m9!20Iq!=],.!Jgho\n");
            printf("!O)ZJ!Fu;B!aGht!`T8`!j2W*!p9Y9!r`97!u(h3\"\"46/\"#pA)\"%%WL\"\"',K*\"(D>A\")\\1W\"*+Im\n");
            printf("\")n>.\")S,C\"',LF\"\"FC8!r`:)!kADY!aGht!]U9%%!UKnf!R:cJ!M9Fr!F>h2!?;.F!4W';!&+Fe\n");
            printf("!&O^Y!;lkt!L3^=!WW;_!`B(^!mq%%g\"+1-&\"-*DU\"/u=,\"3^eU\"3^ea!tbRe!dk&7!Xo/*!ZV:N\n");
            printf("!]U9%%!pfr8!u(d#!mgtp!iuF$!E9)u!>GRU!>Y]i!>ki(!>kh9!>kgJ!>YZX!>GMf!E9%%m!E9&\\\n");
            printf("!jDZd!jDYa!q61d!pou>!pfpL!pfr8!E9)a!jD]i!jD[#!E9&p!E9)a~>\n");
            printf("<~!!+8I!sKkN!<XYP!<XML\"![sO\"X4$L,m,=p,m+;O,QeA~>}U NC} B\n");
        }
        else
        {
            printf("/j09 {{<~Pop+'z\"Q0F@!<`L0!&t'c!*B=c!.4l>!<`Kq!<`JR!,M_d!/pud!2K\\/!58NM!8IXk\n");
            printf("!U'U#!NZA#!JCO(!Ajl-!AjmL!Yb`-!Rq4-!Mfg&!Ajm`!AjnK!Dim&!<<5O!<NA.!<`L]!<`L0\n");
            printf("!Mfc>!O)VB!Peaj!RLm5!T4#O!Up.h!W`@*!Yb]@!T4$P!Pebe!.4iu!-eR@!,M_4!+Z.K!)s\"k\n");
            printf("!'C<>!$hUe!$D=S!&=T]!(-ef!)s\"%%!+l9F!-\\Je!.Fu5!.4ia!Q5%%U!Mfc>!/psr!3?4r!5np=\n");
            printf("!8[b[!;lm$!OMn2!IOr2!E]C?!/psr!Ajfk!F,Ws!I+Um!Jg`[!LNkH!ModI!O)Q_!P/8s!P\\W1\n");
            printf("!PSQ@!PAEN!NuLS!L<`N!IOnH!F,X;!Ajg&!Ajg>!WW6`!Q5\"`!Ls0e!AjgR!AjhM!GMS2!M'7i\n");
            printf("!RLkG!NZ=_!JUX)!EB0@!>kjO!87I\\!20Fr!,M\\;!,qt/!13eW!6G84!<`Fr!<`F\"!'C9Y!*fOY\n");
            printf("!.Y)4!:0_K!5SZr!/1E=!&ssX!'C6L!0I8,!7V\"Y!<`E'!<`D)!<N74!<<*J!AjdE!Ajfk\"$cq#\n");
            printf("\"$cs)\"(VL]\"\"XP]!sf!j!fR5C!`T9'!`fDN!`oJ%%!`fCU!`T7/!_rgT!^Zso!]C+4!Z;&9!UKk)\n");
            printf("!V?F%%!ZV7o!]^<W!_`Z3!aYqb!c8\";!dFdh!eLL>!f-q*!f-r+!t59Z!t570!t56]\"\"\"(c\"'bnD\n");
            printf("\"-s\")\"1\\JN\"2k7_\"3psn\"4.+\"\"3L\\&\"2b2(\"2\"]*\"1A9,\"0Vd-\"/l:&\"/#^m\".0.^\",[/L\"*Oa7\n");
            printf("\"(D>!\"&o>j\"&&cf\"%%33b\"$cph\"$cq#!d\"Jh\"#p?-!u_3Q!r;q4!n[N-!jVi9!g*Mj!d\"Jh\"4.$]\n");
            printf("\"4.$i\"+gN8\"$-G#!r)d)!tkVm\"\"=7Y\"$?UA\"&8m'\"(2/f\")n;Q\"-<R0\"'>V0\"#L'=![n+0!_<A0\n");
            printf("!aGdP!d\"IB!gNd[!l+gF!dF^M!ZV2T!NZ8\\!O)PP![7V:!ep]3!o*e:!u(a*\"&&]+\",$Y>\"-`dh\n");
            printf("\"0DQ:\"4.$]~>\n");
            printf("<~!!+GL\"Y9`Z!<XML,m,Cr+p.iJ\"XF0N,m>Ut\"ToqP!sKtQ\"ToqP+p.im\"![sO,6]7n\n");
            printf("-j9VR+p@uL,6]Cr+pA&n\"UY~>}U NC} B\n");
        }
    }
    if (!bSingle || strstr(jiestr, "j0a ") != NULL)
    {
        printf("/j0a {{<~Pop(^z\"Q0F@!MBNK!>GVi!8%%BM!87N/!8IYC!8IX3!8IW\"!87J+!8%%=N!>GRM!>GRq\n");
        printf("!H8)#!E]B(!Aab\"!<N9g!71`V!13cB!*B6*!+Z)*!/pog!4`*]!:0^b!?M7e!D!5`!GDLQ!Jgb;\n");
        printf("!O;_3!U'O9!KmGO!;un4!%%7h<!%%\\+0!<rNu!N6!D!Y>@E!dF]m!q$%%b\"*=N&\"*jlE\".9.&\"4R<q\n");
        printf("\"4R=,\"$lp;!k86!!]U24!bq`9!g3R.!j2Pf!m1OI!pBZ$!sepL!l+iL!gj\"M!It34!K@,N!La%%g\n");
        printf("!N6%%+!R(SS!K[?C!n[P7!n[Oh!u(dg!tbS?!tYMb!tYN%%!tYP?\"\"XNs!q6:g!mh$'!OMq_!Rq3R\n");
        printf("!U^&1!X&UU\".TID\"&oBH!uqE-!)*KZ!,MaZ!/(H%%!1j:C!5&Da!OMrZ!O2`I!NH6(!MBNK!>GVU\n");
        printf("!n[Sp!n[Rm!>GUR!>GVU!>GU>!n[RY!n[QZ!>GT?!>GU>!>GT+!n[QF!n[PK!>GS0!>GT+!I+Wm\n");
        printf("!g!G1!bqa-!^6W)!Xo)$!RUoc!MBH_!I+Wm~>\n");
        printf("<~!!+8G+pB1n+pA&o\"TotQ\"![so!sKtQ\"XO6S,m+;Q\"T]kN!@.aJ,Qe&L+pA;~>}U NC} B\n");
    }
    if (!bSingle || strstr(jiestr, "j0b ") != NULL)
    {
        printf("/j0b {m08} B\n");
    }
    if (!bSingle || strstr(jiestr, "j0c ") != NULL)
    {
        if (bIsSim)
        {
            printf("/j0c {{<~Pop*Xz\"Q0F@!V?IV!CR#P!Fu9P!IOtp!L<g9!OMqW!V?IB!V?Ht!V-<O!Up0+![Ij\"\n");
            printf("![Ijr!jDbT!jDaA!osF8!oX4O!oO.i!oO//\"2\"^a\"*aoa\"%%WMV!oO/C!oO0>!sA_-!iuLJ!j2X,\n");
            printf("!jDcS!jDbh![Ik1![Il0!_<Et!Up3@!V->t!V?JC!V?IV!K[?O!XJhr!WW7<!S@DQ!K[;[!K[?O\n");
            printf("!E]=U!K[:T!K[;G!SRP$!Y,5V!\\=A2!^$Kq!_*2_!_`VQ!`8tB!a>[J!bq`k!dF`5!dXlH!cA$N\n");
            printf("!b)1S!_i]P!\\aYF!\\see!]:#/!]U5M!i,kd!i?!8!e::K!\\aVE!^$II!d4R[!i,i\"!lP+E!p0MA\n");
            printf("!r`39!t#&)!u:mn\"!Ra#\"\"jTI\"$-Gn\"#p<,\"\"41.!uM&/!r)e!!ltCY!mgt<!n77l!n78?\"&o<j\n");
            printf("\"&o8n\"&o8V\"%%`K?\"#L\"*\"!.Gi!rW+J!mgq#!mgph!tkTE\"#U'M\"#p9+\")S#r\",Hq[\",Hr9\",I!9\n");
            printf("\"/l7q\")%%`a\"&&b!!n78S!n7:%%\"4.+\"\",m<\"\"'bnl!A\";:!DEQ:!Fu7Z!IY$\"!LNq=!XJjX!XJi1\n");
            printf("!L*Wg!E]CK!EoNj!F,Ym!F,XT!F,W:!EoJ:!E]=U!]U73!i,mJ!i,l#!]U5a!]U73!A\";r!>kmb\n");
            printf("!8R\\S!4Doa!2KX9!0I:d!/(AK!.FrD!-\\H;!*fP\"!%%\\.M!%%\\.=!)rtZ!,qri!.Y(k!0@3l!0@3J\n");
            printf("!.Y'Z!,qpi!-\\EP!1!Uc!42_u!5nk3!5nkF!5nkX!5ef$!5\\`S!5JU,!5nm\\!6tU;!7q6m!;6Hc\n");
            printf("!A\";r!&+Ji!%%7oU!*')O!-/-A!.Fu)!/^gf!1*`f!2]f+!42eC!5&@`!5&A*!5&AH!0%%&A!&+Ji\n");
            printf("!/q#Y!/(HE!3?9=!6,+-!7Crj!8[eR!:0dW!<*''!=o8J!>>Pe!=Ap\"!<<42!7h7(!/q#Y~>\n");
            printf("<~!!+8G\"TnoS!sL%%S\"XF0R!<WK-!?qUm\"Tno1-3YRq+pA&m\"Y0ZY,Qf4o!@.aJ\"![sO\n");
            printf("-NsMQ,R\"2N,R\"M~>}U NC} B\n");
        }
        else
        {
            printf("/j0c {{<~Pop*Tz\"Q0F@!GDHQ!MBET!MBK:!b;?T!b;<8!b).?!al!j!h96]!gs%%-!giut!gj$3\n");
            printf("\"'>UI\"'>Pb\"'>PJ\"&Ju6\"$cj&\"#'^k!tYHO!oO''!oO&l\"!%%A>\"$?QC\"$?Q#\"*\";h\",m4S\",m59\n");
            printf("\",m9i\"0_hH\"*alH\"&o=Y!gj$G!gj%%n!oO-a!oO-1!uM*$!u1m9!u(gp!u(hs\"5Eu<\".TI<\")J'5\n");
            printf("!u(i2!u(j%%\"#L+Y!o*n)!o=$Z!oO00!oO/S![%%S=![%%T4!^$Rh!U'X4!U9cm!UKoC!UKn^!B:0T\n");
            printf("!DikP!H8-#!UKnJ!UKmG!U9`Z!U'T0![%%Q#![%%QK!b;A:!b;?h!N6&V!GDO6!GVZ=!Ghe4!Ghcn\n");
            printf("!GhbS!GVUH!GDHQ!oO-u![%%Q_![%%S)!oO/?!oO-u!Q5$F!Or12!RLl$!Ta?l!Vc\\_!Ta?!!QY:%%\n");
            printf("!MBGj!N6\"j!R^ud!VHI[!Y>BO!ZD)>![7Y+![n'j!\\FES!]^8]!_`V5!aYm`!aPh)!_<?8!\\seF\n");
            printf("!X8\\J!Q5$F!k\\R;!jD_#!m1Pt!o3mi!pBZT!nmZm!l+gs!giuc!i,hc!mq#k!q?:_!sAX@!u(c(\n");
            printf("\"!.Ii\"!dm]\"\"=6O\"#L#\\\"%%3/.\"&o:T\"&].p\"$cm/\"\"aPA!s&GE!k\\R;!H\\DH!Fu9<!<rT<!71b*\n");
            printf("!5\\b[!4)]5!2]cq!1Epd!0.(V!,qs:!'gQe!'gQU!,)Br!/(A,!0dL.!2KW/!2KVa!0dJp!/(?)\n");
            printf("!/pne!3?0&!6bF;!8@KN!87E`!8%%9p!7q4<!7h.k!7V#D!8%%;p!9+#G!:'Yq!?;.9!H\\DH!(6n,\n");
            printf("!'C=m!,2Lg!/:PY!0RCA!1j6)!36/)!4i4C!6>3[!71d#!71dB!71d`!20IY!(6n,!2'Fq!13k]\n");
            printf("!5J\\U!87NE!9OA-!:g3j!<<2o!>5J?!@%%[b!@It(!?M>:!>GWJ!9sZ@!2'Fq~>\n");
            printf("<~!!+;H\"TnoP\"!RmN+pBCt\"XaBU,Qe2q!sL\"R+p@um!s&KJ\"Tp%%S!<XML\"!n*O\"![sO\n");
            printf("-NsMQ,R\"2N,R\"M~>}U NC} B\n");
        }
    }
    if (!bSingle || strstr(jiestr, "j0d ") != NULL)
    {
        printf("/j0d {{<~Pop'[z\"Q0F@!&Oa\"!)s\"\"!,M]B!/:O`!2KZ)!;lm,!;lhe!8%%:%%!?;)-!B^@$\"*=N^\n");
        printf("\"\"4/^!pfo?!B:(4!B:,h\"3:NP\"*alT\"#p?)!\\aZu!`B(_!a5Y.!_<B7!]:%%>!X]\"6!QY>t!Pec`\n");
        printf("!T4$Z!V?GP!W3\"B!X&R3!XSop!Xo,Q!&Oa\"!iuJ@\"3:Q]\"*ao]\"$cr>!iuJT!iuKO!n7=>!cS7[\n");
        printf("!ceC-!d\"NQ!d\"Mq!Ls6C!Ls7F!Q5)5!FQ#N!Fc.r!Fu:@!Fu9`!&Od/!)s%%/!,M`O!/:Rm!2K]6\n");
        printf("!Fu9L!Fu7j!Ls4i!Ls6/!d\"M]!d\"L2!iuI1!iuJ@~>\n");
        printf("<~!!*0*/d!:$\"!RmN!@IsQ,Qe2o!sL(T$3~>}U NC} B\n");
    }
    if (!bSingle || strstr(jiestr, "j0e ") != NULL)
    {
        if (bIsSim)
        {
            printf("/j0e {{<~Pop(Nz\"Q0F@![%%Q[!h]V1!h]SD![%%Nn![%%Q[!n7:e\"(22G\"(2/Z!n78#!n7:e!U'Q'\n");
            printf("![%%N\"![%%NZ!h]S0!h]Q?!hKC^!h969!n[K8!n@9o!n75F!n77d\"(2/F\"(2._\".0+V\"-ioK\"-`jQ\n");
            printf("\"-`kg\"1SEN\"+UIN\"'>WS!n7;$!n7=2!r)kr!h9AF!hKLo!h]Wp!h]VE![n-\"!U'U_!U9a3!UKlH\n");
            printf("!UKkE!UKjB!U9]S!U'Q'!;HOJ!AjdM!AORr!AFNc!AFQt!EoO+!I\"S'!JUWf!L*VN!MTUX!O)U-\n");
            printf("!PJNV!P/<k!N#nk!KmKk!GVZV!AFR/!AFSB!Rq1<!K[B<!FPu1!AFSV!AFU`!FQ\"C!K[D$!PeeZ\n");
            printf("!JCQ^!FZ)%%!AXb8!;HYB!5/JJ!.k;V!([2g!)*J[!/q\"K!6,+8!;lq$!;lo\"!&+JU!)N`U!-A:0\n");
            printf("!;$>[!9=2e!6P?_!2]eH!.k61!*B7m!%%7jR!&+EN!.k5@!6,&6!;lm0!;ljg!;Z\\o!;HOJ~>\n");
            printf("<~!!+>I!@.aJ,6J)O\"X=*Q-NaMr\"!RmL\"!n*t!sJcN\"TnoS\"!RmN+pA;~>}U NC} B\n");
        }
        else
        {
            printf("/j0e {{<~Pop)Sz\"Q0F@\")n<P\",m;+\"'>W+\"#p@@!k8;j!k8=,\"2k99\",$b9\"'>X6!k8=@!k8>S\n");
            printf("!t>?[\"&&fJ\"*=Wu\"$?\\$!u(jA!oF*R!hKLY!aGh^!Ykfb!QY@b!QY@R!U'Vr![\\\"_!e^Yp!e^Xa\n");
            printf("!K7*h!NZ@h!RLoC!e^XM!e^W6!XJjd!R(VD!R:b&!RLm@!RLl<!RLk7!R:^N!R(R,!WW7#!WW7;\n");
            printf("!e^Sj!e^RS!MBGn!Pe]n!TX7I!e^R?!e^Pi!GhaP!K7\"P!Mf]p!PSP9!SdZW\"4.$M\",$ZM\"&Ju6\n");
            printf("!k85H!k86s\"/l4d\"(VEd\"#L#Y!k872!k88I\"$?U#\"$?TT\")n9G\")n<P!WW:H!e^W\"!e^U\\!WW9-\n");
            printf("!WW:H!k8;V\"$?X0\"$?Vj!k8:;!k8;V!WW8n!e^UH!e^T)!WW7O!WW8n!k8:'\"$?VV\"$?U7!k88]\n");
            printf("!k8:'!MBO.!GDS2!C?mI!>Yc_!9=5u!3lW4!.Y/N!)Nbk!)s%%_!,_m&!1a3[!8mrQ!8mpO!'gUY\n");
            printf("!+5kY!/(E4!8%%@3!6P@?!4;k@!13f4!.\"['!*0+k!%%\\-V!&O]R!.4f:!4;iq!8mnM!8ml$!8[^5\n");
            printf("!8IQ.!>kf9!>PTX!>GPQ!>GT#!B'uj!DEOX!E9*@!F,Z(!Fu4s!H%%q&!I\"R,!It3>!K$o^!L!Q'\n");
            printf("!KR99!Ib(?!GhfC!D*>A!>GT7!>GTo!NZ?]!Ghh]!C-^Z!>GU.!>GW8!DET%%!I=iZ!MBO.~>\n");
            printf("<~!!+PO\"ToqP!sL(T+pBJ!\"YKlX,Qe&m!s'YL!@.aJ\"!RmN\"XO6p\"ToqP\"\"\"0u!sK#~>}U NC} B\n");
        }
    }
    if (!bSingle || strstr(jiestr, "j0f ") != NULL)
    {
        printf("/j0f {{<~Pop(&z\"Q0F@!/pte!4W(=!71cb!;HU@!AFR+!GDNk!MTWV!T4#E!T3up!/pqp!3?2p\n");
        printf("!5nn;!8[`Y!;lk\"!T3u\\!T3s&!&+Ct!)NYt!,)@?!.k2]!2'=&\"3^aU\"+1*U\"$cj2!ZV2b!ZV5C\n");
        printf("\"*=QG\"!doG!pBZ$!ZV5W!ZV80!`9\"m!gs*i!qZP%%!so#o!u_4c\"\"43X\"#U,K\"%%*+U\"&]0t\"(20=\n");
        printf("\"(hTU\"(VHi\"(;7&\"&f8/\"$-L0\"!@Z/!mC_e!^$Q%%!]0ub!e(4!!kAB+!osE)!]^<o!KmLh!:U%%h\n");
        printf("!@7eu!Drot!H\\Ce!L<fT!QbEi!Xo0M\"2G\"X\")J(T\"#'h1!*B>V!-eTV!0@;!!3--?!6>7]!O)ZF\n");
        printf("!K$t7!Fu93!BpS9!>bg=!;HVW!9+'1!6YF_!3QB7!/pte~>\n");
        printf("<~!!*0G\"X=*Q,Qe3\"\"\"\"0R,R#Ir\"TotQ$3~>}U NC} B\n");
    }
    if (!bSingle || strstr(jiestr, "j10 ") != NULL)
    {
        printf("/j10 {{<~Pop(jz\"Q0F@!AFV3!mCb&!mCa3!AFU@!AFV3!AFU,!mC`t!mC_u!AFT-!AFU,!MBI(\n");
        printf("!o*fe!o*eZ!Ghc>!Ghd1!MBI(!Ghc*!o*eF!o*d7!Ghap!Ghc*!;$>O!AFSN!AFSn!QY>\\!QkJ6\n");
        printf("!R(Uj!R(UM!5nqD!9=2D!?_G7!R(U9!R(T6!'C;O!*fQO!-A7o!0.*8!3?4V![%%O!!Smd`!M0=T\n");
        printf("!Fu4U!AFPA!AX\\+!Ajgp!Ajgf!:0_U!0R@3!%%7jV!&+EN!/LXc!8[`%%!Ajg>!Ajf;!AXY4!AFL-\n");
        printf("!Gha4!Gha\\!o*d#!o*c4!uM#;!u1fZ!u(aZ!u(c8\"#p<p!qZMd!mC[i!QY:d!Y>C$!_i^.!e:=,\n");
        printf("\"3:Mu\"*akq\"#p>N!gE`T!kn^F!pftB!uqAE\"&&cH\")\\1&\",$`7\"$?Y7\"\"jYc!tGBu!nm]j!i?#_\n");
        printf("!ce>W!^HeU!X&Pn!X&Qq!i,lg!c.pk!^Hfl!X&R0!X&R\\!\\=DG!Up0/!mC_a!mC_5!setD!sJba\n");
        printf("!sA]G!sA^J\"\"XP5!pg$-!lP22!B:1O!;$B3!;6MT!;HY$!;HXI!;HWn!;6K6!;$>O~>\n");
        printf("<~!!+>I!@.aJ,m+/n!s'VK\"XaBU!sJcN\"ToqP,m+;r!sKqP+pA&m\"Y9`Z,m,=p$3~>}U NC} B\n");
    }
    if (!bSingle || strstr(jiestr, "j11 ") != NULL)
    {
        printf("/j11 {m07} B\n");
    }
    if (!bSingle || strstr(jiestr, "j12 ") != NULL)
    {
        printf("/j12 {{<~Pop)5z\"Q0F@![n-B!Z2\"2!Y>F1!W<(=!T4#W!Q\"mo!Or1P!Q\"mP!QtNN!SRSZ!U^!r\n");
        printf("!WiE5!Y>DS!ZD,&![@bL![n,9![n-B!iuK_!mCbJ!c.t_!cA)u!c.qJ!bML3!abuo!`B&e!^6Wi\n");
        printf("!\\+3l!Y#.o!U'Nr!Q\"hs!K@(g!D!2M!DEJA!JpeC!Pe\\H!Up)N![%%KS!_!+W!al$Y!dXlZ!fd<$\n");
        printf("!h9<c!i?\"l!k&-2!m1O[!o<r/!qZKd!tG=T\"\"+)B\"%%iQ8\"*af6\",[(e\"/u9<\"4R<e\"4R<q\"1/&^\n");
        printf("\"-W_R\"*+CM\"&K!G\"#'`R!tkVo!qQG5!nICd!k\\RS!sSg+\"%%E?K\"+10_\"/H\">\"'bp:\"$Qe6!sesb\n");
        printf("!k8:g!iQ0/!i,nI!iuK_!C-\\`!Fc)b!IOpV!K7&<!Ls1!!NZ<*!PAGV!R(S-!QbAD!OMmI!M0>L\n");
        printf("!I+Y?!C-\\t!C-^B!TX<<!MBM<!H8+1!C-^V!C-`d!I+]T!Mom4!QY@Z!K[Df!HJ:2!C?mA!<<4>\n");
        printf("!5/J9!/L_J!+5mo!+Z0c!.4l(!3u\\h!=/d$!=/as!'C=Q!*fSQ!.Y-,!<<1W!:9hV!7CoK!3cL8\n");
        printf("!0%%##!+Gs_!&+EB!&O]6!/pq1!7Cn-!=/`,!=/]`!<rOl!<`BR!CQoY!C6^#!C-Y_!C-\\`~>\n");
        printf("<~!!*0J\"T]eL,R\"8t\"Tp\"R+p0\"m!@.gq!sJcN\"TnoS\"!RmN+pA&P$3~>}U NC} B\n");
    }
    if (!bSingle || strstr(jiestr, "j13 ") != NULL)
    {
        if (bIsSim)
        {
            printf("/j13 {{<~Pop(Lz\"Q0F@!>#=J!QY>X!O)VW!K@,e!FPq-!AFPB!=Akt!:U%%p!;ZbE!<iOq!>#=J\n");
            printf("!]0tk!Up0k!Q5&h!>kmf!A+Bk!B^HQ!Cd/n!D`f4!EfMN!Fu:g!=/e3!<iRW!;$@G!7V(W!42ef\n");
            printf("!/(Bp!(6iu!)ru$!/prI!5&?s!9=2L!:BmM!>#9d!D!5:!@e*K!<rPV!87FY!3Q<[!-nQX!'C6P\n");
            printf("!(6fL!/pnY!65(^!;6D[!@.ZV!D*:H!Fu3.!L<`*!S%%1A![%%Jr!bq^L!oa2^\",HqR\"-!:t\"/Z'O\n");
            printf("\"4R=8\"4R=D!snsr!dk!D!\\OJe!T*n/!N#kl!JCIr!N#m2!Pnf<!Rq/<!TjG:!Vle5!Xo.+!]0tk\n");
            printf("!k\\T1\"!I_K\"(20O\"*t\"?\"-Wc-\"/Q%%>\"0_gq\"1eON\"0)Da\"+C;T\"&]2F!tbU%%!k\\TE!k\\VO!o*m6\n");
            printf("!e:B_!eLN!!e^YF!e^Y$!e^Sr!e^S<!eLF[!e::'!l+g.!keUX!k\\P-!k\\PU!k\\T1~>\n");
            printf("<~!!*0H\"T]hM+pA&m\"Tp\"R\"!n*Q,R\"8N,R#Cp\"Tno1\"To)~>}U NC} B\n");
        }
        else
        {
            printf("/j13 {{<~Pop*\\z\"Q0F@\"#'eD\"$cpP\"(22C\"*amt\",[0<\".KAW\"0;Rl\"2\"^%%\")n?!\"&K(*!V?I\"\n");
            printf("!V?Ij\"%%35f!rrFf!mh$[!V?J)!V?JQ!ZV<@!Or6]!P/B/!PAMB!PAL?!9=4f!2'EF!29P7!2BU,\n");
            printf("!29N#!2'@m!13df!/^da!.+^[!*TAI!%%7h,!&O[0!,_dU!1*\\j!3Q=o!5nmr!7:gt!7V%%t!7h2r\n");
            printf("!8%%@+!8%%AF\"&o?o\"#'eD\"&&cP\"$?X@\"$$Er\"#L'R\"\"XL5\"!dpl!u1kS!rrB>![%%O]!WiE?!V?F=\n");
            printf("!V?FU!V?G,\"!@Yu!o*ji!jD`f!V?G@!V?Gp!Z2!W!PAL'!PSW`!Pec:!Peb]!9=2l!=/`p!AFRS\n");
            printf("!PebI!Pea^!Pea;!S.;@!X&Pn\"#'cF\"&K$h\")%%`;\"*aki\"'ksS\"&AtS\"&8ni\"&&c(\"&&c=\"&&cP\n");
            printf("!JCHs!D*9O!<<+'!2olO!3cGK!>PT+!G2=T!Mf^s!S.7-!Xf!I!^Zlp!dF]A!j2Mm!oa2H!u:l#\n");
            printf("\"&T%%\\\"-`dJ\"-ER\\\"/Z'/\"4R<m\"4R=(\"-*A,\"#U(&!lt@j!a5SW!Wi@n!Pe]Z!U9[_!Y#/f![n(o\n");
            printf("!_`WR!Xo+N!UKi[!GDM,!H8(V!It4%%!LNoC!D!8'!C[%%N!B:+b!?_Db!=/]a!9aF^!5JTY!6bG]\n");
            printf("!:^'O!>PVI!B:*J!E&q,!Ghbs!JCHs!CQrj!FPqe!Up,K!T3uX!Q5!Z!Ls/R!IOnJ!F>dQ!CQrj\n");
            printf("\"3:K#\"1S?m\"0MXn\"0;M'\"/u;4\"/l5L\"/l5o\".0*_\"-im6\"-ETk\",m6T\",6g;\"+:1)\")n7q\"&&^M\n");
            printf("\"#L#5\"\"403\"\"40I\"\"41t\"&&`[!tYLO!q65`!iQ-m!b_VM!c7t\"!c.mD!b;<]!aGa!!^Qh/!YbX5\n");
            printf("![%%K9!a5T@!e(.2!f@!a!gWj:!hB?m!h]RQ!r)eT!r)cZ!r)c7!t>7;\"#p:f\",m5e\"/Z(*\"1eKL\n");
            printf("\"3:K#~>\n");
            printf("<~!!*0G\"Y0ZY+p0%%n\"![sn!s&KJ\"TnoV!sKtQ\"TotQ!<iQO\"TotQ,Qe2P+pA&m\"T]eL\n");
            printf("+p@uk\"ToqP!sL\"R+pA&m\"X4$P!sK#~>}U NC} B\n");
        }
    }
    if (!bSingle || strstr(jiestr, "j14 ") != NULL)
    {
        printf("/j14 {{<~Pop''z\"Q0F@\"&o?G\"+112\"$?Z2!tYP/!JCOL!R(XD!W`CW![Il0!QYAe!PJT*!M'<m\n");
        printf("!GDQ0!;HWj!5&CJ!58N)!5JX[!5JW6!5JUe!58H<!5&:g!;HOf!;HPi!uq<:!uq;+\"'>P&\"'#>U\n");
        printf("\"&o;(\"&o?G!;HWV!uqC'!uq@.!;HT]!;HWV!;HTI!uq?o!uq<N!;HQ(!;HTI~>\n");
        printf("<~!!+>I\"TnoP\"!Rmp!sJ]N!s'YL$3~>}U NC} B\n");
    }
    if (!bSingle || strstr(jiestr, "j15 ") != NULL)
    {
        printf("/j15 {{<~Pop*Rz\"Q0F@!Ls3*!Or1Z!JCMR!GDNk!42ea!.Y,I!.k8(!/(C[!/(C6!/(Be!.k6>\n");
        printf("!.Y)l!42c_!42cs!=/^r!=/[q!42`b!42b<!7V$#!.Y)H!.k4T!/(?f!/(?&!&st#!+5d`!,qp3\n");
        printf("!9sQ/!RLhV!R(Pb!B:(X!B:*.!Pe^a!K[=e!H8'%%!B:*B!B:+M!Ghe,!Ghda!MBIX!M'7j!Ls2?\n");
        printf("!Ls3*!42eM!Ghf[!Ghe@!42d2!42eM!_<A,!sAZ>!oX1B!l4oU!i,ju!e^Tm!bDDp!_<A,\"#p>b\n");
        printf("\"'bmA\"!dqA!sAZR!`/qH!b;@#!e^VO!]U7/![7[p!WiDZ!S@FC!TX9G!W2u$!Z1sf!]U5a!`/pK\n");
        printf("!bqb?!f-l=!_!,f!U]t2!JCIJ!JgaB!Q>'7!VQO$!ZV4`!ZV3_!ZD&`!Z1nc!_`SV!_`T5!uq;[\n");
        printf("!uq:p\"&Jtc\"&/bu\"&&]Z\"&&^e\")%%]@\"#'a8!u(bU!`/n;!]U33!bDC#!fR.c!iuEI!o*fY\"\"=73\n");
        printf("\"-`g/\".90L\"0DSp\"4.'F\"4.'R\"-N[i\"'kq:\"#'an!s/LL!oO*6!ltD,!q66$!u1jn\"#p>b!_`V#\n");
        printf("!uq=I!uq;o!_`TI!_`V#!Yb_^\"(23V\"#L)/\"%%347\"*\"DG\".TH7\"2k9]\"+UJ]\"'bpf!Yb_r!Yb`m\n");
        printf("\"+UK0\"$?\\0!t5:%%!0dSU!42iU!6bOu!9OB>!<`L\\!TX?=!TX>B!3cPb!3?9-!2'F!!0RF0!/(FR\n");
        printf("!-A;1!+Z/d!+,fS!+l;T!,M_S!-\\L[!/:Ql!0dQ&!2'D9!3-+O!4)ad!4Dt,!42hR!TX>.!TX=K\n");
        printf("!TF0^!T4#i!Z1uh!Ykd(!Yb^]!Yb_^!:0dJ!:0d2!K[B,!K[BD!:0dJ!9aKG!9aK3!L*Y5!L*YI\n");
        printf("!9aKG!b_Yr!b_YZ!setP!seth!b_Yr!b_Xs!b_X_!sesU!sesi!b_Xs~>\n");
        printf("<~!!+AJ+pB@s\"X4$P.g#qU,Qe&L+p@up!sJcO\"ToqP,m+;s\"![sO,6\\)n!s'VK\"XsNW\n");
        printf(",m,Fs!sJc1!@.aJ,Qe&m!s'YL$3~>}U NC} B\n");
    }
    if (!bSingle || strstr(jiestr, "j16 ") != NULL)
    {
        printf("/j16 {{<~Pop)ez\"Q0F@\"#pAG\"%%34K\")\\2X\"-s$G\"2\"^m\"+12u\"&oA%%!XJm-![Rqb!\\FM+![%%T4\n");
        printf("!YPU;!V-?0!Pefi!Or6Y!S@LV!U9c?!Up1j!2'En!2'FA!0@;1!/1MH!-//e!*B=3!'LDU!'(,@\n");
        printf("!)*IJ!+#`R!-/.j!/:R9!1Eu]!29Q.!2'EZ\"'bpn\"#pAG!E]E9!/L]h!2osh!6bMC!E]E%%!E]Ck\n");
        printf("!1X*Y!5&@Y!9=28!E]CW!E]BD!&O_t!)rut!.4gS!E9*,!C6a>!?M8>!9sS+!4Dmm!-eLW!&+D?\n");
        printf("!&st;!0mPh!8mk4!>khI!Die]!IOok!Ls1t!a#K1!ce<`!gE^N!kn[R!p9RT\"!R`j\")n7A\"+UBs\n");
        printf("\".9/J\"2\"Wp\"2\"X'\"+C7F\"$lqp!sAWO!lb7-!gE_*!c.nE\"2FrE\"*akE\"%%312!e^Tq!e^V/\"%%WJI\n");
        printf("!sesI!o*iF!e^VC!e^WR\"%%33h!sA\\h!n[Re!e^Wf!e^XA!h]W$!_`\\E!_rh9!`/sl!`/s2!K7)m\n");
        printf("!K7*H!Mff#!E9.L!EK:C!E]F!!E]E9!K7)Y!`/rs!`/qd!K7(J!K7)Y!K7(6!`/qP!`/p=!K7'#\n");
        printf("!K7(6!C-Y?!B^A/!L*Sl!S7=A!X8XX!]0mn!a,M'!d\"E-!fd71!hKBB!i,f_!iZ0%%!i>s6!h'+:\n");
        printf("!fd8>!d\"F2!_`Tk![IcO!QkD\\!C-Y?!NZ<H!N6$8!S@EH!VugJ!Y,5>![7X2!\\sc7!^6VN!_NId\n");
        printf("!_rb\"!_`V3!_EDB!^$KF![Ie=!Xo*3!TX8p!NZ<H~>\n");
        printf("<~!!*0*,6K+n!sKnO,R#@o!A4Hu\"ToqP\"![sO+pB\\'\"XF0R!@.aJ,Qe&L-3XDP,m=V~>}U NC} B\n");
    }
    if (!bSingle || strstr(jiestr, "j17 ") != NULL)
    {
        printf("/j17 {{<~Pop)[z\"Q0F@!:0a1!&O`#!)s!#!,M\\C!/:Na!2KY*!8%%=^!3cK+!-/)8!$D93!%%7i/\n");
        printf("!-eLj!4`+G!:0_s!:0^H!9sQ1!9aD2!@.Y-!?hGt!?_C)!?_DJ!Die_!H\\>d!KI0\\!N,qR!Oi'c\n");
        printf("!PSR9!Q5!c!P&4p!Ls0a!Ib&P!EB/3!?_D^!?_EQ!T4!g!Ls2c!H8(`!?_Ee!?_Fh!B^EC!9aJh\n");
        printf("!9sVB!:0ai!:0a1!Up(;![n%%:![n%%n\"\"XFs\"\"XF+\"(VC&\"(;18\"(2,j\"(2/f\",$^M\"%%WJM!uq@J\n");
        printf("!\\aZ]!Up.A!V-9J!V?DR!V?CV!V?BY!V-5P!Up(;![n*A\"\"XKF\"\"XJ3![n).![n*A![n(o\"\"XIt\n");
        printf("\"\"XHa![n'\\![n(o![n'H\"\"XHM\"\"XG6![n&1![n'H!`T5S!`T5?!tYNQ!tYNe!`T5S\"\"XN#\"$cq/\n");
        printf("\")%%c'\",$aY\"-`lp\"/H#1\"1SFI\"4.,a\",I%%e\"(VKn!ZV;%%!ZV;q\"'>YY!tYRU!o*mF!5nu,!9=6,\n");
        printf("!;lqL!>Ycj!Ajn3!TX?9!TX>B!5J[r!5J\\A!3cQ1!29Q:!0@9W!.\"_1!+Q)_!*THE!+#`8!+H#*\n");
        printf("!,_k0!.k9H!1!\\`!2Tb&!3cOG!4i6f!5J[6!5J[^!TX>.!TX=8!TF0:!T4#5![%%P8!Z_>O!ZV9C\n");
        printf("!ZV:f\"(VKZ\"\"XN#!:0d>!:0d*!N6(<!N6(P!:0d>!`/sN!`/s:!setH!set\\!`/sN!:0c?!:0c+\n");
        printf("!N6'=!N6'Q!:0c?~>\n");
        printf("<~!!+8G\"Tno1+pA&p\"XjHV!@7gO,m,=p!@.aJ,Qe&m!s'YL!<XML.0B`\"\"!n*Q\"TnoP\n");
        printf("!s'YL!@.aJ,QeA~>}U NC} B\n");
    }
    if (!bSingle || strstr(jiestr, "j18 ") != NULL)
    {
        printf("/j18 {{<~Pop)Kz\"Q0F@!_<E$\"!@\\R!qcX<!m1T/!h]V-!dXq)!a#O3!]U9I!^-WX!^Zuh!_<E$\n");
        printf("!e:BO!]1#/!\\OS^!Z_Ar!WW<j!TF1a!PJQW!K[AM!LNqI!R_%%[!X&So!\\=F-!^m+b!au/O!e^WJ\n");
        printf("!]C*`!TaA/!K[?_!LNoW!Vuj/!`T4d!i,mJ!pftZ\"$-K)\"-<Qe\".]K7\"181a\"5!Z7\"5!ZC\")7m2\n");
        printf("!t><H!l+l1!pp'3\"!@[K\"'>Y!\"+12Y\"%%36Y\"!@\\f!`T8D!b;Cl!ceC6!e:BO!ltBB\"3^cG\",m7G\n");
        printf("\"(2-D!ltBV!ltDT\".0,%%\"'>U%%\"\"XK\"!ltDh!ltEC!pB\\*!g!IO!g3U6!gE`f!gE`4!Or0W!S@FW\n");
        printf("!Up-\"!X\\t@![n)^!gE_u!gE^\"!S@De!U9\\G!Vc[n!WW7/!ZV5[!S@FG!S%%4.!Q>(J!N6#E!Jgaf\n");
        printf("!PAEj!Rq,M!gE]c!gE\\U!g3O`!g!C1!mCX,!m(FA!ltA,!ltBB!3cK3!9aGS!=AiY!>khG!DNS?\n");
        printf("!H/!&!Ib&Q!K7&&!L!PL!KmJn!K[?9!JCLO!GhfY!E9+b!Ajje!=T$c!IOu[!Ls7:!Fu;:!C-aG\n");
        printf("!3cQa!-eUM!.\"`D!.4j`!.4hJ!.4f4!.\"X=!-eJh!42_k!3lNS!3cI]!3cK3!C-a3!;HVO!AFRA\n");
        printf("!D`b&!EK6R!F,Z(!EK5S!CQs+!AOUV!<<.'!3cKG!3cQM!C-a3~>\n");
        printf("<~!!*0H\"T\\]L\"TotQ\"![sO+pB4o\"T^.V\"X4$P,6J)n!sKqP\"Tno/,m>Ut+pA&P!<XPM\n");
        printf("+p//~>}U NC} B\n");
    }
    if (!bSingle || strstr(jiestr, "j19 ") != NULL)
    {
        printf("/j19 {{<~Pop(<z\"Q0F@!Jgj%%!I+^7!EfM.!A\"<_!<*&:!6#\"a!.Y,)!/L\\!!8R]a!@e/T!H8,P\n");
        printf("!K@0*!OVud!U'SU!J1@C!:^)e!&O^e!&O^Q!;$;D!K[?(!XJiQ!dFac!qHAH\"*aiW\",[,4\"/l6f\n");
        printf("\"4.(A\"4.(Q\",-c`\"$Qb&!rN)M!kA?s!d+PS!\\a[D!cS3g!ji$F!rN-7\"!dt\"!osH\"!k8=t!K[CK\n");
        printf("!O;fP!QkM0!S@LA!Jgj%%!IOtl!JCP+!k8=`!e:@-!_*6[!Xo-D!RUtF!MKSS!IOtl!>khO!>kh;\n");
        printf("!I+V+!Q\"iZ!VQMr!\\+25!`B#A!cS-C!f[1D!hfTR!iuAo!k&)5!k&)J!ic6V!hKCb!e(-X!_`U<\n");
        printf("!Z;!s!O;_$!>khO!OMm?!O)U+!UToE!ZD)O!]C'G!`B%%?!bDBF!cS/]!dXks!e153!e(/H!dk#[\n");
        printf("!c%%g]!_NKL![n):!VQPn!OMm?~>\n");
        printf("<~!!+8I\"![sO,6\\/o\"XF0R!s'SJ+p@uL-3XDP,m=V~>}U NC} B\n");
    }
    if (!bSingle || strstr(jiestr, "j1a ") != NULL)
    {
        printf("/j1a {{<~Pop(Lz\"Q0F@!2KY.!5no.!8IUN!;6Gl!>GR5!q65h!q64=!5nmd!9=.d!;lj/!>Y\\M\n");
        printf("!Ajfk!q64)!q62O!/po>!3?0>!5nk^!8[^'!;lhE!q62;!q61<\"\"XF7\"\"=4N\"\"4/k\"\"427\"&K#s\n");
        printf("!t54o!pBZt!2KY.\"#'eP\"#p@P\"%%WKp\"'boC\"*=Us\",m<N\"/Z.q\"2k95\"+UJ=\"'>X>!Y>GJ!Y>H]\n");
        printf("\"(24Y!uM-a!o*m>!13kM!4W,M!71gm!9sZ6!=/dT!Sdd)!Sdbk!2'E.!1X-V!/q\"F!.b4J!,r\"c\n");
        printf("!*TH<!(-gh!'11P!'UIG!($a=!)<TC!+H\"W!-SEk!/1K0!0@8R!1Etr!2'DD!2'Do!SdbW!Sda6\n");
        printf("!SRT'!S@G*!Yb\\1!YGK.!Y>F0!Y>G6\"'bp.\"#'eP!8%%@W!8%%@C!MBLa!MBLu!8%%@W!`T6*!`T5k\n");
        printf("\"!@Z8\"!@ZL!`T6*!7V'8!7V'$!MBKF!MBKZ!7V'8!`T4d!`T4P\"\"44%%\"\"449!`T4d~>\n");
        printf("<~!!*0*,Qe2q!sKnO\"XF0N\"!Rmt!sKtQ,m=AS!sKkN!@.aJ,Qe&m!s'YL$3~>}U NC} B\n");
    }
    printf("/BH {%d} def\n", 2250 / nWeeks);
    printf("/Mf {gsave currentpoint translate\n");
    printf("0 0 600 BH L\n600 0 600 BH L\n1200 0 600 BH L\n");
    printf("1800 0 600 BH L\n2400 0 600 BH L\n3000 0 600 BH L\n");
    printf("3600 0 600 BH L\ngrestore} B\n");
    printf("%%%%EndSetup\n");
    printf("\n%%%%Page: 1 1\n");
    if (bSingle)
    {
        printf("gsave 50 400 moveto currentpoint translate\n");
        printf("%% Grids\n");
        int i;
        for (i = 0; i < 7; i++)
            printf("%d 2250 600 150 F\n", (i * 600));
        for (i = 0; i < nWeeks; i++)
            printf("0 %d m Mf\n", (i * 2250 / nWeeks));
        printf("%% Month heading\n");
    }
}

void PrintClosingPS()
{
    printf("grestore\nshowpage\n%%%%Trailer\n%%%%EOF\n");
}
