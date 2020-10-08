import csv
import random

loopTime = 500 #5000000
intMax = 20 #2147483647
intMin = -20 #-2147483648
with open('input.csv','w',newline = '') as csvfile:
	writer = csv.writer(csvfile, delimiter = '|')
	row = []
	for i in range(0,loopTime):
		for j in range(0,20):
			row.append(random.randint(intMin,intMax))
		writer.writerow(row)
		row.clear()
		if i % 10000 == 0:
			print(i)

