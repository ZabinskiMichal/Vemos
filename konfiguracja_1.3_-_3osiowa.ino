//wiblioteka wire sluzy do komunikacji I2C
#include <Wire.h>
#include <TimeLib.h>
/*MPU-6050 gives you 16 bits data so you have to create some 16int constants
 * to store the data for accelerations and gyro*/


int16_t Acc_rawX, Acc_rawY, Acc_rawZ,Gyr_rawX, Gyr_rawY, Gyr_rawZ;

float Acceleration_angle[3];
float Gyro_angle[3];
float Total_angle[3];

float elapsedTime;

float czas = 0;
float timePrev;
int i;
float rad_to_deg = 180/3.141592654;

void setup() {
  delay(5000); // Wait until start. Last chance to upload new sketch!!!
  
  //jawne ustawienie pinów 0 (d3) oraz 2 (d4) ktore będa wykorzystane w komunikacji I2C
  Wire.begin(0,2); //begin the wire comunication

  
  Wire.beginTransmission(0x68); // 0x68 is the i2c address of MPU6050
  Wire.write(0x6B);//funkcja odpowiedzialna za pisanie z czujnika w opowiedzi na zapytanie z mikrokontrolera lub kolejkuje bajty do transmisji z kontrolera do urzadzenia zewnetrznego
  //ox6b decymalnie to 107 argument to rejestr do ktorego chemy dostep

  
  Wire.write(0);
  Wire.endTransmission(true);d
  Serial.begin(115200); //

  czas = millis(); //Start counting time in milliseconds

  Serial.println("IMU check starting...");

}//end of setup void

