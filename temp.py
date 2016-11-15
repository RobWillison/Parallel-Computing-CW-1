

first = True
core1 = 0
results = []
string = "50x50"
f = open('KarpFlatt/' + string,'w')
f.write('cores time karpflatt\n')
with open(string, "r") as my_file:
    for lines in my_file:
        if first:
            first = False
            continue

        lines = lines.replace("\n", '');
        numbers = lines.split(' ')

        if int(numbers[0]) == 1:
            core1 = float(numbers[1])
            continue

        top = ((1 / float(numbers[2])) - (1 / float(numbers[0])))
        bottom = 1 - (1 / float(numbers[0]))
        numbers.append(top / bottom)

        f.write(str(numbers[0]) + ' ' + str(numbers[1]) + ' ' + str(numbers[3]) + '\n')

f.close()
