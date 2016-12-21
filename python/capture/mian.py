__author__ = 'lenovo'

from CCapture import CCapture

def filter():
    capture = CCapture()
    param = {'page':1, 'name':'qiushibaike'}
    capture.setParam(**param)
    result = capture.start()
    print result
    return

filter()
