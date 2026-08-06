# Biometric-Stress-Monitoring-System-
Multi-sensor embedded system using sensors on skin response, heart rate, and muscle tension to monitor, estimate, and display physiological arousal associated with stress. The project utilizes a photoplethysmography sensor (estimates blood volume associated with a heartbeat), galvanic skin response (measures human resistance through skin sweat conductivity), an electromyography sensor (measures muscle electrical activity) and an Arduino microcontroller to process and estimate stress from all three sensors. Metrics contributing to the estimated results are shown in a visual Python display and final results are represented physically through simple LED lights (green for relaxed, yellow for alert, and red for stressed).

Hardware Materials and Budget: 
- Arduino UNO R4 Minima $20
- HiLetgo 5pcs 5V Heartbeat Detect PPG Sensor $7
- Grove Seeed Studio GSR Sensor $12
- EMG Muscle Signal ShillehTek Sensor $20
- 40pcs 20 cm Female to Male Wires $4 

Total budget: ~ $55

  Other materials: 
- 3 led lights (220 resistor for each)
- Male to male wires
- Breadboard
- USB cord 

General algorithm pipeline: 
- Initialize everything
- Read sensor
- Filter signal
- Extract necessary features from signal
- Classify and send to LED
- If using python display print metrics to be parsed and displayed otherwise print and plot on Arduino

Signal processing for each sensor: 
PPM: general smoothing (moving mean), peak detection, bpm calculation, fixed thresholds 

GSR: general smoothing (moving mean), find baseline continuously, find dips, mark threshold as a percentage 
This sensor measures human skin resistance so when its signal dips, the finger is sweatier, conducting electricity easier and leading to less resistance. The GSR sensor has 2 important parts to it, the tonic and phasic parts: the tonic part is the slow baseline drift that occurs in the signal because the person's baseline may change slowly over time, and the phasic part which are the more rapid "dips" in the signal which imply an emotionally triggering or stressful event which the body is responding to. Because of these unique features, an adaptive baseline was necessary in the GSR algorithm in order to consistently keep up with any drifting baseline. That is why after generally smoothing the signal with a simple moving mean, the algorithm takes into account deviations from the previous baseline continuously, updating it by incorporating a fraction of the current signal into the baseline. After calculating the adaptive baseline, any dips are calculated and compared to different fractions of the baseline to categorize the dip as green, yellow, or red (different arbitrary levels of stress according to threshold fractions experimented with during testing). Also, in the case where the algorithm detects a significant level of stress, the LED (yellow or red) is programmed to hold for a few seconds to display the event more clearly. 

EMG: RMS method, find baseline, offset, square, mean, square root, mark threshold as a percentage 
Similar to the GSR sensor, the EMG sensor needed an adaptive baseline, because everyone has a different starting baseline for their body and using fixed values would not transfer well. For the EMG sensor, different compared to the others was the need for the RMS method in filtering and smoothing the signal. This is because the signal is special in that it oscillates around a baseline and the amplitude of that oscillation is how much more muscle tension is occurring meaning it is necessary to first find the general baseline of the signal, remove the offset, square everything to make it positive, apply the moving mean average, then square root. After this the muscle activation can be easily calculated, and the classification of stress is similar to the GSR sensor as being different threshold fractions applied to the difference in the activation compared to their adjusted baseline. 

Integrated Sensors: The integrated sensor algorithm works the same way as the others through the general algorithm pipeline described above. However, the classification of the LEDs includes a weighted stress decision where a stress score is calculated based on 2 factors: the "weight" of each sensor and the current state of each sensor. If at any point the GSR sensor is classified as red, the overall stress state is overridden to red. Else, the weighted calculation takes place. The weight each sensor has is in priority of GSR, PPG, and then EMG. This is because the GSR is more directly associated with sympathetic nervous system activation whereas a heartbeat can be influenced not only by stress but also by events such as exercise or caffeine, and the EMG sensor can be triggered directly through voluntary motion. 

Python Display: The Python display was built to more clearly and easily visualize a chaotic amount of data. The algorithm first parses the data given by the Arduino and plots a graph for each sensor to visualize important information and a final report: the raw and smoothed signals as well as any other calculated features and final metrics such as the baselines, BPM, amplitudes, and overall stress score/ state. 


Future Improvements: 
--> actively working on 
- working on a custom pcb to replace temporary breadboard and actually wearable

--> others to consider
- wireless communication instead of arduino connected to laptop - app to visually display stress less technically (not through graphs) 
- change emg sensor placement and different integration of sensors to be more stress accurate instead of general estimation
- better quality and resolution hardware
- more advanced software filtering
- extract more features from each signal to take into account
- implement more specific and experimented on/ researched thresholds instead of relying one singular individual's testing
- use machine learning to extract features instead of fixed thresholds

Applications: 
- personal and medical stress management thru consistent awareness
- nonverbal individual ability to express simple visual stress levels - inexpensive and accessible
- research on biometric stress estimation 

Limitations: 
- technically stress estimation of physiological arousal instead of direct stress measurement because of many different factors potentially affecting results - excessive movement, exercise, caffeine, excitement etc. 
- sensor accuracy (cheaply and simply made)
- movement makes signals very noisy and placement of sensors on the body also affects results
- limited memory
- estimated thresholds based on personal testing - could need adjustment for different people 

