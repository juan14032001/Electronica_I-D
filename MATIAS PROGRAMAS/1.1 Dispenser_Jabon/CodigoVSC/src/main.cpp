#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

/*
Recordar (para centrar en la OLED):
en la pantalla OLED el ancho de los caracteres (y el espacio) es:
display.setTextSize(2); ---> tamaño 12 (letra y espacio)
display.setTextSize(1); ---> tamaño 6 (letra y espacio)
*/

#define PIN_SALIDA D8 // Colocar en "D3" el pin que queremos utilizar como salida
#define RETARDO_REINICIO 2000

#define ANCHO_PANTALLA 128 // Ancho pantalla OLED
#define ALTO_PANTALLA 64   // Alto pantalla OLED
Adafruit_SSD1306 display(ANCHO_PANTALLA, ALTO_PANTALLA, &Wire, -1);

#define CANT_BOTONES 4
//{ Subir , Bajar , ON , OFF }
int BOTONES[CANT_BOTONES] = {D5, D6, D7};

#define BOTON_SUBIR 0
#define BOTON_BAJAR 1
#define BOTON_ON 2
int litros = 5;
float tiempo_litro;
int dir_litros = 5;
int dir_tiempo = 0;
/*
int velocidad_motor;
int dir_velocidad_motor=10;
*/

// Declaracion de funciones:
void Funcion_Boton_Retardo(int, int);
void Funcion_Boton_Subir(int, int);
void Funcion_Boton_Bajar(int, int);
void Pantalla_Menu();
void Funcion_Pantalla(int);
void Funcion_Menu();
void Subir_Tiempo();
void Bajar_Tiempo();
void Funcion_Boton_Inicio();
void Funcion_Pantalla_Fin();
void Funcion_Pantalla_Con_Reloj(int, int, int, int);
void Funcion_Pantalla_Pausada(int, int, int);
int Funcion_Boton_Retardo2(int);
// Fin de declaracion de funciones

