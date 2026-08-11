//integrated stress detection: PPG + GSR + EMG -> single 3-LED readout

//---- pins ----
const int ppgPin = A1;
const int gsrPin = A2;
const int emgPin = A0;

const int greenLED = 2;
const int yellowLED = 3;
const int redLED = 4;

//========== initialize PPG (heartbeat) ==========
const int ppgWindowSize = 10;  //potentially subject to change * comment
int ppgSamples[ppgWindowSize];
int ppgSampleIndex = 0;
long ppgSampleSum = 0;

float ppgPrev = 0;
float ppgCurr = 0;
float ppgNext = 0;

unsigned long prevPeak = 0;
float bpm = 0;
const int bpmWindow = 5; 
float bpmHistory[bpmWindow];
int bpmIndex = 0;
float bpmSum = 0;
float averageBPM = 0;

const int minPeakDist = 300;
const float minPeakDifference = .05;

const float ppgMaxRest = 90;
const float ppgElev = 120;

int ppgState = 0; //0=green,1=yellow,2=red

//========== initialize GSR ==========
const int gsrWindowSize = 5; //potentially subject to change * comment
int gsrSamples[gsrWindowSize];
int gsrSampleIndex = 0;
long gsrSampleSum = 0;

float gsrBaseline = 0;
const float gsrBaselineAlpha = 0.01; 

const float threshold1_gsr = 0.05; //potentially subject to change * comment
const float threshold2_gsr = 0.1; //potentially subject to change * comment

unsigned long redTime = -10000;
unsigned long yellowTime = -10000;

//========== initialize EMG ==========
const int emgWindowSize = 20; //potentially subject to change * comment
float emgSquaredSamples[emgWindowSize];
int emgSampleIndex = 0;
float emgSquaredSum = 0;

float drift = 0;
const float driftAlpha = 0.01;

float rmsBaseline = 0;

float baselineSum_emg = 0;
int baselineSamples_emg = 0;
const unsigned long baselineTime_emg = 2000;

const float threshold1_emg = 0.3; //potentially subject to change * comment
const float threshold2_emg = 0.6; //potentially subject to change * comment

//========== Sensor Weights ==========
const float gsrWeight = 0.40; //potentially subject to change * comment
const float ppgWeight = 0.35; //potentially subject to change * comment
const float emgWeight = 0.25; //potentially subject to change * comment
float threshold1_overall = .5;
float threshold2_overall = 1.25;
float threshold3_overall = 2;

void setup()
{
    Serial.begin(9600);

    pinMode(greenLED, OUTPUT);
    pinMode(yellowLED, OUTPUT);
    pinMode(redLED, OUTPUT);

    for (int i = 0; i < ppgWindowSize; i++) { ppgSamples[i] = 0; }
    for (int i = 0; i < gsrWindowSize; i++) { gsrSamples[i] = 0; }
    for (int i = 0; i < emgWindowSize; i++) { emgSquaredSamples[i] = 0; }
    for (int i = 0; i < bpmWindow; i++) { bpmHistory[i] = 0; }
}

