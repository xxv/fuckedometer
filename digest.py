#!/usr/bin/env python3

import requests

url='http://table-cache1.predictwise.com/latest/group_3.json'
output='fucked.txt'

r=requests.get(url)
result = r.json()

republican_win=None

for table in result['tables']:
    if table['id'] == "1032":
        for sub_table in table['table']:
            if sub_table[0] == "Republican":
                republican_win=sub_table[1]
                break
        break

if republican_win != None:
    with open(output, 'w') as out:
        out.write(republican_win)
