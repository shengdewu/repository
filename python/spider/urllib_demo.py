import urllib
import urllib.request

def catch_url(url):
    #打印字符串
    #urlopen(url, data, timeout)

    response = urllib.request.urlopen(url)
    result = response.read()

    return result


url = "http://www.baidu.com"

response = catch_url(url)

print (response)

