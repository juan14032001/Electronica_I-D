#include <Arduino.h>

/* PANTALLA NEXTION */
#define NEXTION_TX D0 // D0 CON RX DEL SIM808
#define NEXTION_RX D8 // D8 CON TX DEL SIM808
#include <SoftwareSerial.h>
SoftwareSerial Nextion(NEXTION_RX, NEXTION_TX); // RX, TX
void Enviar_Nextion(String);
void Nex_Totem_Conectado();
void Nex_Totem_Desconectado();
void Nex_Totem_EstaConectando();
void Nex_Totem_ErrorConexion();

/* PANTALLA OLED */
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define ANCHO_PANTALLA 128 // Ancho pantalla OLED
#define ALTO_PANTALLA 64   // Alto pantalla OLED
Adafruit_SSD1306 display(ANCHO_PANTALLA, ALTO_PANTALLA, &Wire, -1);
void Pantalla_Conectando();
void Pantalla_conectado();
void Pantalla_desconectado();
void Limpiar_display();
void Pantalla_Alerta(int);
void Pantalla_error_conexion();
/* FIN DE PANTALLA OLED */

// ID del sensor
const String SensorID = String(ESP.getChipId(), HEX);
int flag = 0;
int flag1 = 0;

//---> EEMPROM
#include <EEPROM.h>

//---> WIFI
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
ESP8266WebServer Servidor(80); /*el 80 es habitual, podes usar cualquier otro */

//---> HTTP
#include <WiFiClient.h>        //Para poder iniciar la peticion
#include <ESP8266HTTPClient.h> //Se encarga de las peticiones
const char *link = "http://httpbin.org/get";

int dir_ssid = 0;
int dir_pass = 50;
String Estado_red = "";

//---> Nombres y contraseñas de las redes
#define TAM_SSID 50
#define TAM_PASS 50
char ssid[TAM_SSID];
char pass[TAM_PASS];
// String ssid;
// String pass;
const char *ssidConf = "Wifi_Configuracion";
const char *passConf = "12345678";

//---> Variables aux
int time_ant = 0;

//---> NEOPIXEL
#include <Adafruit_NeoPixel.h>
#define PIN D3                                                  // Pin donde estan conectados los leds
#define NUMPIXELS 10                                            // cantidad de pixeles qie tenemos conectados en seria
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800); // creamos el objeto

// Pines para los pulsadores:
#define BOTON_1 D5
#define BOTON_2 D6
#define BOTON_3 D7

// Declaracion de funciones:
int Funcion_Boton_Retardo(int, int);
void Lectura_Botones();
void Pagina_raiz();
void Pagina_wifi();
void Conectar_wifi();
void Escanear_redes();
void Conectar_nueva_red();
void Funcion_prueba();
void Configurar_servidor();
void Leer_EEPROM();
void Envio_Servidor(String);
void Luces_sin_wifi();

//---> Pagina HTML:
String Pagina_html = "";
String Pagina_html_fin = "</body>"
                         "</html>";
String mensaje_html = "";

//------------------------------------------ SETUP
void setup()
{
  //---> Iniciamos SERIE:
  Serial.begin(9600);
  // Serial.print("\n Iniciando...");

  //---> Nextion
  Nextion.begin(9600);
  Enviar_Nextion("0");
  Enviar_Nextion("page Iniciar");

  //---> Pantalla OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  //---> NeoPixel:
  pixels.begin(); // Inicializamos el objeto "pixels"

  pixels.setBrightness(5);
  pixels.fill(pixels.Color(100, 100, 100), 0, 10);
  pixels.show();

  //  pixels.clear(); // Apaga todos los leds
  //  pixels.show();  // Mostrar en los leds

  //---> Configuracion de pulsadores
  pinMode(BOTON_1, INPUT_PULLUP);
  pinMode(BOTON_2, INPUT_PULLUP);
  pinMode(BOTON_3, INPUT_PULLUP);

  //---> Configuracion de WIFI
  Conectar_wifi(); // La funcion EEPROM esta dentro de Conectar_wifi
  Configurar_servidor();
}

