import urllib
import urllib2

values = {"username":"shengdewu", "password":"baicsm19881205"}
data = urllib.urlencode(values)
url = "http://passport.csdn.net/account/long?from=http://my.csdn.net/my/mycsdn"

response = urllib2.urlopen(url,data)

print (response.read())


