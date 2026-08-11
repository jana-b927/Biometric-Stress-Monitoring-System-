import time
from collections import deque

import serial
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# ---------------- Configuration ----------------

PORT = "COM3"
BAUD = 115200
BUFFER_LEN = 100

STATE_NAMES = {
    0: "GREEN",
    1: "YELLOW",
    2: "RED"
}

# ---------------- Data Buffers ----------------

rawPPG = deque(maxlen=BUFFER_LEN)
smoothPPG = deque(maxlen=BUFFER_LEN)
bpm = deque(maxlen=BUFFER_LEN)
ppgState = deque(maxlen=BUFFER_LEN)

rawGSR = deque(maxlen=BUFFER_LEN)
smoothGSR = deque(maxlen=BUFFER_LEN)
baseline = deque(maxlen=BUFFER_LEN)
dip = deque(maxlen=BUFFER_LEN)
gsrState = deque(maxlen=BUFFER_LEN)

rawEMG = deque(maxlen=BUFFER_LEN)
rms = deque(maxlen=BUFFER_LEN)
activation = deque(maxlen=BUFFER_LEN)
emgState = deque(maxlen=BUFFER_LEN)

stressScore = deque(maxlen=BUFFER_LEN)
overallState = deque(maxlen=BUFFER_LEN)


# ---------------- Parse CSV ----------------

def parse_line(line):
    parts = line.strip().split(",")

    if len(parts) != 15:
        return None

    try:
        numbers = []
        for x in parts:
            numbers.append(float(x))

        return numbers
    
    except:
        return None


# ---------------- Main ----------------

ser = serial.Serial(PORT, BAUD, timeout=1)
time.sleep(2)

fig, (axPPG, axGSR, axEMG) = plt.subplots(3, 1, figsize=(9,8))

# PPG
axPPG.set_title("PPG")
ppgRawLine, = axPPG.plot([], [], label="Raw")
ppgSmoothLine, = axPPG.plot([], [], label="Smoothed")
ppgText = axPPG.text(0.02,0.85,"",transform=axPPG.transAxes)
axPPG.legend()

# GSR
axGSR.set_title("GSR")
gsrRawLine, = axGSR.plot([], [], label="Raw")
gsrSmoothLine, = axGSR.plot([], [], label="Smoothed")
gsrBaselineLine, = axGSR.plot([], [], label="Baseline")
gsrText = axGSR.text(0.02,0.85,"",transform=axGSR.transAxes)
axGSR.legend()

# EMG
axEMG.set_title("EMG")
emgRawLine, = axEMG.plot([], [], label="Raw")
emgRMSLine, = axEMG.plot([], [], label="RMS")
emgText = axEMG.text(0.02,0.85,"",transform=axEMG.transAxes)
axEMG.legend()

plt.tight_layout()


def update(frame):

    while ser.in_waiting:

        values = parse_line(ser.readline().decode(errors="ignore"))

        if values is None:
            return

        # ---------- Store Data ----------

        rawPPG.append(values[0])
        smoothPPG.append(values[1])
        bpm.append(values[2])
        ppgState.append(values[3])

        rawGSR.append(values[4])
        smoothGSR.append(values[5])
        baseline.append(values[6])
        dip.append(values[7])
        gsrState.append(values[8])

        rawEMG.append(values[9])
        rms.append(values[10])
        activation.append(values[11])
        emgState.append(values[12])

        stressScore.append(values[13])
        overallState.append(values[14])

    if len(rawPPG) == 0:
        return

    x = range(len(rawPPG))

    # ---------- PPG ----------

    ppgRawLine.set_data(x, rawPPG)
    ppgSmoothLine.set_data(x, smoothPPG)

    axPPG.relim()
    axPPG.autoscale_view()

    ppgText.set_text(
        f"BPM: {bpm[-1]:.1f}\n"
        f"State: {STATE_NAMES[int(ppgState[-1])]}"
    )

    # ---------- GSR ----------

    gsrRawLine.set_data(x, rawGSR)
    gsrSmoothLine.set_data(x, smoothGSR)
    gsrBaselineLine.set_data(x, baseline)

    axGSR.relim()
    axGSR.autoscale_view()

    gsrText.set_text(
        f"Dip: {dip[-1]:.1f}\n"
        f"State: {STATE_NAMES[int(gsrState[-1])]}"
    )

    # ---------- EMG ----------

    emgRawLine.set_data(x, rawEMG)
    emgRMSLine.set_data(x, rms)

    axEMG.relim()
    axEMG.autoscale_view()

    emgText.set_text(
        f"Activation: {activation[-1]:.1f}\n"
        f"State: {STATE_NAMES[int(emgState[-1])]}"
    )

    print(
        f"Stress Score: {stressScore[-1]:.2f}   "
        f"Overall: {STATE_NAMES[int(overallState[-1])]}"
    )


ani = FuncAnimation(fig, update, interval=50)

plt.show()

ser.close()