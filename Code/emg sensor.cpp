 
 //emg sensor algorithm
 //wiring setup
const int emgPin = A0;
const int greenLED = 2;
const int yellowLED = 3;
const int redLED = 4;

//RMS setup
const int windowSize = 20;
float squaredSamples[windowSize];
int sampleIndex = 0;
float squaredSum = 0;

//wandering baseline signal drift setup
float drift = 0;
const float driftAlpha = 0.01;

//RMS baseline setup
float rmsBaseline = 0;
const float rmsBaselineAlpha = 0.01;

//threshold setup
float threshold1 = 0.3;
float threshold2 = 0.6;

void setup()
{
    Serial.begin(9600);

    pinMode(greenLED, OUTPUT);
    pinMode(yellowLED, OUTPUT);
    pinMode(redLED, OUTPUT);

    //initialize RMS window
    for(int i = 0; i < windowSize; i++)
    {
        squaredSamples[i] = 0;
    }
}

void loop()
{
    int raw = analogRead(emgPin);

    //continuous drift baseline
    //take into account first case
    if(drift == 0)
    {
        drift = raw;
    }
    //take into account next signal into baseline 
    else
    {
        drift = drift + driftAlpha * (raw - drift);
    }

    //remove offset
    float centeredSignal = raw - drift;

    //square signal
    float squaredSignal = centeredSignal * centeredSignal;

    //moving RMS
    squaredSum -= squaredSamples[sampleIndex];
    squaredSamples[sampleIndex] = squaredSignal;
    squaredSum += squaredSamples[sampleIndex];

    sampleIndex++;

    if(sampleIndex >= windowSize)
        sampleIndex = 0;

    float rmsValue = sqrt(squaredSum / windowSize);

    //continuous RMS baseline
    if(rmsBaseline == 0)
    {
        rmsBaseline = rmsValue;
    }
    else
    {
        rmsBaseline = rmsBaseline + rmsBaselineAlpha * (rmsValue - rmsBaseline);
    }

    //muscle activation
    float activation = rmsValue - rmsBaseline;

    if(activation < 0)
        activation = 0;

    //LEDs
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);

    if(activation < threshold1 * rmsBaseline)
    {
        digitalWrite(greenLED, HIGH);
    }
    else if(activation < threshold2 * rmsBaseline)
    {
        digitalWrite(yellowLED, HIGH);
    }
    else
    {
        digitalWrite(redLED, HIGH);
    }

    //plot and print
    Serial.print("Raw:");
    Serial.print(raw);
    Serial.print(",");

    Serial.print("RMS:");
    Serial.print(rmsValue);
    Serial.print(",");

    Serial.print("Baseline:");
    Serial.print(rmsBaseline);
    Serial.print(",");

    Serial.print("Activation:");
    Serial.println(activation);

    delay(10);
}
 