//----------------------------- LOOP
void loop()
{

  int time = millis();

  Servidor.handleClient(); /* Recibe las peticiones */
  Lectura_Botones();
  delay(10);

  if ((time - time_ant) > 5000)
  {
    Luces_sin_wifi();
    time_ant = time;
  }
}

void Luces_sin_wifi()
{
  if (flag1 == 3)
  {
    if (WiFi.status() != WL_CONNECTED)
    {

      // Serial.print("\n Sin conexion, parpadeando...");
      int time = 50;
      for (int i = 0; i < 3; i++)
      {
        pixels.setBrightness(255);
        pixels.fill(pixels.Color(255, 255, 0), 0, 10);
        pixels.show();
        delay(time);
        pixels.clear();
        pixels.show();
        delay(time);
      }
    }
  }
  if (flag1 < 3)
  {
    flag1++;
  }
}

void Leer_EEPROM()
{

  EEPROM.begin(512);
  for (int i = 0; i < 50; i++)
  {
    ssid[i] = 0;
    pass[i] = 0;
  }
  EEPROM.get(dir_ssid, ssid);
  EEPROM.get(dir_pass, pass);
  // Comentado para usar con nextion
  /*
  Serial.print("\n--------------------------------");
  Serial.print("\n Datos leidos de la EEPROM:");
  Serial.print("\n SSID: " + String(ssid));
  Serial.print("\n PASS: " + String(pass));
  */
}

void Configurar_servidor()
{
  Servidor.on("/", Pagina_raiz);
  Servidor.on("/wifi", Pagina_wifi);
  Servidor.on("/escanear", Escanear_redes);
  Servidor.on("/conectar", Conectar_nueva_red);
  Servidor.begin();
  // Comentado para usar con nextion
  /*
  Serial.println("\n Servidor iniciado");
  */
}

/* Prueba para pasar de char a String */
void Funcion_prueba()
{
  int TAM_CADENA = 50;
  char cadena[TAM_CADENA];

  int TAM_STRING = 0;
  String string_origen = "Hola probando strings! 1233";
  String string_destino = "";

  //---> Ponemos toda la cadena de char en 0
  for (int i = 0; i < TAM_CADENA; i++)
  {
    cadena[i] = 0;
  }

  //---> Mostramos los valores (antes de hacer el pasaje):
  // Comentado para usar con nextion
  /*
  Serial.print("\n String inicio: " + string_origen);
  Serial.print("\n String vacio: " + string_destino);
  Serial.print("\n Cadena: " + String(cadena));
  */

  TAM_STRING = string_origen.length(); // Largo del string

  for (int i = 0; i < TAM_STRING; i++)
  {
    cadena[i] = string_origen[i]; // Copiamos el string a la cadena
  }

  for (int i = 0; i < TAM_CADENA; i++)
  {
    if (cadena[i] != 0)
      string_destino += cadena[i]; // Vamos concatenando en el String cada posicion del char
  }

  //---> Mostramos los resultados
  // Comentado para usar con nextion
  /*
  Serial.print("\n ------------> Luego de convertir:");
  Serial.print("\n String inicio: " + string_origen);
  Serial.print("\n String vacio: " + string_destino);
  Serial.print("\n Cadena: " + String(cadena));
  */
}

