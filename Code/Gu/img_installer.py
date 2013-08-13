#-*- coding: UTF-8 -*-

import urllib.request
import urllib.parse
from pyquery import PyQuery as pq

#target_url = 'http://www.plant.csdb.cn/photo?page=1'
target_url = 'http://www.plantphoto.cn/list?keyword=%E5%8C%99%E5%8F%B6%E9%BB%84%E6%9D%A8'

def _get_resource_name(headers):
    '''
    \brief  Get resource name from response headers
    '''
    for header in headers:
        if header[0] == 'Content-Disposition':
            break
    else:
        return None

    begin = header[1].find('"') + 1
    end = header[1].rfind('"')

    return header[1][begin: end]

def _get_resource_size(headers):
    for header in headers:
        if header[0] == 'Content-Length':
            break
    else:
        return None

    return header[1]

def download_resource(tg):
        print(tg)
        assert tg.startswith('http://')
        d = pq(url=tg)
        p = d('img')
        print(len(p))
        dst = []
        for i in p:
                tmp = i.items()
                aa = d(i).attr('src')
                dst.append(aa)

        print(len(dst))
        for url in dst:
            print(url)
            nounce = 1
            res = urllib.request.urlopen(url)
            print("comehere")
            if res.status != 200:
                    continue
            print(res.getheaders())
            name = _get_resource_name(res.getheaders())
            size = _get_resource_size(res.getheaders())
            print(name)
            print(size)
            with res, open(str(nounce), 'wb') as output:
                print("------------------------")
                output.write(res.read())
            nounce += 1
					
def test():
	ret = download_resource(target_url)

if __name__ == '__main__':
	test()
