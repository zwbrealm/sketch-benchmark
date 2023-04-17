import os
import time
# import csv
# import pandas as pd

# times_of_run = 5
# app_list = ['./nitro','./um','./skv','./es','./fr','./cbf','./cm','./cs']
app_list = ['./skv2']
# num_threads_cmd = ['-l 0-1 -n 2 -- -P -p 1 --rx="(0,0,0,0)(0,1,1,1)" ','-l 0-2 -n 2 -- -P -p 1 --rx="(0,0,0,0)(0,1,1,1)(0,2,2,2)"','-l 0-3 -n 2 -- -P -p 1 --rx="(0,0,0,0)(0,1,1,1)(0,2,2,2)(0,3,3,3)"','-l 0-4 -n 2 -- -P -p 1 --rx="(0,0,0,0)(0,1,1,1)(0,2,2,2)(0,3,3,3)(0,4,4,4)"','-l 0-5 -n 2 -- -P -p 1 --rx="(0,0,0,0)(0,1,1,1)(0,2,2,2)(0,3,3,3)(0,4,4,4)(0,5,5,5)"','-l 0-6 -n 2 -- -P -p 1 --rx="(0,0,0,0)(0,1,1,1)(0,2,2,2)(0,3,3,3)(0,4,4,4)(0,5,5,5)(0,6,6,6)"','-l 0-7 -n 2 -- -P -p 1 --rx="(0,0,0,0)(0,1,1,1)(0,2,2,2)(0,3,3,3)(0,4,4,4)(0,5,5,5)(0,6,6,6)(0,7,7,7)"','-l 0-8 -n 2 -- -P -p 1 --rx="(0,0,0,0)(0,1,1,1)(0,2,2,2)(0,3,3,3)(0,4,4,4)(0,5,5,5)(0,6,6,6)(0,7,7,7)(0,8,8,8)"','-l 0-9 -n 2 -- -P -p 1 --rx="(0,0,0,0)(0,1,1,1)(0,2,2,2)(0,3,3,3)(0,4,4,4)(0,5,5,5)(0,6,6,6)(0,7,7,7)(0,8,8,8)(0,9,9,9)"']
num_threads_cmd = ['-l 0-1 -n 2 -- -P -p 1 --rx="(0,0,0,0)"','-l 0-1 -n 2 -- -P -p 1 --rx="(0,0,0,0)(0,1,1,1)"','-l 0-2 -n 2 -- -P -p 1 --rx="(0,0,0,0)(0,1,1,1)(0,2,2,2)"','-l 0-3 -n 2 -- -P -p 1 --rx="(0,0,0,0)(0,1,1,1)(0,2,2,2)(0,3,3,3)"','-l 0-4 -n 2 -- -P -p 1 --rx="(0,0,0,0)(0,1,1,1)(0,2,2,2)(0,3,3,3)(0,4,4,4)"','-l 0-5 -n 2 -- -P -p 1 --rx="(0,0,0,0)(0,1,1,1)(0,2,2,2)(0,3,3,3)(0,4,4,4)(0,5,5,5)"','-l 0-6 -n 2 -- -P -p 1 --rx="(0,0,0,0)(0,1,1,1)(0,2,2,2)(0,3,3,3)(0,4,4,4)(0,5,5,5)(0,6,6,6)"','-l 0-7 -n 2 -- -P -p 1 --rx="(0,0,0,0)(0,1,1,1)(0,2,2,2)(0,3,3,3)(0,4,4,4)(0,5,5,5)(0,6,6,6)(0,7,7,7)"','-l 0-8 -n 2 -- -P -p 1 --rx="(0,0,0,0)(0,1,1,1)(0,2,2,2)(0,3,3,3)(0,4,4,4)(0,5,5,5)(0,6,6,6)(0,7,7,7)(0,8,8,8)"','-l 0-9 -n 2 -- -P -p 1 --rx="(0,0,0,0)(0,1,1,1)(0,2,2,2)(0,3,3,3)(0,4,4,4)(0,5,5,5)(0,6,6,6)(0,7,7,7)(0,8,8,8)(0,9,9,9)"']
cmd = []
for i in app_list:
    for index,j in enumerate(num_threads_cmd):
        # tmp_cmd = 'timeout -s SIGKILL 25s '+i+' '+j+'>> res/'+i[1:]+'-'+str(index+2)
        tmp_cmd = 'timeout -s SIGKILL 20s '+i+' '+j
        cmd.append(tmp_cmd)
print(cmd)
t1 = time.time()
# for i in cmd:
#     print(i)
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
key = app_list
overall_throughoutput_dict = dict([(k, []) for k in key])
overall_cpu_cycle_dict = dict([(k, []) for k in key])




for cmd_index,cmd_tmp in enumerate(cmd):
    str_1 = os.popen(cmd_tmp).read()
    str_1 = str_1.split('\n')
    for n in str_1:
        n = n.replace('\n','')
        n = n.replace('\r','')

    # throughoutput = str_1[2].split(':')[1]
    # cycles_per_packet = str_1[3].split(':')[1]
    # print(i+' '+'throughoutput:'+throughoutput + ' cycles_per_packet: '+cycles_per_packet+'\n')
    # print(str_1)
    new_str1 = []
    for index,i in enumerate(str_1):
        if i[:3] == 'avg':
            break
    new_str1.extend(str_1[index:])
    # print(new_str1)

    key = [0,1,2,3,4,5,6,7,8,9]
    throughoutput_dict = dict([(k, []) for k in key])
    cpu_cycle_dict = dict([(k, []) for k in key])
    # print(new_str1)
    # print(len(new_str1)-2)
    for i in range(0,len(new_str1)-2,3):
        cpu_cycle_value = int(new_str1[i].split(":")[1])
        throughoutput_value = int(new_str1[i+1].split(":")[1])
        key_id = int(new_str1[i+2][-1])
        
        throughoutput_dict[key_id].append(throughoutput_value)
        cpu_cycle_dict[key_id].append(cpu_cycle_value)

    print(throughoutput_dict)
    print(cpu_cycle_dict)

    overall_throughoutput = 0
    overall_cpu_cycle = 0
    avg_cpu_cycle = 0
    # index = 5
    num_cores = cmd_index % 10 + 1

    for i in range(num_cores):
        overall_throughoutput+=throughoutput_dict[i][1]
        overall_cpu_cycle+=cpu_cycle_dict[i][1]
    avg_cpu_cycle = overall_cpu_cycle/num_cores
    avg_cpu_cycle = int(avg_cpu_cycle)
    
    name = app_list[int(cmd_index/10)]
    overall_throughoutput_dict[name].append(overall_throughoutput)
    overall_cpu_cycle_dict[name].append(avg_cpu_cycle)
    print(str(name)+' cores:'+str(num_cores)+':' + '  cycle: '+str(avg_cpu_cycle)+'  throughoutput  '+str(overall_throughoutput))
    with open('res/skv.txt','a') as f:
        f.write(str(name)+' cores:'+str(num_cores)+':' + '  cycle:'+str(avg_cpu_cycle)+'  throughoutput '+str(overall_throughoutput)+'\n')
    # print(num_cores)
    time.sleep(2)

t2 = time.time()
# print(a)
# print(b)
print(t2-t1)