void Conectar_nueva_red()
{
  String ssid_aux = "";
  String password_aux = "";
  // Comentado para usar con nextion
  /*
  Serial.print("\n Datos recibidos guardados!");
  */

  ssid_aux = String(Servidor.arg("ssid"));
  password_aux = String(Servidor.arg("pass"));

  for (int i = 0; i < TAM_SSID; i++)
  {
    ssid[i] = ssid_aux[i]; // Copiamos el string a la cadena
  }
  for (int i = 0; i < TAM_PASS; i++)
  {
    pass[i] = password_aux[i]; // Copiamos el string a la cadena
  }

  // Comentado para usar con nextion
  /*
  Serial.print("\n -----------------------------------");
  Serial.print("\n Argumentos recibidos del server");
  Serial.print("\n ssid: " + String(ssid));
  Serial.print("\n pass: " + String(pass));
  Serial.print("\n -----------------------------------");
*/

  EEPROM.put(dir_ssid, ssid);
  EEPROM.commit();
  EEPROM.put(dir_pass, pass);
  EEPROM.commit();

  mensaje_html = "\n Configuración guardada! Se intentará realizar la conexión.";
  Pagina_wifi();
  Conectar_wifi();
  mensaje_html = "";
}

void Pagina_raiz()
{
  // Comentado para usar con nextion
  // Serial.print("\n Enviando Pagina_raiz");
  Pagina_html = "<!DOCTYPE html>"
                "<html>"
                "<head>"
                "<title>Configuración WiFi</title>"
                "<meta charset='UTF-8'>"
                "</head>"
                "<body>"
                "<h1> - - - Configuración del totem - - - </h1> <br>"
                "<p> Estado: ";
  Pagina_html += Estado_red + "</p> <br>";
  if (Estado_red == "conectado")
  {
    Pagina_html += "<p>SSID: " + String(ssid) + "</p>";
    Pagina_html += "<p>IP asignada por la red: " + WiFi.localIP().toString() + "</p>";
  }
  else if (Estado_red == "desconectado")
  {
    Pagina_html += "<p> Falló la conexion a la red: " + String(ssid) + "</p>";
  }
  Pagina_html += "<br><br><br><br><a href='/wifi'><button class='boton'>Configurar WiFi</button></a><br><br>";

  Servidor.send(200, "text/html", Pagina_html + Pagina_html_fin);
}

void Escanear_redes()
{
  // Serial.print("\n Enviando Escanear_redes");
  int cant_redes = WiFi.scanNetworks(); // devuelve el número de redes encontradas
  // Serial.println("escaneo terminado");
  if (cant_redes == 0) // si no encuentra ninguna red
  {
    // Serial.println("no se encontraron redes");
    mensaje_html = "no se encontraron redes";
  }
  else
  {
    // Serial.print("\n " + String(cant_redes) + " redes encontradas!");
    mensaje_html = "";
    mensaje_html = "<p> Redes encontradas: </p><br>";
    for (int i = 0; i < cant_redes; ++i)
    {
      // agrega al STRING "mensaje" la información de las redes encontradas
      mensaje_html += "<p>" + String(i + 1) + ": " + WiFi.SSID(i) + " </p>\r\n";
      delay(20);
    }
    // Serial.println(mensaje_html);
    Pagina_wifi();
    mensaje_html = "";
  }
}

void Pagina_wifi()
{
  // Serial.print("\n Enviando Pagina_wifi");
  String boton_back = "<br><a href='/'>[Back]</a><br>";
  String pagina = "<!DOCTYPE html>"
                  "<html>"
                  "<head>"
                  "<title>Configuración WiFi</title>"
                  "<meta charset='UTF-8'>"
                  "</head>"
                  "<body>"
                  "<h1> - - - Ingresar red - - - </h1> <br>";
  /* OJO CAMBIE EL GET POR EL POST
  ANTES ESTABA ASI:
  pagina += "</form>"
             "<form action='conectar' method='get'>" */
  pagina += "</form>"
            "<form action='conectar' method='post'>"
            "SSID:"
            "<input class='input1' name='ssid' type='text'><br>"
            "PASSWORD:"
            "<input class='input1' name='pass' type='password'><br><br>"
            "<input class='boton' type='submit' value='CONECTAR'/><br><br>"
            "</form>"
            "<a href='/escanear'><button class='boton'>Escanear</button></a><br><br>";

  Servidor.send(200, "text/html", pagina + mensaje_html + boton_back + Pagina_html_fin);
}