void setup()
{
  Serial.begin(9600);
  EEPROM.begin(512);
  pinMode(D5, INPUT_PULLUP);
  pinMode(D6, INPUT_PULLUP);
  pinMode(D7, INPUT_PULLUP);

  pinMode(PIN_SALIDA, OUTPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  EEPROM.get(dir_litros, litros);
  if (litros < 1 || litros > 999)
    litros = 10;

  EEPROM.get(dir_tiempo, tiempo_litro);
  if (tiempo_litro < 1 || tiempo_litro > 999)
    tiempo_litro = 1.25;

  /*
    EEPROM.get(dir_velocidad_motor, velocidad_motor);
    if (velocidad_motor < 0 || velocidad_motor > 255)
      velocidad_motor=164;
  */
}

void loop()
{
  Funcion_Pantalla(litros);
  Funcion_Boton_Retardo(BOTON_SUBIR, BOTON_BAJAR);
  Funcion_Boton_Inicio();
}

//******************************************
//--- Funcion para leer el estado del botón CON/SIN RETARDO.
//--- La funcion devuelve 1 si se presiona el botón
//--- La funcion devuelve 2 si me mantiene presionado un
// tiempo mayor a "retardo_ms"
//--- devuelve 0 si no está presionado.
void Funcion_Boton_Retardo(int boton1, int boton2)
{

  int retardo_ms = 1500; // Cantidad de milisegundos que quiero
  // que espere cuando mantengo presionado el boton.
  int dos;
  dos = 0;

  // Miro si estan ambos presionados al mismo tiempo
  if (digitalRead(BOTONES[boton1]) == LOW && digitalRead(BOTONES[boton2]) == LOW)
  {
    while (digitalRead(BOTONES[boton1]) == LOW && digitalRead(BOTONES[boton2]) == LOW && dos < retardo_ms)
    {
      delay(1);
      dos++;
    }
    if (dos == retardo_ms)
    { // Se mantuvieron los dos presionados x segundos
      // carpeta menu
      Funcion_Menu();
    }
  }
  else if (digitalRead(BOTONES[boton1]) == LOW)
  {
    Funcion_Boton_Subir(boton1, boton2);
  }
  else if (digitalRead(BOTONES[boton2]) == LOW)
  {
    Funcion_Boton_Bajar(boton1, boton2);
  }
  delay(50);
  //*****************
  //} //cierre del if del boton presionado.
}
//--- Fin de la función

//************ FUNCION SUBIR ***********************
void Funcion_Boton_Subir(int boton1, int boton2)
{
  int flag = 0;
  int retardo = 400;
  int uno = 0;
  int x, i;
  i = 0;
  x = 0;

  //*** Subo litros +1
  if (litros < 999)
  {
    litros++;
    Funcion_Pantalla(litros);
  }
  //***

  while (digitalRead(BOTONES[boton1]) == LOW && digitalRead(BOTONES[boton2]) == HIGH && uno < retardo)
  {
    delay(1);
    uno++;
  }

  if (uno == retardo)
  { // Se presiono uno solo
    // cont automatico.

    //****************************** conteo automatico
    while (digitalRead(BOTONES[boton1]) == LOW && flag == 0)
    {
      delay(1);
      i++;
      if (digitalRead(BOTONES[boton1]) == LOW && digitalRead(BOTONES[boton2]) == LOW)
      {
        flag = 1;
        Funcion_Boton_Retardo(boton1, boton2);
      }
      else
      {
        if (i > 1000 && i < 3000)
        {
          x++;
          if (x == 300)
          {
            if (litros < 999)
              litros++;
            Funcion_Pantalla(litros);
            x = 0;
          }
        }
        if (i > 3000 && i < 8000)
        {
          if (x > 170)
            x = 0;
          x++;
          if (x == 170)
          {
            if (litros < 999)
              litros++;
            Funcion_Pantalla(litros);
            x = 0;
          }
        }
        if (i > 8000)
        {
          if (x > 100)
            x = 0;
          x++;
          if (x == 100)
          {
            if (litros < 999)
              litros++;
            Funcion_Pantalla(litros);
            x = 0;
          }
        }
      } // llave else
    }   // llave while

    //******************************
  } // if uno>400
} // cierre funcion

//**************************************************
//************ FUNCION BAJAR ***********************
void Funcion_Boton_Bajar(int boton1, int boton2)
{
  int flag = 0;
  int retardo = 400;
  int uno = 0;
  int x, i;
  i = 0;
  x = 0;

  //*** Resto litros -1
  if (litros > 1)
  {
    litros--;
    Funcion_Pantalla(litros);
  }
  //***

  while (digitalRead(BOTONES[boton2]) == LOW && digitalRead(BOTONES[boton1]) == HIGH && uno < retardo)
  {
    delay(1);
    uno++;
  }

  if (uno == retardo)
  { // Se presiono uno solo
    // cont automatico.

    //*** conteo automatico
    while (digitalRead(BOTONES[boton2]) == LOW && flag == 0)
    {
      delay(1);
      i++;
      if (digitalRead(BOTONES[boton1]) == LOW && digitalRead(BOTONES[boton2]) == LOW)
      {
        flag = 1;
        Funcion_Boton_Retardo(boton1, boton2);
      }
      else
      {
        if (i > 1000 && i < 3000)
        {
          x++;
          if (x == 300)
          {
            if (litros > 1)
              litros--;
            Funcion_Pantalla(litros);
            x = 0;
          }
        }
        if (i > 3000 && i < 8000)
        {
          if (x > 170)
            x = 0;
          x++;
          if (x == 170)
          {
            if (litros > 1)
              litros--;
            Funcion_Pantalla(litros);
            x = 0;
          }
        }
        if (i > 8000)
        {
          if (x > 100)
            x = 0;
          x++;
          if (x == 100)
          {
            if (litros > 1)
              litros--;
            Funcion_Pantalla(litros);
            x = 0;
          }
        }
      } // llave else
    }   // llave while

    //************
  } // if uno>400
} // cierre funcion

//***************************************************
//***************************************************
void Pantalla_Menu()
{
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(25, 5);
  display.print("CONFIGURACION");

  // Dibujamos Linea.
  display.drawLine(0, 20, 128, 20, SSD1306_WHITE);

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(35, 30);
  display.print(tiempo_litro);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 50);
  display.print("segundos por litro.");

  display.display();
}
//***********************************************
void Funcion_Pantalla(int litros)
{
  //---------
  display.clearDisplay();

  // Titulo:
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 0);
  display.print("ECLIPSE");

  // Dibujamos Linea.
  display.drawLine(0, 18, 128, 18, SSD1306_WHITE);

  // Litros:
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 28);
  display.print(litros);

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(55, 28);
  if (litros > 1)
    display.print("LITROS");
  if (litros == 1)
    display.print("LITRO");

  // Segundos por litro
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 55);
  display.print(tiempo_litro);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(55, 55);
  display.print("seg / litro");

  display.display();
}
//--- Fin de la función
//***********************************************
//*** Funcion Menu ***
void Funcion_Menu()
{
  Pantalla_Menu();
  while (digitalRead(BOTONES[BOTON_SUBIR]) == LOW || digitalRead(BOTONES[BOTON_BAJAR]) == LOW)
    delay(1);

  while (digitalRead(BOTONES[BOTON_ON]) != LOW)
  {
    Subir_Tiempo();
    delay(15);
    Bajar_Tiempo();
    delay(15);
  }

  while (digitalRead(BOTONES[BOTON_ON]) == LOW)
    delay(1);
  Funcion_Pantalla(litros);
  // Guardar valor en la EEPROM:
  EEPROM.put(dir_tiempo, tiempo_litro);
  EEPROM.commit();
}
//*********************
//************ FUNCION SUBIR TIEMPO ***********************
void Subir_Tiempo()
{
  int i, x = 0;
  float paso = 0.01;
  //******
  i = 0;
  if (digitalRead(BOTONES[BOTON_SUBIR]) == LOW)
  {
    
    tiempo_litro += paso;
    Pantalla_Menu();

    while (digitalRead(BOTONES[BOTON_SUBIR]) == LOW && i < 500)
    {
      delay(1);
      i++;
    }
    //******
    if (i == 500)
    {
      while (digitalRead(BOTONES[BOTON_SUBIR]) == LOW)
      {
        delay(1);
        i++;

        if (i > 1000 && i < 3000)
        {
          x++;
          if (x == 70)
          {
            tiempo_litro += paso;
            Pantalla_Menu();
            x = 0;
          }
        }
        if (i > 3000 && i < 8000)
        {
          if (x > 25)
            x = 0;
          x++;
          if (x == 25)
          {
            tiempo_litro += paso;
            Pantalla_Menu();
            x = 0;
          }
        }
        if (i > 8000)
        {
          if (x > 5)
            x = 0;
          x++;
          if (x == 5)
          {
            tiempo_litro += paso;
            Pantalla_Menu();
            x = 0;
          }
        }
      }
    }
    while (digitalRead(BOTONES[BOTON_SUBIR]) == LOW)
      delay(1);
  }

} // llave del if principal

