<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="/">
  <html>
  <body>
  <h2>Help Files</h2>
  <table border="1">
    <tr bgcolor='#9acd32'>
      <th>Level</th>
      <th>Keywords</th>
	  <th>Race</th>
	  <th>Clan</th>
	  <th>Topic</th>
	  <th>Syntax</th>
	  <th>Description</th>
    </tr>
    <xsl:for-each select="Help/help_object">
    <tr>
      <td><xsl:value-of select="level"/></td>
      <td><xsl:value-of select="keyword"/></td>
      <td><xsl:value-of select="race"/></td>
      <td><xsl:value-of select="clan"/></td>
      <td><xsl:value-of select="Topic"/></td>
      <td><xsl:value-of select="Syntax"/></td>
	  <td><xsl:value-of select="Desc"/></td>
   </tr>
    </xsl:for-each>
  </table>
  </body>
  </html>
</xsl:template>

</xsl:stylesheet> 