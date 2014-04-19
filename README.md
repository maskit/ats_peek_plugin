ats_peek_plugin
===============

A peek plugin for Apache Traffic Server

Example
-------

```
20140420.00h30m40s [INFO] ## Request (11079d9f0)
20140420.00h30m40s [INFO] # Method: GET
20140420.00h30m40s [INFO] # URL: /images/top/sp2/cmn/logo-ns-131205.png (was http://k.yimg.jp/images/top/sp2/cmn/logo-ns-131205.png)
20140420.00h30m40s [INFO] # Version: HTTP/1.1
20140420.00h30m40s [INFO] = User-Agent: curl/7.30.0
20140420.00h30m40s [INFO] = Accept: */*
20140420.00h30m40s [INFO] = Host: k.yimg.jp
20140420.00h30m40s [INFO] A Client-ip: 127.0.0.1
20140420.00h30m40s [INFO] A X-Forwarded-For: 127.0.0.1
20140420.00h30m40s [INFO] A Via: http/1.1 <censored> (ApacheTrafficServer/5.0.0)
20140420.00h30m40s [INFO] ##

20140420.00h30m41s [INFO] ## Response (11079d9f0)
20140420.00h30m41s [INFO] # Version: HTTP/1.1
20140420.00h30m41s [INFO] # Status: 200
20140420.00h30m41s [INFO] # Reason: OK
20140420.00h30m41s [INFO] = Date: Sat, 19 Apr 2014 15:21:40 GMT
20140420.00h30m41s [INFO] = Expires: Sat, 03 May 2014 15:18:01 GMT
20140420.00h30m41s [INFO] = Last-Modified: Wed, 04 Dec 2013 09:04:03 GMT
20140420.00h30m41s [INFO] = Accept-Ranges: bytes
20140420.00h30m41s [INFO] D Content-Length: 2928
20140420.00h30m41s [INFO] = Content-Type: image/png
20140420.00h30m41s [INFO] M Age: 542 (was 541)
20140420.00h30m41s [INFO] = Via: HTTP/1.1 ytsorigin8204.img.umd.yahoo.co.jp (YahooTrafficServer/1.20.10 [cHs f ]),HTTP/1.1 yts8240.img.umd.yahoo.co.jp (YahooTrafficServer/1.20.10 [cHs f ])
20140420.00h30m41s [INFO] M Server: ATS/5.0.0 (was YTS/1.20.10)
20140420.00h30m41s [INFO] = Cache-Control: public
20140420.00h30m41s [INFO] = Connection: keep-alive
20140420.00h30m41s [INFO] A Transfer-Encoding: chunked
20140420.00h30m41s [INFO] # Body: PNG image data, 213 x 55, 8-bit colormap, non-interlaced
20140420.00h30m41s [INFO] ##
```