//--- Fin de la función
//***********************************************
//************ FUNCION BAJAR TIEMPO ***********************
void Bajar_Tiempo()
{
  int i, x = 0;
  float paso = 0.01;
  //******
  i = 0;
  float minimo = 0.11;
  if (digitalRead(BOTONES[BOTON_BAJAR]) == LOW)
  {
    if (tiempo_litro > minimo)
      tiempo_litro -= paso;
    Pantalla_Menu();

    while (digitalRead(BOTONES[BOTON_BAJAR]) == LOW && i < 500)
    {
      delay(1);
      i++;
    }
    //******
    if (i == 500)
    {
      while (digitalRead(BOTONES[BOTON_BAJAR]) == LOW)
      {
        delay(1);
        i++;

        if (i > 1000 && i < 3000)
        {
          x++;
          if (x == 70)
          {
            if (tiempo_litro > minimo)
              tiempo_litro -= paso;
            Pantalla_Menu();
            x = 0;
          }
        }
        if (i > 3000 && i < 8000)
        {
          if (x > 25)
            x = 0;
          x++;
          if (x == 25)
          {
            if (tiempo_litro > minimo)
              tiempo_litro -= paso;
            Pantalla_Menu();
            x = 0;
          }
        }
        if (i > 8000)
        {
          if (x > 5)
            x = 0;
          x++;
          if (x == 5)
          {
            if (tiempo_litro > minimo)
              tiempo_litro -= paso;
            Pantalla_Menu();
            x = 0;
          }
        }
      }
    }
    while (digitalRead(BOTONES[BOTON_BAJAR]) == LOW)
      delay(1);
  }
} // llave del if principal

//--- Fin de la función

