import matplotlib.pyplot as plt
import math
import sys


offset_espmillis_time_tracker = []
ntp_lib_time_tracker = []
rtc_32k_isr_time_tracker = []
rtc_sec_32k_isr_millis_time_tracker = []
rtc_time_tracker = []
actual_time_corrector = []
corrector_x = []
actual_time = []
actual_rtc_time = []

def extract_offset(line):
    return float(line[line.find('Offset: ') + len('Offset: ') : line.find(' ms', line.find('Offset: '))])

def results_parser(path):
    state = 0 # 0 - init, 1 - separator, 2 - O+- rtc context log, 3 - offset, 4 - ntp, 5 - rtc 32k, 6 - rtc sec
    ntp_resync_state = 0 # 0 - complete, 1 - partial
    ntp_resync_progress_state = 0 # 0 - n/a, 1 - in progress

    with open(path, 'r', encoding='utf-16-le') as f:
        for line in f:
            if line == '\n':
                pass
            elif state == 0 and '--------' not in line:
                print("INTRO: ", line)
            elif 'NTPTimeSynced--------' in line and ntp_resync_state == 0:
                ntp_resync_progress_state = 0
                state = 1
                print("ntp separator")
            elif '--------' in line and ntp_resync_progress_state == 0:
                state = 1
                print("separator")
            elif 'NTPClient State:' in line:
                ntp_resync_progress_state = 1
                if '[NTP-event] 2:' in line:
                    if ntp_resync_state == 0:
                        corrector_x.append(len(offset_espmillis_time_tracker) + 1)
                        actual_time_corrector.append(extract_offset(line))
                        ntp_resync_state = 1
                    elif ntp_resync_state == 1:
                        actual_time_corrector[-1] += extract_offset(line)
                elif '[NTP-event] 0:' in line:
                    if ntp_resync_state == 1:
                        actual_time_corrector[-1] += extract_offset(line)
                        ntp_resync_state = 0
                    elif ntp_resync_state == 0:
                        corrector_x.append(len(offset_espmillis_time_tracker) + 1)
                        actual_time_corrector.append(extract_offset(line))
                else:
                    print("Other NTP event: ", line)
            elif ntp_resync_progress_state != 0:
                print("DROPPING (ntp sync): ", line)
            elif 'O+' in line or 'O-' in line:
                state = 2
                print("X: ", len(offset_espmillis_time_tracker), " with time: ", line)
            elif state == 1:
                offset_espmillis_time_tracker.append(float(line))
                state = 3
            elif state == 3:
                ntp_lib_time_tracker.append(float(line))
                state = 4
            elif state == 4:
                rtc_32k_isr_time_tracker.append(float(line))
                state = 5
            elif state == 5:
                rtc_sec_32k_isr_millis_time_tracker.append(float(line))
                rtc_time_tracker.append(float(line) - float(line) % 1000)
                state = 6
            else:
                print("ERROR: ", line)

def drop_close_elems():
    for i in range(0, len(corrector_x)):
        corrector_x[i] -= (i + 1)
        offset_espmillis_time_tracker.pop(corrector_x[i])
        ntp_lib_time_tracker.pop(corrector_x[i])
        rtc_32k_isr_time_tracker.pop(corrector_x[i])
        rtc_sec_32k_isr_millis_time_tracker.pop(corrector_x[i])
        rtc_time_tracker.pop(corrector_x[i])

def remove_base():
    base = ntp_lib_time_tracker[0]
    rtc_base = rtc_time_tracker[0]
    for i in range(0, len(offset_espmillis_time_tracker)):
        offset_espmillis_time_tracker[i] -= base
        ntp_lib_time_tracker[i] -= base
        rtc_32k_isr_time_tracker[i] -= base
        rtc_sec_32k_isr_millis_time_tracker[i] -= base
        rtc_time_tracker[i] -= rtc_base