void loop()
{
    unsigned long now = millis();

    //----Read Sensors ----
    int rawPPG = analogRead(ppgPin);
    int rawGSR = analogRead(gsrPin);
    int rawEMG = analogRead(emgPin);

    //---- Filter Signals ----

    //PPG moving mean
    ppgSampleSum -= ppgSamples[ppgSampleIndex];
    ppgSamples[ppgSampleIndex] = rawPPG;
    ppgSampleSum += ppgSamples[ppgSampleIndex];
    ppgSampleIndex++;
    if (ppgSampleIndex >= ppgWindowSize) {ppgSampleIndex = 0; }
    float ppgSmooth = (float)ppgSampleSum / ppgWindowSize;

    ppgPrev = ppgCurr;
    ppgCurr = ppgNext;
    ppgNext = ppgSmooth;

    //GSR moving mean
    gsrSampleSum -= gsrSamples[gsrSampleIndex];
    gsrSamples[gsrSampleIndex] = rawGSR;
    gsrSampleSum += gsrSamples[gsrSampleIndex];
    gsrSampleIndex++;
    if (gsrSampleIndex >= gsrWindowSize) { gsrSampleIndex = 0; }
    float gsrSmooth = (float)gsrSampleSum / gsrWindowSize;

    //EMG offset removal + RMS
    if (drift == 0) {drift = rawEMG; }
    else { drift = drift + driftAlpha * (rawEMG - drift); }

    float emgCentered = rawEMG - drift;
    float emgSquared = emgCentered * emgCentered;

    emgSquaredSum -= emgSquaredSamples[emgSampleIndex];
    emgSquaredSamples[emgSampleIndex] = emgSquared;
    emgSquaredSum += emgSquaredSamples[emgSampleIndex];
    emgSampleIndex++;
    if (emgSampleIndex >= emgWindowSize) { emgSampleIndex = 0; }
    float emgRMS = sqrt(emgSquaredSum / emgWindowSize);

    //---- Extract Features ----

    //GSR baseline + dip amplitude
    if (gsrBaseline == 0) { gsrBaseline = gsrSmooth; }
    else { gsrBaseline = gsrBaseline + gsrBaselineAlpha * (gsrSmooth - gsrBaseline); }

    float dipAmplitude = gsrBaseline - gsrSmooth;
    if (dipAmplitude < 0) { dipAmplitude = 0; }

    //EMG RMS baseline calibration (first 2 seconds), then fixed baseline
    if (now < baselineTime_emg)
    {
        baselineSum_emg += emgRMS;
        baselineSamples_emg++;
    }
    else if (rmsBaseline == 0)
    {
        rmsBaseline = baselineSum_emg / baselineSamples_emg;
    }

    float activation = emgRMS - rmsBaseline;
    if (activation < 0) {activation = 0;}

    //PPG peak detection + BPM (updates ppgState when a peak completes)
    if (ppgCurr > ppgPrev && ppgCurr > ppgNext &&
        ppgCurr - ppgPrev > minPeakDifference && ppgCurr - ppgNext > minPeakDifference)
    {
        if (now - prevPeak > minPeakDist)
        {
            if (prevPeak != 0)
            {
                unsigned long intv = now - prevPeak;
                bpm = 60000.0 / intv;
                bpmSum -= bpmHistory[bpmIndex];
                bpmHistory[bpmIndex] = bpm;
                bpmSum += bpmHistory[bpmIndex];
                bpmIndex++;
                if (bpmIndex >= bpmWindow) bpmIndex = 0;
                averageBPM = bpmSum / bpmWindow;
            }

            prevPeak = now;

            //---- Classify PPG ----
            if (averageBPM > 0)
            {
                if (averageBPM < ppgMaxRest) ppgState = 0;
                else if (averageBPM < ppgElev) ppgState = 1;
                else ppgState = 2;
            }
        }
    }

    //---- Classify GSR ----
    if (dipAmplitude >= threshold2_gsr * gsrBaseline)
    {
        redTime = now;
    }
    else if (dipAmplitude >= threshold1_gsr * gsrBaseline)
    {
        yellowTime = now;
    }

    //---- GSR Hold Logic ----
    int gsrState;
    if (now - redTime < 3000) gsrState = 2;
    else if (now - yellowTime < 2000) gsrState = 1;
    else gsrState = 0;

    //---- Classify EMG ----
    int emgState;
    if (activation < threshold1_emg * rmsBaseline) emgState = 0;
    else if (activation < threshold2_emg * rmsBaseline) emgState = 1;
    else emgState = 2;

  //---- Weighted Stress Decision ----

    // Calculate weighted stress score
    float stressScore =
    (gsrWeight * gsrState) +
    (ppgWeight * ppgState) +
    (emgWeight * emgState);

    // Convert weighted score into LED state
    int overallState;
    //override overall state if gsr picks up red  //potentially subject to change * comment
    if (gsrState == 2){
        overallState = 2;
    }
    else
{
    
    if (stressScore < threshold1_overall){overallState = 0;}      // Green}
    else if (stressScore < threshold2_overall) {overallState = 1;}      // Yellow}
    else{overallState = threshold3_overall; }     // Red
}
    //---- 7. Update LEDs ----
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);

    if (overallState == 0) {digitalWrite(greenLED, HIGH);}
    else if (overallState == 1) {digitalWrite(yellowLED, HIGH);}
    else {digitalWrite(redLED, HIGH);}

    //---- Send Metrics via Serial ----
    Serial.print(rawPPG);
Serial.print(",");
Serial.print(ppgSmooth);
Serial.print(",");
Serial.print(averageBPM);
Serial.print(",");
Serial.print(ppgState);

Serial.print(",");

Serial.print(rawGSR);
Serial.print(",");
Serial.print(gsrSmooth);
Serial.print(",");
Serial.print(gsrBaseline);
Serial.print(",");
Serial.print(dipAmplitude);
Serial.print(",");
Serial.print(gsrState);

Serial.print(",");

Serial.print(rawEMG);
Serial.print(",");
Serial.print(emgRMS);
Serial.print(",");
Serial.print(activation);
Serial.print(",");
Serial.print(emgState);

Serial.print(",");

Serial.print(stressScore);
Serial.print(",");
Serial.println(overallState);

    delay(10);
}