//********************************************
//*** FUNCION BOTON INICIO ***
void Funcion_Boton_Inicio()
{
  // Cuando se pulsa el botón comenzar
  int frenar = 0, pausa = 0;
  float tiempo_restante = 0;
  int minutos = 0;
  int segundos = 0;
  float centesimos = 0;

  if (digitalRead(BOTONES[BOTON_ON]) == LOW)
  {
    EEPROM.put(dir_litros, litros);
    EEPROM.commit();

    tiempo_restante = (float)tiempo_litro * litros;

    minutos = (int)tiempo_restante / 60;

    segundos = (int)(((tiempo_restante / 60) - minutos) * 60);

    centesimos = (int)(((((tiempo_restante / 60) - minutos) * 60) - segundos) * 100);

    /*
    Tiempo por litro: 3,37
    Litros: 26
    float tiempo restante(segundos) = 87.62
    Minutos: 1.4603333

    Segundos: tiempo_restante / 60) - minutos) * 60
    Segundos: 27,62

    Segundos (int) : 27

    centesimos: [segundos- segundos(int)]*100
    centesimos:62

    Final: 1:27:62
    */

    Serial.print("Litros:");
    Serial.println(litros);
    Serial.print("Tiempo por litro:");
    Serial.println(tiempo_litro);
    Serial.print("Tiempo_restante:");
    Serial.println(tiempo_restante);
    Serial.print("Minutos:");
    Serial.println(minutos);
    Serial.print("Segundos:");
    Serial.println(segundos);
    Serial.print("Centesimos:");
    Serial.println(centesimos);

    Funcion_Pantalla_Con_Reloj(litros, minutos, segundos, centesimos);

    /* Espero a que se suelte el boton para continuar */
    while (digitalRead(BOTONES[BOTON_ON]) == LOW)
      delay(1);

    //------- IMPORTANTE ---------------
    int tiempo;
    int tiempo_ant;
    float diferencia;

    while ((frenar == 0) && tiempo_restante > 0)
    {

      tiempo_ant = millis();
      digitalWrite(PIN_SALIDA, HIGH); // Enciendo la bomba
      // Decremento del tiempo
      if (centesimos == 0)
      {
        centesimos = 99;
        if (segundos > 0)
        {
          segundos--;
        }
        else if (segundos == 0)
        {
          if (minutos > 0)
          {
            minutos--;
            segundos = 59;
          }
          else if (minutos == 0)
          {
            tiempo_restante = 0;
            digitalWrite(PIN_SALIDA, LOW); // APAGAR
          }
        }
      }
      //---- cierre del if

      Funcion_Pantalla_Con_Reloj(litros, minutos, segundos, centesimos);
      diferencia = 0;
      //-------------------------
      while (frenar == 0 && centesimos > 0 && tiempo_restante > 0)
      {

        // Nos fijamos el tiempo:
        tiempo = millis();
        diferencia += (tiempo - tiempo_ant);
        tiempo_ant = millis();
        //----

        /*
          milisegundos = 915
          centesimos = 91.5

          milisegundos = 10
          centesimos = 1

        */
        Serial.print("\n Cent:" + String(centesimos) + " DifIn: " + String(diferencia) + " Tant: " + String(tiempo_ant) + " T: " + String(tiempo));

        if (centesimos > (diferencia / 10))
        {
          centesimos -= (diferencia / 10);
          diferencia = 0;
          Funcion_Pantalla_Con_Reloj(litros, minutos, segundos, centesimos);
        }
        else
        {
          centesimos = 0;
        }
        /* Verificamos si se presiono el boton pausa: */
        pausa = 0;
        pausa = Funcion_Boton_Retardo2(BOTON_ON);
        if (pausa == 1)
        { // Se presiono sin retardo
          pausa = 0;
          while (pausa == 0)
          {
            // Se presionó el botón de pausa.
            digitalWrite(PIN_SALIDA, LOW);
            Funcion_Pantalla_Pausada(minutos, segundos, centesimos);
            pausa = Funcion_Boton_Retardo2(BOTON_ON);
          }
          if (pausa == 1)
          {
            digitalWrite(PIN_SALIDA, HIGH); // Enciendo la bomba
          }
        }
        if (pausa == 2)
        {
          frenar = 1;
          digitalWrite(PIN_SALIDA, LOW); // APAGO LA BOMBA
          Funcion_Pantalla_Fin();
        }
        /* Terminamos de verificar boton pausa*/
      }

      Serial.print("\n Afuera del while");

    }                              // Llave del while
    digitalWrite(PIN_SALIDA, LOW); // APAGO LA BOMBA
    Funcion_Pantalla_Fin();
    delay(2500); // Muestro "FIN" en la pantalla por este tiempo.
    frenar = 0;
    
  } // Cierre del if
}

//--- Funcion para mostrar "FIN" en la pantalla.
void Funcion_Pantalla_Fin()
{
  display.clearDisplay();
  display.setTextSize(5);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 10);
  display.print("FIN");
  display.display();
}
//--- Fin de la función

