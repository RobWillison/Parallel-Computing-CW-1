

first = True
core1 = 0
results = []
string = "100"
f = open('Isoefficiency/New/' + string,'w')
f.write('cores time efficiency\n')
with open("Isoefficiency/" + string, "r") as my_file:
    for lines in my_file:
        lines = lines.replace("\n", '');
        numbers = lines.split(' ')

        if int(numbers[0]) == 1:
            core1 = float(numbers[1])

        p = float(numbers[0])
        numbers.append((core1 / (p * float(numbers[1]))))

        f.write(str(numbers[0]) + ' ' + str(numbers[2]) + '\n')

f.close()