void loop() {
    timePrev = czas;  // the previous time is stored before the actual time read
    czas = millis();  // actual time read

    //wyznaczenie czasu w milisekundach
    elapsedTime = (czas - timePrev) / 1000; 
    
/////////////////////////////I M U/////////////////////////////////////
  
  /*The tiemStep is the time that elapsed since the previous loop. 
   * This is the value that we will use in the formulas as "elapsedTime" 
   * in seconds. We work in ms so we haveto divide the value by 1000 
   to obtain seconds*/

  /*Reed the values that the accelerometre gives.
   * We know that the slave adress for this IMU is 0x68 in
   * hexadecimal. For that in the RequestFrom and the 
   * begin functions we have to put this value.*/
   
     Wire.beginTransmission(0x68);
     Wire.write(0x3B); //Ask for the 0x3B register- correspond to AcX
     Wire.endTransmission(false);
     Wire.requestFrom(0x68,6,true); 
   
   /*We have asked for the 0x3B register. The IMU will send a brust of register.
    * The amount of register to read is specify in the requestFrom function.
    * In this case we request 6 registers. Each value of acceleration is made out of
    * two 8bits registers, low values and high values. For that we request the 6 of them  
    * and just make then sum of each pair. For that we shift to the left the high values 
    * register (<<) and make an or (|) operation to add the low values.*/
    
     Acc_rawX=Wire.read()<<8|Wire.read(); //each value needs two registres
     Acc_rawY=Wire.read()<<8|Wire.read();
     Acc_rawZ=Wire.read()<<8|Wire.read();

     
    /* Now we can apply the Euler formula. The atan will calculate the arctangent. The
     *  pow(a,b) will elevate the a value to the b power. And finnaly sqrt function
     *  will calculate the rooth square.*/

     //katy pobrane z akcelerometry
     /*---X---*/
     Acceleration_angle[0] = atan((Acc_rawY/16384.0)/sqrt(pow((Acc_rawX/16384.0),2) + pow((Acc_rawZ/16384.0),2)))*rad_to_deg;
     /*---Y---*/
     Acceleration_angle[1] = atan(-1*(Acc_rawX/16384.0)/sqrt(pow((Acc_rawY/16384.0),2) + pow((Acc_rawZ/16384.0),2)))*rad_to_deg;
     
  

   
   Wire.beginTransmission(0x68);
   Wire.write(0x43); //Gyro data first adress
   Wire.endTransmission(false);

   //dane z kazdej osi zajmuja 2 bajty
   Wire.requestFrom(0x68,6,true); //Just 4 registers // zamiana na 6 rejestrow wozwolila na otrzymanie kolejnej osi (Z)
   
   
   Gyr_rawX=Wire.read()<<8|Wire.read(); //Once again we shift and sum
   Gyr_rawY=Wire.read()<<8|Wire.read();
   Gyr_rawZ=Wire.read()<<8|Wire.read();

 
   /*Now in order to obtain the gyro data in degrees/seconds we have to divide first
   the raw value by 131 because that's the value that the datasheet gives us*/


    //obliczenie katkow na podstawie danych z zyroskopu
   /*---X---*/
   Gyro_angle[0] = Gyr_rawX/131.0; 
   /*---Y---*/
   Gyro_angle[1] = Gyr_rawY/131.0;
   //pozycja Z
   Gyro_angle[2] = Gyr_rawZ/131.0;

    //Z zyroskopu bierzemie 98/100 pomiaru, z przyspieszeniomierza tylko 2%

     /*
    dane zebrane z zyroskopu moga byc traktowane jako predkosc katowa, mnozac ta wartosc przez czas trwania jednej iteracji da nam to przebyty kat
     */

      /*---X axis angle---*/
   //Total_angle[0] = 0.98 *(Total_angle[0] + Gyro_angle[0]*elapsedTime) + 0.02*Acceleration_angle[0];
   Total_angle[0] = 0.98 *(Total_angle[0] + Gyro_angle[0]*elapsedTime) + 0.02*Acceleration_angle[0]; // responds faster to changes, no shift over time observed...
   
   /*---Y axis angle---*/
   //Total_angle[1] = 0.98 *(Total_angle[1] + Gyro_angle[1]*elapsedTime) + 0.02*Acceleration_angle[1];
    Total_angle[1] = 0.98 *(Total_angle[1] + Gyro_angle[1]*elapsedTime) + 0.02*Acceleration_angle[1];

    //obliczenie kąta odsepstwa od osi Z na podstawie danych pobranych z akcelerometru oraz zyroskopu
    
    Total_angle[2] = 0.98 *(Total_angle[2] + Gyro_angle[1]*elapsedTime) + 0.02*Acceleration_angle[2];
    Total_angle[2] = 0.98 *(Total_angle[2] + Gyro_angle[2]*elapsedTime) + 0.02*Acceleration_angle[2];
    /*

   Serial.print("Pozycja X: ");
    Serial.print("\t");
    Serial.print("\t");
    Serial.print("\t");
    
    Serial.print("Pozycja Y: ");
    Serial.print("\t");
    Serial.print("\t");
    
    Serial.print("Pozycja Z: ");
    Serial.print("\t");
    Serial.print("\t");
    */

    Serial.print(Gyro_angle[0]);Serial.print("\t");Serial.print("\t");Serial.print("\t");Serial.print(Gyro_angle[1]);Serial.print("\t");Serial.print("\t");Serial.print("\t");Serial.print(Gyro_angle[2]);
    Serial.println();
    
    

    

   
   /*Now we have our angles in degree and values from -10º0 to 100º aprox*/
    //Serial.println(Total_angle[1]);
    
    //Serial.print("X axis angle : "); Serial.print("\t"); Serial.print("\t"); Serial.print("Y axis angle : ");Serial.print("Z axis angle : "); Serial.print("\t"); Serial.print("\t");
    //Serial.println();
    
    //Serial.print(Gyro_angle[0]);Serial.print("\t"); Serial.print("\t"); Serial.print("\t"); Serial.print(Gyro_angle[1]);Serial.print(Gyro_angle[2]);Serial.print("\t"); Serial.print("\t");
    //Serial.println();

    //Serial.print(Gyro_angle[0]);Serial.print("\t"); Serial.print("\t"); Serial.print("\t"); Serial.print(Total_angle[1]);
    //Serial.print(Gyro_angle[2]);Serial.print("\t"); Serial.print("\t"); Serial.print("\t");
    //Serial.println();

    /* left for historic and debugging reasons
    Serial.print("Acc_rawX: "); Serial.print("\t"); Serial.print("Acc_rawY: ");Serial.print("\t"); Serial.println("Acc_rawZ: ");
    Serial.print(Acc_rawX); Serial.print("\t"); Serial.print("\t"); Serial.print(Acc_rawY);Serial.print("\t"); Serial.print("\t"); Serial.println(Acc_rawZ);
    Serial.print("Gyr_rawX: "); Serial.print("\t"); Serial.println("Gyr_rawY: ");
    Serial.print(Gyr_rawX); Serial.print("\t"); Serial.print("\t"); Serial.println(Gyr_rawY);
    Serial.println("------------------------------------------------------------------------------");    
*/
    //keep the delay (loop time) low to minimize the discrete error
    delay(5); // around 5 ms loop time
   
}//end of loop void
