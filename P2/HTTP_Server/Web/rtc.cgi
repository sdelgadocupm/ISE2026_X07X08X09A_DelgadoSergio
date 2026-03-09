t <html><head><title>RTC</title>
t <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t <script language=JavaScript type="text/javascript">
# Define URL and refresh timeout
t var formUpdate = new periodicObj("rtc.cgx", 200);
t function periodicUpdateRtc() {
t   updateMultiple(formUpdate);
t   rtc_elTime = setTimeout(periodicUpdateRtc, formUpdate.period);
t }
t setTimeout(periodicUpdateRtc, formUpdate.period);
t </script></head>
i pg_header.inc
t <h2 align="center"><br>RTC Module</h2>
t <p><font size="2">This page allows you to view time and date from <b>RTC Module</b>.</font></p>
t <form action="rtc.cgi" method="post" name="ad">
t <input type="hidden" value="ad" name="pg">
t <table border=0 width=99%><font size="3">
t <tr style="background-color: #aaccff">
t  <th width=15%>Item</th>
t  <th width=15%>Data</th></tr>
t <tr><td><img src="pabb.gif">Time:</td>
t   <td align="center">
t <input type="text" readonly style="background-color: transparent; border: 0px"
c h 1  size="10" id="time_value" value="%s"></td></tr>
t <tr><td><img src="pabb.gif">Date:</td>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c h 2  size="10" id="date_value" value="%s"></td></tr>
t </font></table></form>
i pg_footer.inc
. End of script must be closed with period

