/*The default sensitivity setting of the accelerometer is +/- 2g 
The output is 16 bits. 16 bits equals 65,535.   
This means we can get 65,535 different readings for the range  between -2 and +2. (or -2,000 MilliGs and +2,000 MilliGs)

 4,000 MilliGs / 65,535 = 0.061

Each time the LSB changes by one, the value changes by 0.061, which is the value highlighted in blue in the table above.
*/
#include <Wire.h>
const int MPU_ADDR = 0x68;
const byte N = 20; //Number of reads for the entire pattern
int16_t accelerometer_x, accelerometer_y, accelerometer_z; // variables for accelerometer raw data
int16_t gyro_x, gyro_y, gyro_z; // variables for gyro raw data
int16_t temperature; // variables for temperature data
float a_sen = 0.061;
float temp_acc,temp_gyro;
float acc_x[100],acc_y[100],acc_z[100]; //True accelaration values
float gy_x[100],gy_y[100],gy_z[100]; //True gyro values
float P_t [3][N];
int reccount=0;
float pos[3][5];
float che_pos[3];
int count=0;
bool checkflag=false;
bool recorddone=false;
bool startbool = true;
bool matchbool=false;
//values read for checking match
int16_t rd_accelerometer_x, rd_accelerometer_y, rd_accelerometer_z;
int16_t rd_gyro_x, rd_gyro_y, rd_gyro_z;



float readable_values(int accelaration,int gyro)
{
  
  temp_acc = (accelaration * a_sen)/100;

  return temp_acc;
}


void setup() 
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  
}

void loop() {

  if( startbool==true){
     Serial.println("Enter 's' to start: ");
     delay(3000);
    char startchar = Serial.read();
    Serial.println(startchar);
    if( startchar == 's'){
      int start=millis();
      Serial.println(start);
      while(millis()< start+6000){
        record();
        recorddone=true;
        startbool=false;
        matchbool=true;
      }
      if(recorddone){
        Serial.print("Recorded data");
        Serial.println();
        recorddone=false;
      }
    }
  }
  

  if(matchbool==true){
     Serial.println("Enter 'm' to match: ");
     delay(3000);
    char match = Serial.read();
    if( match == 'm'){
      int timenow=millis();
      Serial.println(timenow);
      while(matchbool==true){
        mapGesture();
        compare();
        matchbool=false;
      }
    }
  }
}


void record()// reading samples for the unlock pattern
{
float sum_x=0;
float sum_y=0;
float sum_z=0 ;
  Serial.println("----------------------------------");
  for(int i =0 ; i < N ; i++)
  {
    
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
    Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
    Wire.requestFrom(MPU_ADDR, 7*2, true); // request a total of 7*2=14 registers
    // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
    accelerometer_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
    accelerometer_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
    accelerometer_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
    temperature = Wire.read()<<8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
    gyro_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
    gyro_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
    gyro_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)
    // print out data
    sum_x = sum_x+readable_values(accelerometer_x,0);
    sum_y = sum_y+readable_values(accelerometer_y,0);
    sum_z = sum_z+readable_values(accelerometer_z,0);
    Serial.print("aX = "); Serial.print(readable_values(accelerometer_x,0));
    Serial.print(" | aY = "); Serial.print(readable_values(accelerometer_y,0));
    Serial.print(" | aZ = "); Serial.print(readable_values(accelerometer_z,0));
    Serial.println();
    // delay
    delay(50); 
  }
  pos[0][count]= sum_x/N;
  pos[1][count]= sum_y/N;
  pos[2][count]= sum_z/N;

for(int j = 0 ; j < 3 ; j++)
{
   Serial.print(" ");Serial.print(pos[j][count]);
}
Serial.println();
  count++;
Wire.endTransmission(true);
delay(500);
}

void mapGesture() //reading actual unlock pattern
{
float sum_x=0;
float sum_y=0;
float sum_z=0 ;
  Serial.println("comparing");
  Serial.println("----------------------------------");
  for(int i =0 ; i < 60 ; i++)
  {
    
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
    Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
    Wire.requestFrom(MPU_ADDR, 7*2, true); // request a total of 7*2=14 registers
    // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
    rd_accelerometer_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
    rd_accelerometer_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
    rd_accelerometer_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
    temperature = Wire.read()<<8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
    rd_gyro_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
    rd_gyro_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
    rd_gyro_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)
    // print out data
    sum_x = sum_x+readable_values(rd_accelerometer_x,0);
    sum_y = sum_y+readable_values(rd_accelerometer_y,0);
    sum_z = sum_z+readable_values(rd_accelerometer_z,0);
    Serial.print("aX = "); Serial.print(readable_values(rd_accelerometer_x,0));
    Serial.print(" | aY = "); Serial.print(readable_values(rd_accelerometer_y,0));
    Serial.print(" | aZ = "); Serial.print(readable_values(rd_accelerometer_z,0));
    Serial.println();
    // delay
    delay(50); 
  }
  che_pos[0]= sum_x/60;
  che_pos[1]= sum_y/60;
  che_pos[2]= sum_z/60;

for(int j = 0 ; j < 3 ; j++)
{
   Serial.print(" ");Serial.print(che_pos[j]);
}
Serial.println();
  
Wire.endTransmission(true);
}

void compare()
{
  float avg_x=0;
  float avg_y=0;
  float avg_z=0 ;
  float trained_accel, test_accel, errX, errY, errZ;
  for(int i=0;i<4;i++)
  {
    avg_x=avg_x+pos[0][i];
    avg_y=avg_y+pos[1][i];
    avg_z=avg_z+pos[2][i];
  }
  avg_x=avg_x/5;
  avg_y=avg_y/5;
  avg_z=avg_z/5;

  errX=(abs(che_pos[0]-avg_x)/avg_x)*100;// error on X axis
  errY=(abs(che_pos[1]-avg_y)/avg_y)*100;// error on Y axis
  errZ=(abs(che_pos[2]-avg_z)/avg_z)*100;// error on Z axis

  if(errX <= 15 && errY <= 15 && errZ <= 15 )
  {
    Serial.println("Password Matches!!");
    while(1)
    {
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(100);                       // wait for a second
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
      delay(100);                       // wait for a second
    }
  }
  else
  {
    Serial.println("No Match");
  }
}