def calculate_actual_time_datapoints():
    actual_time.extend([0.0] * len(ntp_lib_time_tracker))

    x1 = 0
    y1 = ntp_lib_time_tracker[0]
    x2 = 1
    y2 = ntp_lib_time_tracker[1]
    k = 1.0

    for i in range(0, len(corrector_x)):
        x2 = corrector_x[i]
        y2 = ntp_lib_time_tracker[x2]
        k = (y2-y1)/(rtc_32k_isr_time_tracker[x2]-rtc_32k_isr_time_tracker[x1])#(offset_espmillis_time_tracker[x2] - offset_espmillis_time_tracker[x1])#(y2-actual_time_corrector[i]-y1) #1.0 / (1.0 - (actual_time_corrector[i] / (y2 - y1)))
        for j in range(x1 + 1, x2):
            actual_time[j] = ntp_lib_time_tracker[x1] + k * (ntp_lib_time_tracker[j] - ntp_lib_time_tracker[x1])
        actual_time[x2] = y2
        print(actual_time[x2], y2)
        x1 = x2
        y1 = y2

    for j in range(corrector_x[-1] + 1, len(offset_espmillis_time_tracker)):
        actual_time[j] = ntp_lib_time_tracker[corrector_x[-1]] + k * (ntp_lib_time_tracker[j] - ntp_lib_time_tracker[corrector_x[-1]])

    for i in range(0, len(actual_time)):
        actual_rtc_time.append(int(actual_time[i] / 1000) * 1000)

def timeseries_normalizer():
    for i in range(0, len(offset_espmillis_time_tracker)):
        offset_espmillis_time_tracker[i] = actual_time[i] - offset_espmillis_time_tracker[i]
        ntp_lib_time_tracker[i] = actual_time[i] - ntp_lib_time_tracker[i]
        rtc_32k_isr_time_tracker[i] = actual_time[i] - rtc_32k_isr_time_tracker[i]
        rtc_sec_32k_isr_millis_time_tracker[i] = actual_time[i] - rtc_sec_32k_isr_millis_time_tracker[i]
        rtc_time_tracker[i] = actual_rtc_time[i] - rtc_time_tracker[i]

def plotter():
    zeros = [0.0] * len(actual_time)

    plt.plot(actual_time, ntp_lib_time_tracker, 'o')
    plt.show()
    plt.plot(actual_rtc_time, rtc_time_tracker)
    plt.show()
    plt.plot(actual_time, offset_espmillis_time_tracker)
    plt.show()
    plt.plot(actual_time, rtc_sec_32k_isr_millis_time_tracker)
    plt.show()
    plt.plot(actual_time, rtc_32k_isr_time_tracker)
    plt.show()

    plt.plot(actual_time, offset_espmillis_time_tracker, ntp_lib_time_tracker, rtc_sec_32k_isr_millis_time_tracker, rtc_time_tracker)#, rtc_32k_isr_time_tracker, 'g')
    plt.show()


    plt.plot(zeros, 'b', ntp_lib_time_tracker, 'g')
    plt.show()
    plt.plot(zeros, 'b', rtc_time_tracker, 'r')
    plt.show()
    plt.plot(zeros, 'b', offset_espmillis_time_tracker, 'r')
    plt.show()
    plt.plot(zeros, 'b', rtc_sec_32k_isr_millis_time_tracker, 'r')
    plt.show()
    plt.plot(zeros, 'b', rtc_32k_isr_time_tracker, 'g')
    plt.show()

    plt.plot(zeros, 'b', offset_espmillis_time_tracker, 'r', ntp_lib_time_tracker, 'g', rtc_sec_32k_isr_millis_time_tracker, 'r', rtc_time_tracker, 'g')#, rtc_32k_isr_time_tracker, 'g')
    plt.show()


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("ERROR, wrong number of arguments")
        exit()
    else:
        results_parser(sys.argv[1])
        drop_close_elems()
        remove_base()
        calculate_actual_time_datapoints()
        timeseries_normalizer()
        plotter()