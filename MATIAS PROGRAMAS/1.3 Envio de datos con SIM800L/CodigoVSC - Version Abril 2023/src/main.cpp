#include <Arduino.h>

#define PULSADOR D5
#define Led_Salida D0

#define SIM808_TX D3 // D3 CON RX DEL SIM808
#define SIM808_RX D4 // D4 CON TX DEL SIM808
#include <SoftwareSerial.h>
SoftwareSerial SIM808(SIM808_RX, SIM808_TX); // RX, TX

/* Declaracion de funciones */
String ENVIO_HTTP();
String Enviar_Verificar(String, String, String);
void Parpadeo(int, int);

String apn_SIM = "wap.gprs.unifon.com.ar"; // APN: "wap.gprs.unifon.com.ar" FUNCIONA BIEN PARA MOVISTAR!
String apn_user_SIM = "wap";               // APN-Username
String apn_pass_SIM = "wap";               // APN-Password

String URL_POST = "http://data.ambientecontrolado.com.ar/device/measure"; // URL for HTTP-POST-REQUEST
String Sensor_ID = "638727";
String DATOS_POST = "{\"data\":{\"extraData\":{\"chipId\":\"" + Sensor_ID + "\",\"co2\":" + String(444) + ",\"ssid\":\"" + "Awifi" + "\",\"ip\":\"" + "1696969696" + "\",\"signal\":" + "60" + ",\"firmwareVersion\":\"" + "2.0" + "\"}}}";

void setup()
{
  pinMode(PULSADOR, INPUT_PULLUP);
  pinMode(Led_Salida, OUTPUT);

  Serial.begin(9600);
  SIM808.begin(9600);
}

void loop()
{

  if (digitalRead(PULSADOR) == LOW)
  {
    int cont = 0;
    int max = 1500;
    while (digitalRead(PULSADOR) == LOW && cont < max)
    {
      delay(1);
      cont++;
    }

    if (cont == max)
    {
      digitalWrite(Led_Salida, LOW); // Encendemos el LED
      Serial.println("\n\tSe van a enviar datos al servidor!");

      String Respuesta = ENVIO_HTTP();

      if (Respuesta.compareTo("ENVIADO") == 0)
      {
        Serial.print("\n\t--------> Se envio correctamente el POST\n");
        Parpadeo(Led_Salida, 7000);
      }
      else if (Respuesta.compareTo("NO_RESPONDE") == 0)
      {
        Serial.print("\n\tNo se obtienen respuestas de la placa!\n");
        digitalWrite(Led_Salida, HIGH); // Apagamos el LED
      }
      else
      {
        Serial.print("\n\tHubo un error en el envio de comandos!\n");
        digitalWrite(Led_Salida, HIGH); // Apagamos el LED
      }
    }
  }

  delay(1);
}

