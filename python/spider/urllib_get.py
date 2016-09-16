import urllib
import urllib2

values = {}
values['username'] = '1016903103@qq.com'
values['password'] = 'baicsm19881205'
data = urllib.urlencode(values)
url = ("https://passport.csdn.net/account/"
       "login")
geturl = url + "?" + data

request = urllib2.Request(geturl)
response = urllib2.urlopen(request)

print response.read()
