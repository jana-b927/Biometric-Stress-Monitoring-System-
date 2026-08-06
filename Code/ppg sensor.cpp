 //heartbeat sensor algorithm
 
//wiring setup for pins and leds 
const int ppgPin = A0;
const int greenLED = 2;
const int yellowLED = 3;
const int redLED = 4;

//smoothing setup (moving mean)
const int windowSize = 5;
int samples[windowSize];
int sampleIndex = 0;
long sampleSum = 0;

float prev = 0;
float curr = 0;
float next = 0;

//bpm setup
unsigned long prevPeak = 0;
float bpm = 0;
const int bpmWindow = 5;
float bpmHistory[bpmWindow];
int bpmIndex = 0;
float bpmSum = 0;
float averageBPM = 0;

//adaptive threshold setup
float minPeakHeight = 0;
const int minPeakDist = 300; 

const int peakWindow = 5;
float peakSamples[peakWindow];
int peakSampleIndex = 0;
float peakSampleSum = 0;

//fixed thresholds on known resting or elevated bpms
max_rest = 90;
elev = 120; 

void setup()
{
    Serial.begin(9600);

    pinMode(greenLED, OUTPUT);
    pinMode(yellowLED, OUTPUT);
    pinMode(redLED, OUTPUT);

    //initialize moving average
    for(int i = 0; i < windowSize; i++)
    {
        samples[i] = 0;
    }

    //initialize BPM history
    for(int i = 0; i < bpmWindow; i++)
    {
        bpmHistory[i] = 0;
    }

    //initialize peak height history
    for(int i = 0; i < peakWindow; i++)
    {
        peakSamples[i] = 0;
    }
}

void loop()
{
    int raw = analogRead(ppgPin);

    //moving mean to smooth general signal
    sampleSum -= samples[sampleIndex];
    samples[sampleIndex] = raw;
    sampleSum += samples[sampleIndex];
    sampleIndex++;

    if(sampleIndex >= windowSize)
        sampleIndex = 0;

    float smoothValue = (float)sampleSum / windowSize;

    //shift values
    prev = curr;
    curr = next;
    next = smoothValue;

    //detect peaks
    if(curr > prev && curr > next)
    {
        unsigned long currTime = millis();

        if(curr > minPeakHeight && currTime - prevPeak > minPeakDist)
        {

            //initialize peak height history using first detected peak
            if(minPeakHeight < 2)
            {
                for(int i = 0; i < peakWindow; i++)
                {
                    peakSamples[i] = curr;
                    peakSampleSum += curr;
                }
            }

	else {
            //moving mean of recent peak heights
            peakSampleSum -= peakSamples[peakSampleIndex];
            peakSamples[peakSampleIndex] = curr;
            peakSampleSum += peakSamples[peakSampleIndex];
            peakSampleIndex++;

            if(peakSampleIndex >= peakWindow)
                peakSampleIndex = 0;
}
            float avgPeakHeight = peakSampleSum / peakWindow;

            //update adaptive threshold and set next peak height to compare as a fraction of the average
            minPeakHeight = avgPeakHeight * 0.6;

            //calculate average BPM only after first peak exists
            if(prevPeak != 0)
            {
                unsigned long intv = currTime - prevPeak;
                bpm = 60000.0 / intv;
                bpmSum -= bpmHistory[bpmIndex];
                bpmHistory[bpmIndex] = bpm;
                bpmSum += bpmHistory[bpmIndex];
                bpmIndex++;

                if(bpmIndex >= bpmWindow)
                    bpmIndex = 0;


                averageBPM = bpmSum / bpmWindow;
            }

            prevPeak = currTime;

            //LEDs
            digitalWrite(greenLED, LOW);
            digitalWrite(yellowLED, LOW);
            digitalWrite(redLED, LOW);

            //set thresholds 
if(averageBPM > 0) {
            if(averageBPM < max_rest)
            {
                digitalWrite(greenLED, HIGH);
            }
            else if(averageBPM < elev)
            {
                digitalWrite(yellowLED, HIGH);
            }
            else
            {
                digitalWrite(redLED, HIGH);
            }
}
        }
    }

    //plot and print 
    Serial.print("Raw:");
    Serial.print(raw);

    Serial.print(",");

    Serial.print("Smoothed:");
    Serial.print(smoothValue);

    Serial.print(",");

    Serial.print("BPM:");
    Serial.println(averageBPM);


    delay(10);
}
