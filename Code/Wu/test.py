from cv2 import *

f5 = 'F:/Sample/ÑîÊ÷£¨²»´øÒ¶±ú£©/ÑîÊ÷5.jpg'

def show(img):
    imshow('...', img)
    waitKey(0)

def binary_img(img):
    return threshold(img, 0, 255, THRESH_BINARY | THRESH_OTSU)[1]

def canny(img):
    return Canny(img, 0, 255)

def imgopen(img, it = 1):
    return morphologyEx(img, MORPH_OPEN, None, iterations = it);

def imgclose(img, it = 1):
    return morphologyEx(img, MORPH_CLOSE, None, iterations = it);

def find_contour(img):
    return findContours(img, RETR_LIST, CHAIN_APPROX_NONE)
    
img = imread(f5, 0)
eq = equalizeHist(img)
bi = binary_img(eq)
closed = imgclose(bi, 2)
