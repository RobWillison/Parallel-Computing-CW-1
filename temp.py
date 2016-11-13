

first = True
core1 = 0
results = []

with open("50x50", "r") as my_file:
    for lines in my_file:
        if first:
            first = False
            continue

        numbers = lines.split(' ')

        if int(numbers[0]) == 1:
            core1 = float(numbers[1])

        numbers.append(core1 / float(numbers[1]))

        results.append(numbers.copy())

f = open('myfile','w')
for temp in results:
    f.write(str(numbers[0]) + ' ' + str(numbers[1]) + ' ' + str(numbers[2]) + '\n')
f.close()