/* Funcion Conectar wifi */
void Conectar_wifi()
{
  Leer_EEPROM(); // Leemos el SSID y PASS para conectar
  flag1 = 0;     // Para evitar que titile el amarillo!
  // Comentado para usar con nextion
  /*
  Serial.print("\n Conectando WIFI...");
  Serial.print("\n SSID: " + String(ssid));
  Serial.print("\n Password: " + String(pass));
  */
  if (flag == 0)
  {
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid, pass);
    flag = 1;
  }
  else if (flag == 1)
  {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
  }
  int tiempo_max = 5000;
  int time = 0;
  int time_ant = millis();
  while (WiFi.status() != WL_CONNECTED && (time - time_ant) < tiempo_max)
  {
    delay(1);
    time = millis();
    Pantalla_Conectando();
  }
  Limpiar_display();
  if (WiFi.status() != WL_CONNECTED)
  { // No se pudo realizar la conexion. Iniciamos AP
    Pantalla_error_conexion();
    delay(4000);
    // Serial.print("\n Fallo la conexion a la red: " + String(ssid));
    Estado_red = "desconectado";
    // Iniciamos el AP:
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Sensor_" + String(SensorID), passConf);
    //IPAddress myIP = WiFi.softAPIP();
    // Comentado para usar con nextion
    /*
      Serial.print("\n\n Conectarse a la red: Sensor_" + String(SensorID));
      Serial.print("\n Con la contraseña: " + String(passConf));
      Serial.print("\n IP del access point: ");
      Serial.println(myIP);
      */
    Pantalla_desconectado();
  }
  else
  { // Se realizo la conexion correctamente
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    // Comentado para usar con nextion
    /*
      Serial.print("\n Se conecto a la red: " + String(ssid) + " correctamente!");
      Serial.print("\n Con la IP: ");
      Serial.print(WiFi.localIP());
      */
    Estado_red = "conectado";
    Pantalla_conectado();
  }
}
/* Fin de funcion */

/* Funcion LECTURA BOTONES */
void Lectura_Botones()
{

  //---> LUZ SUAVE
  pixels.setBrightness(5);
  pixels.fill(pixels.Color(100, 100, 100), 0, 10);
  pixels.show();
  /*
    //---> APAGAR TODOS LOS LEDS
    pixels.clear(); // Apaga todos los leds
    pixels.show();
  */

  if (Funcion_Boton_Retardo(BOTON_1, 100) > 0)
  {
    //---> Encendemos luces
    // Comentado para usar con nextion
    /*
      Serial.print("\n -------------------------");
      Serial.print("\n Boton 1: BOMBEROS");
      Serial.print("\n ------------------------- \n\n");
      */
    pixels.setBrightness(255);
    pixels.fill(pixels.Color(255, 0, 0), 0, 10); // Nos sirve para encender los led desde eL 0 al 10, con el valor RGB colocado entre parentesis.
    pixels.show();

    Pantalla_Alerta(1);

    //---> Nextion
    Enviar_Nextion("page Bomberos");

    if (WiFi.status() != WL_CONNECTED)
      // Serial.print("\n No se enviaron datos por falta de conexión");
      delay(2);
    else
      Envio_Servidor("BOMBEROS");
    delay(4000);

    // Enviamos denuevo la pantalla de WiFi
    if (WiFi.status() == WL_CONNECTED)
      Pantalla_conectado();
    else
      Pantalla_desconectado();
  }

  if (Funcion_Boton_Retardo(BOTON_2, 100) > 0)
  {
    // Comentado para usar con nextion
    /*
      Serial.print("\n -------------------------");
      Serial.print("\n Boton 2: AMBULANCIA");
      Serial.print("\n ------------------------- \n\n");
      */
    pixels.setBrightness(255);
    pixels.fill(pixels.Color(0, 255, 0), 0, 10); // Nos sirve para encender los led desde el 0 al 10, con el valor RGB colocado entre parentesis.
    pixels.show();

    Pantalla_Alerta(2);

    //---> Nextion
    Enviar_Nextion("page Ambulancia");

    if (WiFi.status() != WL_CONNECTED)
      delay(2);
    // Serial.print("\n No se enviaron datos por falta de conexión");
    else
      Envio_Servidor("AMBULANCIA");
    delay(4000);

    // Enviamos denuevo la pantalla de WiFi
    if (WiFi.status() == WL_CONNECTED)
      Pantalla_conectado();
    else
      Pantalla_desconectado();
  }

  if (Funcion_Boton_Retardo(BOTON_3, 100) > 0)
  {
    // Comentado para usar con nextion
    /*
      Serial.print("\n -------------------------");
      Serial.print("\n Boton 3: POLICIA");
      Serial.print("\n ------------------------- \n\n");
      */
    pixels.setBrightness(255);
    pixels.fill(pixels.Color(0, 0, 255), 0, 10); // Nos sirve para encender los led desde el 0 al 10, con el valor RGB colocado entre parentesis.
    pixels.show();

    Pantalla_Alerta(3);

    //---> Nextion
    Enviar_Nextion("page Policia");

    if (WiFi.status() != WL_CONNECTED)
      delay(2);
    // Serial.print("\n No se enviaron datos por falta de conexión");
    else
      Envio_Servidor("POLICIA");
    delay(4000);

    // Enviamos denuevo la pantalla de WiFi
    if (WiFi.status() == WL_CONNECTED)
      Pantalla_conectado();
    else
      Pantalla_desconectado();
  }
}
/* Fin de funcion */

