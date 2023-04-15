import os
import time

a = dict()
b = dict()
# times_of_run = 5
file_list = ['./nitro','./um2','./skv','./es','./fr','./cbf','./cm','./cs']
print(file_list)
print(100,128)
t1 = time.time()
# for i in file_list:
#     res_1 = 0
#     res_2 = 0
    
#     cmd = './'+i+'/'+'1'
#     str_1 = os.popen(cmd).read()
#     str_1 = str_1.replace('\n','')
#     str_1 = str_1.replace('\r','')
#     str_1 = str_1.split(' ')
#     throughoutput = str_1[1].split(':')[1]
#     cycles_per_packet = str_1[3].split(':')[1]
#     throughoutput = int(throughoutput)
#     cycles_per_packet = int(cycles_per_packet)
#     res_1+= throughoutput
#     res_2+= cycles_per_packet
#     time.sleep(2)
cmd2 = './cm2 -l 1,2 -n 4'
cmd3 = './cm2 -l 1,2,3 -n 4'
cmd4 = './cm2 -l 1,2,3,4 -n 4'
cmd5 = './cm2 -l 1,2,3,4,5 -n 4'
cmd6 = './cm2 -l 1,2,3,4,5,6 -n 4'
cmd7 = './cm2 -l 1,2,3,4,5,6,7 -n 4'
cmd8 = './cm2 -l 1,2,3,4,5,6,7,8 -n 4'
cmd9 = './cm2 -l 1,2,3,4,5,6,7,8,9 -n 4'
cmd10 = './cm2 -l 1,2,3,4,5,6,7,8,9,10 -n 4'
cmd = [cmd2,cmd3,cmd4,cmd5,cmd6,cmd7,cmd8,cmd9,cmd10]
with open('res.txt','a') as f:
    for i in cmd:
        str_1 = os.popen(i).read()
        str_1 = str_1.replace('\n','')
        str_1 = str_1.replace('\r','')
        str_1 = str_1.split(' ')
        print(str_1)
        throughoutput = str_1[-2].split(':')[1][:-3]
        cycles_per_packet = str_1[-1].split(':')[1]
        f.write(i+' '+'throughoutput:'+throughoutput + ' cycles_per_packet: '+cycles_per_packet+'\n')
        time.sleep(3)
t2 = time.time()
# print(a)
# print(b)
# print(t2-t1)
            