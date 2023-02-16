#include <EEPROM.h>

float tiempo_litro;
int litros;

//Direcciones donde se escriben y leen los datos
int dir_litros = 0;
int dir_tiempo = 10;

void setup() {
  Serial.begin(9600);
  EEPROM.begin(512);

  delay(500);

  //Leemos el valor de la EEPROM
  //Si no hay valor correcto lo ponemos nosotros.
  EEPROM.get(dir_litros, litros);
  if (litros < 1 || litros > 999)
    litros = 10;

  //Leemos el valor de la EEPROM
  //Si no hay valor correcto lo ponemos nosotros.
  EEPROM.get(dir_tiempo, tiempo_litro);
  if (tiempo_litro < 1 || tiempo_litro > 999)
    tiempo_litro = 5.00;

  Serial.print("\n");
  Serial.print("Litros:\t");
  Serial.println(litros);
  Serial.print("Tiempo:\t");
  Serial.println(tiempo_litro);

  //Defino el modo de dos botones para subir y bajar
  pinMode(D5, INPUT_PULLUP);
  pinMode(D6, INPUT_PULLUP);
}

void loop() {

  //----- CON BOTON 1
  if (digitalRead(D5) == LOW && digitalRead(D6) == HIGH) {
    litros++;
    Serial.print("Aumentando litros:\t");
    Serial.println(litros);
    //Escribo en la eeprom:
    EEPROM.put(dir_litros, litros);
    EEPROM.commit();
    //Espero a q suelten el pulsador
    while (digitalRead(D5) == LOW && digitalRead(D6) == HIGH)
      delay(1);
  }

  //----- CON BOTON 2
  if (digitalRead(D6) == LOW && digitalRead(D5) == HIGH) {
    tiempo_litro += 1;
    Serial.print("Aumentando tiempo:\t");
    Serial.println(tiempo_litro);
    //Escribo en la eeprom:
    EEPROM.put(dir_tiempo, tiempo_litro);
    EEPROM.commit();
    //Espero a q suelten el pulsador
    while (digitalRead(D6) == LOW && digitalRead(D5) == HIGH)
      delay(1);
  }

  //----- CON LOS 2 BOTONES
  if (digitalRead(D5) == LOW && digitalRead(D6) == LOW) {
    Serial.print("Reseteando valores...\t");
    litros = 15;
    tiempo_litro = 7.77;
    //Escribo en la eeprom:
    EEPROM.put(dir_tiempo, tiempo_litro);
    EEPROM.commit();
    EEPROM.put(dir_litros, litros);
    EEPROM.commit();
    //Muestro
    Serial.print("Litros:\t");
    Serial.println(litros);
    Serial.print("Tiempo:\t");
    Serial.println(tiempo_litro);
    //Espero
    while (digitalRead(D6) == LOW && digitalRead(D6) == LOW)
      delay(1);
  }


}
