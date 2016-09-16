import urllib.request


respone = urllib.request.urlopen("http://www.baidu.com")

print (respone.read())

key = {'name':'baoyong', 'name':'xufang','age':18}

print (key['name'])
print (len(key))
