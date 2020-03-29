<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:ccal="http://ccal.chinesebay.com/ccal/">
  <xsl:template match="/">
    <xsl:variable name="yearvalue" select="ccal:year/@value" />
    <xsl:variable name="titlevalue">
      <xsl:choose>
        <xsl:when test="count(ccal:year/ccal:month)=1">
          <xsl:value-of select="ccal:year/ccal:month/@name" />&#160;<xsl:value-of select="$yearvalue" />
          / <xsl:value-of select="$yearvalue" />年<xsl:value-of select="ccal:year/ccal:month/@value" />月
        </xsl:when>
        <xsl:otherwise>
          Year&#160;<xsl:value-of select="$yearvalue" /> / <xsl:value-of select="$yearvalue" />年
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <html>
      <head>
        <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
        <meta name="GENERATOR" content="ccal-2.5.2 by Zhuo Meng, http://ccal.chinesebay.com/ccal/" />
        <title>Chinese Calendar for <xsl:value-of select="$titlevalue" /></title>
      </head>
      <body>
        <h1>Chinese Calendar for <xsl:value-of select="$titlevalue" /></h1>
        <center>
          <xsl:for-each select="ccal:year/ccal:month">
            <table border="1" cellspacing="1" width="90%">
              <tr>
                <th colspan="7" width="100%"><xsl:value-of select="@name" />&#160;<xsl:value-of select="$yearvalue" />
                &#160;&#160;<xsl:value-of select="@cname" /></th>
              </tr>
              <tr align="center">
                <td width="15%">
                  <font color="#FF0000">Sun&#160;&#160;日</font>
                </td>
                <td width="14%">Mon&#160;&#160;一</td>
                <td width="14%">Tue&#160;&#160;二</td>
                <td width="14%">Wed&#160;&#160;三</td>
                <td width="14%">Thu&#160;&#160;四</td>
                <td width="14%">Fri&#160;&#160;五</td>
                <td width="15%">
                  <font color="#00E600">Sat&#160;&#160;六</font>
                </td>
              </tr>
              <xsl:for-each select="ccal:week">
                <tr align="right">
                  <xsl:for-each select="ccal:day">
                    <xsl:choose>
                      <xsl:when test="position()=1 or @isholiday='true'">
                        <td>
                          <font color="#FF0000">
                            <xsl:apply-templates select="." />
                          </font>
                        </td>
                      </xsl:when>
                      <xsl:when test="position()=7">
                        <td>
                          <font color="#00E600">
                            <xsl:apply-templates select="." />
                          </font>
                        </td>
                      </xsl:when>
                      <xsl:otherwise>
                        <td>
                          <xsl:apply-templates select="." />
                        </td>
                      </xsl:otherwise>
                    </xsl:choose>
                  </xsl:for-each>
                </tr>
              </xsl:for-each>
            </table>
          </xsl:for-each>
        </center>
      </body>
    </html>
  </xsl:template>
  <xsl:template match="ccal:day">
    <xsl:choose>
      <xsl:when test="@value=1 or @cdate=1">
        <xsl:value-of select="@value" />&#160;&#160;<xsl:value-of select="@cmonthname" /><xsl:value-of select="@cdatename" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="@value" />&#160;&#160;<xsl:value-of select="@cdatename" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
</xsl:stylesheet>
