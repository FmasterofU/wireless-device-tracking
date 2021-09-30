import matplotlib.pyplot as plt
import math
import sys


device1 = [[],[],[]]
device1_mac = 'c6:a4:1a:c6:48:23'
device2 = [[],[],[]]
device2_mac = '40:4e:36:3a:45:1b'
basetime = 0.0
device3 = [[],[],[]]
device3_mac = '50:32:75:4d:2a:69'


def extract_time_from_PR(line):
    return int(line[line.find('PR timestamp ') + len('PR timestamp '): line.find(' channel')])

def store_PR_location(mac, i, time):
     if mac == device1_mac:
        device1[i].append(time)
     elif mac == device2_mac:
         device2[i].append(time)
     elif mac == device3_mac:
         device3[i].append(time)

if __name__ == '__main__':
    for i, path in enumerate(sys.argv):
        if i == 0 or i == 4:
            continue
        else:
            with open(path, 'r', encoding='utf-8') as f:
                for line in f:
                    if 'PR timestamp' in line:
                        if basetime == 0.0:
                            basetime = extract_time_from_PR(line)
                        if extract_time_from_PR(line) < basetime + int(sys.argv[4])*1000:
                            pass
                        elif device1_mac in line:
                            store_PR_location(device1_mac, i-1, extract_time_from_PR(line) - basetime - int(sys.argv[4])*1000)
                        elif device2_mac in line:
                            store_PR_location(device2_mac, i-1, extract_time_from_PR(line) - basetime - int(sys.argv[4])*1000)
                        elif device3_mac in line:
                            store_PR_location(device3_mac, i-1, extract_time_from_PR(line) - basetime - int(sys.argv[4])*1000)
    fig, (ax1, ax2,ax3) = plt.subplots(3,1)
    ax1.plot(device1[0], ['desno']*len(device1[0]), 'ro', device1[1], ['centar']*len(device1[1]), 'bs', device1[2], ['lijevo']*len(device1[2]), 'g^')
    ax2.plot(device2[0], ['desno']*len(device2[0]), 'ro', device2[1], ['centar']*len(device2[1]), 'bs', device2[2], ['lijevo']*len(device2[2]), 'g^')
    ax3.plot(device3[0], ['desno']*len(device3[0]), 'ro', device3[1], ['centar']*len(device3[1]), 'bs', device3[2], ['lijevo']*len(device3[2]), 'g^')
    plt.show()