//////////////////////////////////////////////////////////////////////////////////////////
//
//    Arduino code for the MAX30003 WING board
//
//    This example plots the ECG through Arduino Plotter.
//
//    Arduino connections:
//
//  |MAX30003 pin label| Pin Function         |Arduino Connection|
//  |----------------- |:--------------------:|-----------------:|
//  | MISO             | Slave Out            |  D12             |
//  | MOSI             | Slave In             |  D11             |
//  | SCLK             | Serial Clock         |  D13             |
//  | CS               | Chip Select          |  D10              |
//  | VCC              | Digital VDD          |  +5V             |
//  | GND              | Digital Gnd          |  Gnd             |
//  | FCLK             | 32K CLOCK            |  -               |
//  | INT1             | Interrupt1           |  02              |
//  | INT2             | Interrupt2           |  -               |
//
//   This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//   NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
//   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//
/////////////////////////////////////////////////////////////////////////////////////////

#include <SPI.h>
#include "max30003.h"

#define MAX30003_CS_PIN 10

MAX30003 max30003(MAX30003_CS_PIN);

void setup()
{
    Serial.begin(115200); //Serial begin

    pinMode(MAX30003_CS_PIN,OUTPUT);
    digitalWrite(MAX30003_CS_PIN,HIGH); //disable device

    SPI.begin();

    
    bool ret = max30003.max30003ReadInfo();
    if(ret){
      Serial.println("Max30003 read ID Success");
    }else{

      while(!ret){
        //stay here untill the issue is fixed.
        ret = max30003.max30003ReadInfo();
        Serial.println("Failed to read ID, please make sure all the pins are connected");
        delay(10000);
      }
      
    }
    
    
    Serial.println("Initialising the slave device ...");
    max30003.max30003Begin();   // initialize MAX30003
    
}

void loop()
{
    
    max30003.getEcgSamples();   //It reads the ecg sample and stores it to max30003.ecgdata .
    Serial.println(max30003.ecgdata);
    delay(8);
}