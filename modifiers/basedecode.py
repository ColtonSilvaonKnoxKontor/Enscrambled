import base64

html = '''<!DOCTYPE html>
<html lang="en">
<html>
<head>
<meta http-equiv="content-type" content="text/html; charset=UTF-8">
<title></title>
</head>
<body>
<div align="center">
<h1>NOTICE!<br>
</h1>
</div>
&nbsp;&nbsp;&nbsp; The server of this machine is now infected with
(not so harmful) malware.<br>
<br>
&nbsp;&nbsp;&nbsp; The original webpage of this company,
organization, or an administrator were deleted; or the web server
was altered, and then replaced by this simple html file.<br>
<br>
&nbsp;&nbsp;&nbsp; It means the administrator was trying to install
shady software in an unprotected Linux system. Maybe the admin is
dumb or idiot.<br>
<br>
<br>
<br>
<br>
<div align="right">&nbsp;&nbsp; This message is brought to you by:<br>
<b>ChinaWaterStealers</b><br>
</div>
</body>
</html>
'''

encoded = base64.b64encode(html.encode()).decode()
print(encoded)