//---------------------------Funcion envio de comandos!
String ENVIO_HTTP()
{























  //respuesta = Enviar_Verificar("AT+HTTPTERM", "OK", ImprimirSerie);
  //respuesta = Enviar_Verificar("AT+SAPBR=0,1", "OK", ImprimirSerie);


String Comandos[]={  "AT",  "AT+SAPBR=3,1,Contype,GPRS","AT+SAPBR=3,1,APN," + apn_SIM,
                      "AT+SAPBR=3,1,USER," + apn_user_SIM,"AT+SAPBR=3,1,PWD," + apn_pass_SIM,
                      "AT+SAPBR=1,1","AT+SAPBR=2,1","AT+HTTPINIT","AT+HTTPPARA=CID,1",
                      "AT+HTTPPARA=URL," + URL_POST,"AT+HTTPPARA=\"CONTENT\",\"application/json\"",
                      "AT+HTTPDATA=" + (String)DATOS_POST.length() + ",10000",DATOS_POST,
                      "AT+HTTPACTION=1","AT+HTTPTERM","AT+SAPBR=0,1"};


String Respuestas[]={"OK",  "OK","OK","","","","","","","","","","","",""};


  // Vacio puerto serie
  while (SIM808.available() > 0)
  {
    SIM808.read();
    delay(1);
  }

  String ImprimirSerie = "MOSTRAR";
  /*Si ponemos  String ImprimirSerie = "MOSTRAR"; va a ir mostrando en el serie cada comando que envia */

  String respuesta = "";
  String retorno_envio = "ERROR";

  respuesta = Enviar_Verificar("AT+HTTPTERM", "OK", ImprimirSerie);
  respuesta = Enviar_Verificar("AT+SAPBR=0,1", "OK", ImprimirSerie);

  respuesta = Enviar_Verificar("AT", "OK", ImprimirSerie);

  if (respuesta.compareTo("OK") == 0)
  {
    respuesta = Enviar_Verificar("AT+SAPBR=3,1,Contype,GPRS", "OK", ImprimirSerie);
    if (respuesta.compareTo("OK") == 0)
    {
      respuesta = Enviar_Verificar("AT+SAPBR=3,1,APN," + apn_SIM, "OK", ImprimirSerie);
      if (respuesta.compareTo("OK") == 0)
      {
        respuesta = Enviar_Verificar("AT+SAPBR=3,1,USER," + apn_user_SIM, "OK", ImprimirSerie);
        if (respuesta.compareTo("OK") == 0)
        {
          respuesta = Enviar_Verificar("AT+SAPBR=3,1,PWD," + apn_pass_SIM, "OK", ImprimirSerie);
          if (respuesta.compareTo("OK") == 0)
          {
            respuesta = Enviar_Verificar("AT+SAPBR=1,1", "OK", ImprimirSerie);
            if (respuesta.compareTo("OK") == 0)
            {
              respuesta = Enviar_Verificar("AT+SAPBR=2,1", "OK", ImprimirSerie);
              if (respuesta.compareTo("OK") == 0)
              {
                respuesta = Enviar_Verificar("AT+HTTPINIT", "OK", ImprimirSerie);
                if (respuesta.compareTo("OK") == 0)
                {
                  respuesta = Enviar_Verificar("AT+HTTPPARA=CID,1", "OK", ImprimirSerie);
                  if (respuesta.compareTo("OK") == 0)
                  {
                    respuesta = Enviar_Verificar("AT+HTTPPARA=URL," + URL_POST, "OK", ImprimirSerie);
                    if (respuesta.compareTo("OK") == 0)
                    {
                      respuesta = Enviar_Verificar("AT+HTTPPARA=\"CONTENT\",\"application/json\"", "OK", ImprimirSerie);
                      if (respuesta.compareTo("OK") == 0)
                      {
                        respuesta = Enviar_Verificar("AT+HTTPDATA=" + (String)DATOS_POST.length() + ",10000", "DOWNLOAD", ImprimirSerie);
                        if (respuesta.compareTo("OK") == 0)
                        {
                          respuesta = Enviar_Verificar(DATOS_POST, "OK", ImprimirSerie);
                          if (respuesta.compareTo("OK") == 0)
                          {
                            respuesta = Enviar_Verificar("AT+HTTPACTION=1", "OK", ImprimirSerie);
                            if (respuesta.compareTo("OK") == 0)
                            {
                              //**** Aqui es cuando el envio fue correcto!
                              retorno_envio = "ENVIADO";
                              //****
                              respuesta = Enviar_Verificar("AT+HTTPTERM", "", ImprimirSerie);
                              if (respuesta.compareTo("VACIO") == 0)
                              {
                                respuesta = Enviar_Verificar("AT+SAPBR=0,1", "", ImprimirSerie);
                                if (respuesta.compareTo("VACIO") == 0)
                                {
                                  delay(10);
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  if (respuesta.compareTo("NO_RESPONDE") == 0)
    retorno_envio = "NO_RESPONDE";

  return retorno_envio;
}

//--------------------------------------------------------------------------

//-------------- Funcion: Esperar comando del SIM808 -----------------------
/*
   Devuelve un String
   Devuelve "OK" si la respuesta esperada coincide con la que devuelve el SIM800L
   Devuelve "ERROR" si las respuestas no coinciden
   El tercer parametro es para "mostrar" o no el comando que vamos enviando y la
   respuesta en el puerto serie (pantalla de la compu)
*/
String Enviar_Verificar(String texto_enviar, String texto_buscar, String mostrar)
{
  // Declaracion de variables
  String retorno = "ERROR";
  int indice = 0;
  String stringLeido = ""; // Para la lectura del SIM
  int x = 0;               // Para la lectura del SIM
  char c;                  // Para la lectura del SIM
  int tiempo_espera = 2500;
  int j = 0;

  if (mostrar.compareTo("MOSTRAR") == 0)
  {
    Serial.println("");
    Serial.print("Se envia al SIM: " + texto_enviar);
    if (texto_buscar.compareTo("") == 0)
      Serial.print("\tNo se espera recibir nada.\n");
    else
      Serial.print("\tSe espera recibir: " + texto_buscar);
  }

  // Enviar comando AT:
  SIM808.println(texto_enviar);
  delay(20);

  // Espero a que haya un dato:
  while (SIM808.available() == 0 && j < tiempo_espera)
  {
    delay(1);
    j++;
  }

  if (j < tiempo_espera)
  {

    // Leer datos del SIM
    while (x < 1000)
    { // este valor (x < 1000) puede variar

      if (SIM808.available() > 0)
      {
        c = SIM808.read();
        stringLeido += c;
      }
      delay(1);
      x++;
    }
    //----------------------
    if (mostrar.compareTo("MOSTRAR_TODO") == 0)
    {
      Serial.print("Datos leidos serie:\n");
      Serial.print(stringLeido);
    }

    if (texto_buscar.compareTo("") == 0)
    {
      retorno = "VACIO"; // Si texto buscado es un "string vacio" devuelve: "VACIO"
    }
    else
    {
      indice = stringLeido.indexOf(texto_buscar);
      if (indice != -1)
      {
        retorno = "OK";
        if (mostrar.compareTo("MOSTRAR") == 0)
          Serial.println("---> Enviado correctamente");
      }
      else
      {
        retorno = "ERROR";
        if (mostrar.compareTo("MOSTRAR") == 0)
          Serial.println("---> Error!");
      }
    }
  }
  else
  {
    retorno = "NO_RESPONDE";
    if (mostrar.compareTo("MOSTRAR") == 0)
      Serial.print("\nNo se recibe respuesta en el puerto serie\n");
  }
  // Vacio puerto serie
  while (SIM808.available() > 0)
  {
    SIM808.read();
    delay(1);
  }
  return retorno;
}
//--------------- Fin de funcion "Esperar comando"

void Parpadeo(int pin, int time_max)
{
  int delay_ms=100;
  int time_ant = millis();
  int times;
  int diferencia=0;
  while (diferencia < time_max)
  {
    digitalWrite(Led_Salida,LOW);
    delay(delay_ms);
    digitalWrite(Led_Salida,HIGH);
    delay(delay_ms);
    times=millis();
    diferencia=times-time_ant;
  }
}