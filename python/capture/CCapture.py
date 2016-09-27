#-*-coding:UTF-8-*-
__author__ = 'lenovo'

import urllib2
import re

class CCapture :
    #允许的属性
    __slots__ = ("__pageIndex", "__user_agent", "__headers", "__name")
    #初始化方法
    def __init__(self):
        self.__pageIndex = 1
        self.__user_agent = 'Mozilla/4.0 (compatible; MSIE 5.5; Windows NT)'
        self.__headers = { 'User-Agent' : self.__user_agent }
        return

    def setParam(self, **param):
        self.__pageIndex = param["page"]
        self.__name = param["name"]
        return

    def getPage(self):
        url = 'http://www.qiushibaike.com/hot/page/' + str(self.__pageIndex)
        content = None
        try:

            request = urllib2.Request(url, headers = self.__headers)
            response = urllib2.urlopen(request)
            content = response.read().decode('utf-8')

        except urllib2.URLError, e:
            if hasattr(e, "reason"):
                print u"链接糗事百科失败，失败原因:", e.reason
        finally:
            return content

    def filter(self, page):
        pattern = re.compile('<div class=\"article.*?id.*?>.*?<span>(.*?)</span>',re.S)
        items  = re.findall(pattern, page)
        for item in items:
            print item
        return

    def start(self):
        result = self.getPage()
        self.filter(result)
        return result
