#!/usr/bin/python
import sys
import base64
import argparse
import json
import hmac
import hashlib

def decode_base64(data):
    """Decode base64, padding being optional.

    :param data: Base64 data as an ASCII byte string
    :returns: The decoded byte string.

    Source : http://stackoverflow.com/questions/2941995/python-ignore-incorrect-padding-error-when-base64-decoding

    """
    data = str(data)
    data = data.replace("-","+")
    data = data.replace("_","/")
    missing_padding = len(data) % 4
    if missing_padding != 0:
        data += b'='* (4 - missing_padding)
    return base64.decodestring(data)

def parse_token(jwt_token, silent):
    jwt_tab = jwt_token.split(".")
    if len(jwt_tab) != 3:
        print "Bad format for input JWT token ... Exiting"
        return

    header = { 
            "raw" : decode_base64(jwt_tab[0]), 
            "b64" : jwt_tab[0] ,
            "json": json.loads(decode_base64(jwt_tab[0]))
            }
    payload = { 
            "raw" : decode_base64(jwt_tab[1]), 
            "b64" : jwt_tab[1],
            "json": json.loads(decode_base64(jwt_tab[1]))
            }
    H = decode_base64(jwt_tab[2]).encode('hex')

    if not silent:
        # Print token in human readable format
        print "Header : %s" % json.dumps(header["json"], indent=4, sort_keys=True)
        print "Payload : %s" % json.dumps(payload["json"], indent=4, sort_keys=True)
        print

        # Print using Hashcat format
        final = "%s:%s.%s" %(H, header['b64'], payload['b64'])
        print "Hashcat format : %s" % final
        print

        # Print using John jumbo format
        final = "%s.%s#%s" %(header['b64'], payload['b64'], H)
        print "John jumbo format : %s" % final
        print
    else:
        final = "%s.%s#%s" %(header['b64'], payload['b64'], H)
        print final


def main():
    """"Main function"""
    parser = argparse.ArgumentParser()
    parser.add_argument('jwt_token', action='store',  help='JWT token')

    # Optionnal args
    parser.add_argument('-o', '--only', action='store_true', default=False, help='Only output John jumbo format')

    if len(sys.argv) == 1:
        print "[!] Missing JWT token ... Exiting"
        parser.print_help()
        sys.exit(1)
    else:
        args = parser.parse_args()
        jwt_token = args.jwt_token
        silent = args.only
        parse_token(jwt_token, silent)

if __name__ == "__main__":
    main()
