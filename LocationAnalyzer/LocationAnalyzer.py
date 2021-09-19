import matplotlib.pyplot as plt
import math
import sys


device1 = [[],[],[]]
device1_mac = 'c6:a4:1a:c6:48:23'
device2 = [[],[],[]]
device2_mac = '40:4e:36:3a:45:1b'
basetime = 0.0


def extract_time_from_PR(line):
    return int(line[line.find('PR timestamp ') + len('PR timestamp '): line.find(' channel')])

def store_PR_location(mac, i, time):
     if mac == device1_mac:
        device1[i].append(time)
     else:
         device2[i].append(time)

if __name__ == '__main__':
    for i, path in enumerate(sys.argv):
        if i == 0:
            continue
        else:
            with open(path, 'r', encoding='utf-8') as f:
                for line in f:
                    if 'PR timestamp' in line:
                        if basetime == 0.0:
                            basetime = extract_time_from_PR(line)
                        if device1_mac in line:
                            store_PR_location(device1_mac, i-1, extract_time_from_PR(line) - basetime)
                        elif device2_mac in line:
                            store_PR_location(device2_mac, i-1, extract_time_from_PR(line) - basetime)
    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.plot(device1[0], ['balkon']*len(device1[0]), 'ro', device1[1], ['centar']*len(device1[1]), 'bs', device1[2], ['stepeniste']*len(device1[2]), 'g^')
    ax2.plot(device2[0], ['balkon']*len(device2[0]), 'ro', device2[1], ['centar']*len(device2[1]), 'bs', device2[2], ['stepeniste']*len(device2[2]), 'g^')
    plt.show()