/* ----------> Funcion leer boton retardo <---------- */
/* Retorna: -1: no esta pulsado, 1: se pulso sin retardo, 2: con retardo */
int Funcion_Boton_Retardo(int pulsador, int retardo_ms)
{
  int boton_presionado = 0;
  int i = 0;
  int retorno = -1;
  if (digitalRead(pulsador) == LOW)
  {
    boton_presionado = 1;
    while (digitalRead(pulsador) == LOW && i < retardo_ms)
    {
      delay(1);
      i++;
    }
    // Serial.print("\nSe presiono el pulsador GPIO: " + String(pulsador) + " durante " + String(i) + " milisegundos.");
  }
  //------
  // Sin retardo
  if (boton_presionado == 1 && i < retardo_ms)
  {
    boton_presionado = 0;
    retorno = 1;
    while (digitalRead(pulsador) == LOW)
      delay(1);
  }

  //------
  // Con Retardo
  if (boton_presionado == 1 && i == retardo_ms)
  {
    boton_presionado = 0;
    retorno = 2;
    while (digitalRead(pulsador) == LOW)
      delay(1);
  }

  return retorno;
}
/* Fin de funcion */

/* Funcion Parpadeo */
void Parpadeo(int port)
{

  for (int i = 0; i < 7; i++)
  {
    digitalWrite(port, LOW);
    delay(40);
    digitalWrite(port, HIGH);
    delay(40);
  }
}
/* Fin de funcion parpadeo */

void Envio_Servidor(String dato)
{
  //-----> Envio al servidor
  WiFiClient cliente_1; // Podemos poner cualquier nombre
  HTTPClient http_1;    // Podemos poner cualquier nombre

  String getData, linkCompleto;

  getData = "?dato=" + dato;
  linkCompleto = link + getData;

  // Comentado para usar con nextion
  /*
  Serial.print("LINK:");
  Serial.println(linkCompleto);
*/
  http_1.begin(cliente_1, linkCompleto); /* Iniciamos */

  //int CodigoDeRespuesta = http_1.GET(); 
  /*Codigo de error que responde el
  sevidor; 200: exitosa 404: no encontrado 500: servidor no pudo responder*/

  String respuesta = http_1.getString(); /* Para obtener la propia respuesta del servidor, directamente*/

  // Comentado para usar con nextion
  /*
  Serial.print("\nCodigo de respuesta:");
  Serial.print(CodigoDeRespuesta);
  Serial.print("\n\n Respuesta:");
  Serial.println(respuesta);
  */
}
/* Fin funcion */

