//heartbeat sensor algorithm
 
//wiring setup for pins and leds 
const int ppgPin = A0;
const int greenLED = 2;
const int yellowLED = 3;
const int redLED = 4;

//smoothing setup (moving mean)
const int windowSize = 10;
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
const int minPeakDist = 300; 
const int minPeakDifference = .75;

//fixed thresholds on known resting or elevated bpms
const int max_rest = 90;
const int elev = 120; 


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
    if(curr > prev &&
       curr > next &&
       curr - prev > minPeakDifference &&
       curr - next > minPeakDifference)
    {
        unsigned long currTime = millis();
    // make sure peaks are far enough apart
        if(currTime - prevPeak > minPeakDist)
        {

            // calculate BPM after first peak
            if(prevPeak != 0)
            {
                unsigned long intv = currTime - prevPeak;

                bpm = 60000.0 / intv;


                // moving average of BPM
                bpmSum -= bpmHistory[bpmIndex];

                bpmHistory[bpmIndex] = bpm;

                bpmSum += bpmHistory[bpmIndex];

                bpmIndex++;

                if(bpmIndex >= bpmWindow)
                    bpmIndex = 0;


                averageBPM = bpmSum / bpmWindow;
            }


            // save current peak time
            prevPeak = currTime;
        }
    }


    // LED thresholds (now runs every loop, not just on peak)
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);

    if(averageBPM > 0)
    {
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


    //plot and print 
    
Serial.print("Raw:");
Serial.print(raw);

Serial.print(",");

Serial.print("Smoothed:");
Serial.println(smoothValue);



    delay(10);
}