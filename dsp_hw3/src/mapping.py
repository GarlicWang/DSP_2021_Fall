import sys

with open(sys.argv[1], 'r', encoding='big5-hkscs') as fin:
    data = fin.read().split('\n')
data.pop()
map_1 = []
map_2 = []
map_3 = []
for element in data:
    element = element.split()
    element[1] = element[1].split('/')
    for zhu in element[1]:
        if zhu[0] not in map_1:
            map_1.append(zhu[0])
            map_2.append([])
        if element[0] not in  map_2[map_1.index(zhu[0])]:
            map_2[map_1.index(zhu[0])].append(element[0])
    if element[0] not in map_3:
        map_3.append(element[0])
    # print(map_3)
with open(sys.argv[2], 'w', encoding='big5-hkscs') as fout:
    for idx, zhu in enumerate(map_1):
        fout.write(zhu + '\t')
        for element in map_2[idx]:
            fout.write(element + ' ')
        fout.write('\n')
        # for element in map_2[idx]:
        #     fout.write(element + '\t' + element + '\n')
    for element in map_3:
        fout.write(element + '\t' + element + '\n')