/* Funcion pantalla CONECTANDO... */
void Pantalla_Conectando()
{
  //----> Nextion:
  Nex_Totem_EstaConectando();

  const String SensorID = String(ESP.getChipId(), HEX);
  int time = 150;
  int x = 64, y = 45;
  int tam_text = 1; // 1o2
  int indice_y = 15;
  int indice_y2 = 0;

  String aux = "Conectando...";
  String aux2 = "- " + String(ssid) + " -";
  int tam = aux.length();
  int indice_x = 64 - (6 * (int(tam / 2)));
  tam = aux2.length();
  int indice_x2 = 64 - (6 * (int(tam / 2)));

  // Serial.print("\n i:" + String(i));

  display.clearDisplay();
  display.drawCircleHelper(x, y, 10, 1, SSD1306_WHITE);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(tam_text);
  display.setCursor(indice_x, indice_y);
  display.println(aux);
  display.setCursor(indice_x2, indice_y2);
  display.println(aux2);
  display.display();
  delay(time);

  display.clearDisplay();
  display.drawCircleHelper(x, y, 10, 2, SSD1306_WHITE);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(tam_text);
  display.setCursor(indice_x, indice_y);
  display.println(aux);
  display.setCursor(indice_x2, indice_y2);
  display.println(aux2);
  display.display();
  delay(time);

  display.clearDisplay();
  display.drawCircleHelper(x, y, 10, 4, SSD1306_WHITE);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(tam_text);
  display.setCursor(indice_x, indice_y);
  display.println(aux);
  display.setCursor(indice_x2, indice_y2);
  display.println(aux2);
  display.display();
  delay(time);

  display.clearDisplay();
  display.drawCircleHelper(x, y, 10, 8, SSD1306_WHITE);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(tam_text);
  display.setCursor(indice_x, indice_y);
  display.println(aux);
  display.setCursor(indice_x2, indice_y2);
  display.println(aux2);
  display.display();
  delay(time);
}
/* Fin de funcion PANTALLA CONECTANDO */

/* Funcion PANTALLA Conexion OK*/
void Pantalla_conectado()
{
  //---> Nextion:
  Nex_Totem_Conectado();

  display.clearDisplay();
  display.display();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("SensorID:");
  display.setCursor(55, 0);
  display.println(SensorID);

  display.setCursor(0, 30);
  display.print("IP:");
  display.println(WiFi.localIP().toString());

  display.setCursor(0, 15);
  display.println("SSID:");
  display.setCursor(35, 15);
  display.println(ssid);

  String aux = "Conectado";
  int tam = aux.length();
  int indice_x = 64 - (6 * (int(tam / 2)));
  display.setCursor(indice_x, 50);
  display.print(aux);

  display.display();
}
/* Fin de funcion PANTALLA Conectado OK */

/* Funcion PANTALLA Desconectada */
void Pantalla_desconectado()
{
  //---> Nextion:
  Nex_Totem_Desconectado();

  display.clearDisplay();
  display.display();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("Totem sin conexion!");

  display.setCursor(0, 20);
  display.print("Conectar con la red: ");
  display.setCursor(0, 30);
  display.println("Sensor_" + String(SensorID));

  /*
    display.setCursor(0, 20);
    display.println("SensorID:");
    display.setCursor(55, 20);
    display.println(SensorID);
  */
  display.setCursor(0, 50);
  display.print("IP:");
  display.println(WiFi.softAPIP().toString());

  /*
    String aux = "Desconectado";
    int tam = aux.length();
    int indice_x = 64 - (6 * (int(tam / 2)));
    display.setCursor(indice_x, 50);
    display.print(aux);
  */
  display.display();
}
/* Fin de funcion PANTALLA Desconectado */

/* Funcion limpiar display */
void Limpiar_display()
{
  display.clearDisplay();
  display.display();
}
/* Fin de funcion limpar display */

