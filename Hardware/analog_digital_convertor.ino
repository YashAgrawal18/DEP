/*
 Name:    readADCVoltage.ino
 Created: 12/29/2021 6:47:41 PM
 Author:  Carlos Balseiro
 Adapted from the code of openenergymonitor (https://github.com/openenergymonitor/EmonLib/blob/master/EmonLib.cpp#L114)
*/

#define ADC_COUNTS 1023

double calcV(int ch, double offsetV, double SupplyVoltage, double* Vrms, double* Vpp)
{

  unsigned int crossCount = 0;             //Used to measure number of times threshold is crossed.
  unsigned int numberOfSamples = 0;        //This is now incremented
  unsigned int timeout = 500;              //mseconds 
  unsigned int crossings = 10;            // sampling 10 trace crossing 
  double adjustedV;
  double sampleV;
  double startV;
  double sqV;
  double sumV = 0;
  int sumMin = 0, sumMax = 0, max = 0, min = 1023, Chng = 0, maxCount = 0, minCount = 0, lastSample, onLow = 0, onUp = 0, oldChng = 0, stayCount = 5;
  double lastVCross;
  double checkVCross;
  int maxValue = 0;
  int minValue = 1024;
  int i = 0;

  //-------------------------------------------------------------------------------------------------------------------------
  // Start
  //-------------------------------------------------------------------------------------------------------------------------
  numberOfSamples = 0;
  crossCount = 0;
  adjustedV = 0;
  double start = millis();
  sumV = 0;                                       //Reset accumulator
  startV = analogRead(ch);
  lastSample = startV;
  

  //-------------------------------------------------------------------------------------------------------------------------
  // Measurement loop
  //------------------------------------------------------------------------------------------------------------------------- 
  while (crossCount < crossings && (millis() - start) < timeout)     //millis()-start makes sure it doesnt get stuck in the loop if there is an error.
  {
    numberOfSamples++;                       //Count number of times looped.
//-----------------------------------------------------------------------------
// Read in raw voltage samples
//-----------------------------------------------------------------------------
    sampleV = analogRead(ch);
    //---------------------------------------------------------------------------
    // Voltage calculation corrected by offset
    //-----------------------------------------------------------------------------
    adjustedV = sampleV / ADC_COUNTS * SupplyVoltage;
    //Serial.print(adjustedV); Serial.print(" ");
    adjustedV -= offsetV;
    //Serial.println(adjustedV);

    //-----------------------------------------------------------------------------
    // RMS voltage
    //-----------------------------------------------------------------------------  
    sqV = adjustedV * adjustedV;
    sumV += sqV;
    //-----------------------------------------------------------------------------
    // Find the number of times the voltage has crossed the initial voltage
    // - every 2 crosses we will have sampled 1 wavelength 
    // - so this method allows us to sample an integer number of half wavelengths which increases accuracy
    //-----------------------------------------------------------------------------       
    lastVCross = checkVCross;
    if (sampleV > startV) checkVCross = true;
    else checkVCross = false;

    if (numberOfSamples == 1)
      lastVCross = checkVCross;

    if (lastVCross != checkVCross) {
      crossCount++;
    }

    //-----------------------------------------------------------------------------
    // Find max peak and min peak to measure peak to peak voltage
    //-----------------------------------------------------------------------------  
    if (lastSample < sampleV) {               //trace is going up
      if (Chng < 0 && onUp < stayCount)       //If minimum was detected we are going to check there is not a false minimum
        oldChng = Chng;
      if (oldChng < 0 && onUp> stayCount) {   //if we found a minimum and after stayCount samples the trace is going up then recorded the minimum and reset
        sumMin += min;
        min = 1023;
        minCount++;
        oldChng = 0;
        onUp = 0;
        onLow = 0;
      }
      lastSample = sampleV;
      Chng = 1;
      onUp++;
      if (max < sampleV)
        max = sampleV;                      // get max value
    }
    else if (lastSample > sampleV) {        //trace is going down
      if (Chng > 0 && onLow < stayCount)    //If maximum was detected we are going to check there is not a false maximum
        oldChng = Chng;

      if (oldChng > 0 && onLow > stayCount) {   //if we found a maximum and after stayCount samples the trace is going down then recorded the maximum and reset
        sumMax += max;
        max = 0;
        maxCount++;
        oldChng = 0;
        onLow = 0;
        onUp = 0;
      }
      lastSample = sampleV;
      Chng = -1;
      onLow++;
      if (min > sampleV)
        min = sampleV;                      //get min value
    }
    else
      lastSample = sampleV;

  }
  
float discreteV[100];
float signalV=0;
for(int i=0;i<100;i++)
{
  
 signalV = analogRead(A0);
 discreteV[i]=signalV;

  Serial.println("Signal " + String(discreteV[i]));
}
  //-------------------------------------------------------------------------------------------------------------------------
  // Vrms and Vpp calculations
  //------------------------------------------------------------------------------------------------------------------------- 
  //Calculation of the root of the mean of the voltage squared (rms)
  *Vrms = sqrt(sumV / numberOfSamples);
   if (minCount != 0 && maxCount != 0)
    *Vpp = (double)(sumMax / maxCount - sumMin / minCount) / ADC_COUNTS * SupplyVoltage;
  else
    *Vpp = 0;

  if (*Vpp < 0)
    *Vpp = 0;

  //Serial.println("Vrms_Vpp: " + String(Vrms) + " " + String(Vpp));

  //delay(100);

  return;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {};

}

void loop() {

  // Get Voltage   pin, offset, VSupply
  double Vpp, Vrms;
  calcV(0, 2.34, 5.0, &Vrms, &Vpp);

  Serial.println("Vrms: " + String(Vrms) + "  Vpp: " + String(Vpp));

}