//--- Función para mostrar la pantalla con el contador.
void Funcion_Pantalla_Con_Reloj(int litros, int minutos, int segundos, int centesimos)
{
  display.clearDisplay();
  //-
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Llenando:  ");
  display.print(litros);
  if (litros > 1)
    display.print(" litros");
  if (litros == 1)
    display.print(" litro");

  // Segundos por litro
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 15);
  display.print(tiempo_litro);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(55, 15);
  display.print("seg / litro");

  // TIEMPO RESTANTE
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 30);
  display.print("Tiempo restante:");
  // RELOJ
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  /* Armamos un string aux */
  String aux_tiempo = "";
  int indice_x = 0;
  int tam;

  if (minutos < 10)
    aux_tiempo += "0";
  aux_tiempo += minutos;
  aux_tiempo += ":";
  if (segundos < 10)
    aux_tiempo += "0";
  aux_tiempo += segundos;
  aux_tiempo += ":";
  if (centesimos < 10)
    aux_tiempo += "0";
  aux_tiempo += centesimos;

  // centramos:
  tam = aux_tiempo.length();
  indice_x = 64 - (12 * (int(tam / 2))); // 12 porque es caracter es tamaño "2"
  /* Terminamos el string */

  /*
    Serial.print("\n Aux_tiempo: "+aux_tiempo);
    Serial.print("\n indice_x: "+String(indice_x));
    Serial.print("\n tam: "+String(tam));
  */

  display.setCursor(indice_x, 42);

  display.print(aux_tiempo);
  // Enviar a pantalla
  display.display();
  return;
}
//--- Fin de la función

//--- Función que muestra la pantalla con el sistema pausado.
void Funcion_Pantalla_Pausada(int minutos, int segundos, int centesimos)
{
  display.clearDisplay();
  //-
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Sistema pausado!");
  display.println("Presione:");
  display.print(" ON para continuar");
  // TIEMPO RESTANTE
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 30);
  display.print("Tiempo restante:");
  // RELOJ
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  /* Armamos un string aux */
  String aux_tiempo = "";
  int indice_x = 0;
  int tam;

  if (minutos < 10)
    aux_tiempo += "0";
  aux_tiempo += minutos;
  aux_tiempo += ":";
  if (segundos < 10)
    aux_tiempo += "0";
  aux_tiempo += segundos;
  aux_tiempo += ":";
  if (centesimos < 10)
    aux_tiempo += "0";
  aux_tiempo += centesimos;

  // centramos:
  tam = aux_tiempo.length();
  indice_x = 64 - (12 * (int(tam / 2))); // 12 porque es caracter es tamaño "2"
  /* Terminamos el string */

  /*
    Serial.print("\n Aux_tiempo: "+aux_tiempo);
    Serial.print("\n indice_x: "+String(indice_x));
    Serial.print("\n tam: "+String(tam));
  */

  display.setCursor(indice_x, 42);

  display.print(aux_tiempo);
  // Enviar a pantalla
  display.display();
  return;
}
//--- Fin de la función
//******************************************

//--- Funcion para leer el estado del botón CON RETARDO.
//--- La funcion devuelve 1 si se presiona el botón
//--- La funcion devuelve 2 si me mantiene presionado un
// tiempo mayor a "retardo_ms"
//--- devuelve 0 si no está presionado.

int Funcion_Boton_Retardo2(int codigo_boton)
{

  int retardo_ms = RETARDO_REINICIO;
  int boton_presionado = 0, retorno = 0;
  int i;

  //******
  i = 0;

  if (digitalRead(BOTONES[codigo_boton]) == LOW)
  {
    boton_presionado = 1;
    while (digitalRead(BOTONES[codigo_boton]) == LOW && i < retardo_ms)
    {
      delay(1);
      i++;
    }
  }
  // Sin retardo
  if (boton_presionado == 1 && i < retardo_ms)
  {
    boton_presionado = 0;
    retorno = 1;
    while (digitalRead(BOTONES[codigo_boton]) == LOW)
      delay(1);
  }
  // Con Retardo
  if (boton_presionado == 1 && i == retardo_ms)
  {
    boton_presionado = 0;
    retorno = 2;
    // while (digitalRead(BOTONES[codigo_boton]) == LOW)
    delay(1);
  }
  return retorno;
}
//--- Fin de la función
//***********************************************