/* Pantalla alerta */
void Pantalla_Alerta(int alerta)
{
  display.clearDisplay();
  display.display();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  String aux = "ALERTA:";
  int tam = aux.length();
  int indice_x = 64 - (6 * (int(tam / 2)));
  display.setCursor(indice_x, 10);
  display.print(aux);

  if (alerta == 1)
  {
    aux = "BOMBEROS";
    int tam = aux.length();
    int indice_x = 64 - (6 * (int(tam / 2)));
    display.setCursor(indice_x, 40);
    display.print(aux);
  }
  else if (alerta == 2)
  {
    aux = "AMBULANCIA";
    int tam = aux.length();
    int indice_x = 64 - (6 * (int(tam / 2)));
    display.setCursor(indice_x, 40);
    display.print(aux);
  }
  else if (alerta == 3)
  {
    aux = "POLICIA";
    int tam = aux.length();
    int indice_x = 64 - (6 * (int(tam / 2)));
    display.setCursor(indice_x, 40);
    display.print(aux);
  }
  display.display();
}
/* fin de funcion */

/* Funcion error conexion pantalla */
void Pantalla_error_conexion()
{
  //----> Nextion:
  Nex_Totem_ErrorConexion();

  display.clearDisplay();
  display.display();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  String aux = "Error en la red:";
  int tam = aux.length();
  int indice_x = 64 - (6 * (int(tam / 2)));
  display.setCursor(indice_x, 20);
  display.print(aux);

  aux = ssid;
  tam = aux.length();
  indice_x = 64 - (6 * (int(tam / 2)));
  display.setCursor(indice_x, 40);
  display.print(aux);

  // Serial.print("\n Status wifi: " + String(WiFi.status()));

  display.display();
}
/* fin de funcion */

/* Funcion ENVIAR A NEXTION */
void Enviar_Nextion(String str)
{
  if (str.compareTo("0") != 0)
  {
    // Serial.print("\n Enviando comando: " + str);
    // Nextion.print(str);
    Serial.print(str);
  }
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  /*
  Nextion.write(0xff);
  Nextion.write(0xff);
  Nextion.write(0xff);
*/
}
/* fin de funcion */

/* Funcion totem conectado */
void Nex_Totem_Conectado()
{
  String aux = "";
  Enviar_Nextion("0");
  Enviar_Nextion("page Conectado");
  //----
  aux = "Texto2.txt=\"" + String(SensorID) + "\"";
  Enviar_Nextion(aux);
  //----
  aux = "Texto4.txt=\"";
  for (int i = 0; i < 50; i++)
  {
    if (ssid[i] != '0')
      aux += ssid[i];
  }
  aux += "\"";
  Enviar_Nextion(aux);
  //----
  aux = "Texto6.txt=\"" + WiFi.localIP().toString() + "\"";
  Enviar_Nextion(aux);
}
/* fin de la funcion */

/* Funcion totem DESconectado */
void Nex_Totem_Desconectado()
{
  String aux = "";
  Enviar_Nextion("0");
  Enviar_Nextion("page Desconectado");
  //----
  aux = "Texto2.txt=\"Sensor_" + String(SensorID) + "\"";
  Enviar_Nextion(aux);
  //----
  aux = "Texto4.txt=\"" + WiFi.softAPIP().toString() + "\"";
  Enviar_Nextion(aux);
}
/* fin de la funcion */


/* Funcion totem SE ESTA conectando */
void Nex_Totem_EstaConectando()
{
  String aux = "";
  Enviar_Nextion("0");
  Enviar_Nextion("page Conectando");
  //----
  aux = "Texto2.txt=\"" + String(ssid) + "\"";
  Enviar_Nextion(aux);
  //----
}
/* fin de la funcion */

/* Funcion Error Conexion */
void Nex_Totem_ErrorConexion(){
  String aux = "";
  Enviar_Nextion("0");
  Enviar_Nextion("page ErrorConexion");
  //----
  aux = "Texto2.txt=\"" + String(ssid) + "\"";
  Enviar_Nextion(aux);
  //----
}