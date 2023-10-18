import requests
import json

FilesURL = 'https://github.com/danielweidman/pixmob-ir-reverse-engineering/tree/main/rf/edited_rf_captures/868Mhz/'

for j in json.loads(requests.get(FilesURL).text)['payload']['tree']['items']:
    if '.sub' in j['name']:
        l = [int(int(x)/510) for x in requests.get(FilesURL.replace('github.com', 'raw.githubusercontent.com').replace('tree', '')+j['name']).text.split('RAW_Data: ')[1].split(' ')]
        s = ''
        for i in l:
            if i > 0:
                for d in range(i):
                    s += '1'
            else:
                for d in range(-1*i):
                    s += '0'
        s=s+'00000000'
        Hex = ''
        for i in range(0, len(s), 8):
            Hex = Hex + hex(int(s[i:i+8], 2)) + ','
        print(('// {' + Hex[:-1] + '}, //' + j['name'].replace('.sub', '')).replace(',0x0}','}'))
