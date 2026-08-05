//gsr sensor algorithm
//wiring setup pin and leds
const int gsrPin = A0;
const int greenLED = 2;
const int yellowLED = 3;
const int redLED = 4;

//smoothing setup (moving mean)
const int windowSize = 5;
int samples[windowSize];
int sampleIndex = 0;
long sampleSum = 0;

//baseline setup
float baseline = 0;
const float baselineAlpha = 0.01; 

//dip detection setup
float prev = 0;
float curr = 0;
float next = 0;
float threshold1 = .05;
float threshold2 = .1;

//hold setup
unsigned long redTime = -10000;
unsigned long yellowTime = -10000;

void setup()
{
    Serial.begin(9600);


    pinMode(greenLED, OUTPUT);
    pinMode(yellowLED, OUTPUT);
    pinMode(redLED, OUTPUT);


    for(int i = 0; i < windowSize; i++)
    {
        samples[i] = 0;
    }
}

void loop()
{
    int raw = analogRead(gsrPin);

    //smooth thru moving mean
    sampleSum -= samples[sampleIndex];
    samples[sampleIndex] = raw;
    sampleSum += samples[sampleIndex];
    sampleIndex++;

    if(sampleIndex >= windowSize)
        sampleIndex = 0;

    float smoothValue = (float)sampleSum / windowSize;

    //consider first baseline value
    if(baseline == 0)
    {
        baseline = smoothValue;
    }
    //take into account deviations from baseline continuously and only a little bit
    else
    {
        baseline = baseline + baselineAlpha * (smoothValue - baseline);
    }

    //shift values
    prev = curr;
    curr = next;
    next = smoothValue;

    //calculate all dips and their amplitudes
    float dipAmplitude = baseline - curr;
    if(dipAmplitude < 0)
        dipAmplitude = 0;

    //consider thresholds and assign times
    if(dipAmplitude >= threshold2 * baseline)
    {
        redTime = millis();
    }
    else if(dipAmplitude >= threshold1 * baseline)
    {
        yellowTime = millis();
    }

    //leds and thresholds
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);

    //hold red for a time
    if(millis() - redTime < 3000)
    {
        digitalWrite(redLED, HIGH);
    }
    //otherwise hold yellow for a time
    else if(millis() - yellowTime < 2000)
    {
        digitalWrite(yellowLED, HIGH);
    }
    //otherwise green
    else
    {
        digitalWrite(greenLED, HIGH);
    }

    //plot and print
    Serial.print("Raw:");
    Serial.print(raw);
    Serial.print(",");

    Serial.print("Smoothed:");
    Serial.print(smoothValue);
    Serial.print(",");

    Serial.print("Baseline:");
    Serial.print(baseline);
    Serial.print(",");

    Serial.print("Dip:");
    Serial.println(dipAmplitude);

    delay(10);
}
