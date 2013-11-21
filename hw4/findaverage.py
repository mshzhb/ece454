file = open("measurements.out", 'r')

row = file.readlines()

counter = 0
sum = 0.0
for line in row:
	counter += 1
	data = line.split()
	#print data[1]
	sum += float(data[1])
	if counter == 5:
		print str(data[0])+"     "+str(sum/5.0)
		counter = 0
		sum = 0

