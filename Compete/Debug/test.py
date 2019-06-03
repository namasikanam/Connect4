import os
import time

number_of_run = 1
l = 80
r = 102

sum_of_win_rate = 0

start_time = time.time()

for number_of_dll in range(l, r, 2):
    os.system('.\\Compete.exe C:\\Users\\namas\\Documents\\Course\\2019Spring\\Introduction-to-Artificial-Intelligence\\AI_project_win\\Connect4\\Release\\Strategy.dll C:\\Users\\namas\\Documents\\Course\\2019Spring\\Introduction-to-Artificial-Intelligence\\AI_project_win\\TestCases\\' +
              str(number_of_dll) + '.dll result.txt ' + str(number_of_run))
    with open('result.txt', 'r') as f:
        lines = f.readlines()
        win_rate = float(lines[-2].split(':')[1])
    sum_of_win_rate += win_rate

    print('Win rate to ' + str(number_of_dll) + '.dll: ' + str(win_rate))
    print('------ %s seconds -------' % (time.time() - start_time))

print('Total win rate =', sum_of_win_rate / ((r-l)/2))
