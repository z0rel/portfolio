from pandas import DataFrame
import json

with open("data.json", "r") as f:
    objs = json.loads(f.read())
    marks = []
    names = []
    values = []
    for x in objs:
        marks.append(x['mark'])
        names.append(x['name'])
        values.append(x['value'])
    df = DataFrame(data={'mark': marsk, 'names': names, 'values': values})
    df.to_excel('out.xlsx')


