#include <Arduino.h>
#include "WebServer.h"
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <LittleFS.h>
#define SPIFFS LittleFS

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <SoftwareSerial.h>

// AGREGADO OLED Y NEOPIXEL
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Ticker.h>
#include <RTClib.h>

#include "DFRobot_MultiGasSensor.h"

String firmVer = "2.0";

String webpage = "";
String datos_set;
String status_string = "";

const char *host_test_connection = "www.google.com";

Ticker timer_1ms;
RTC_DS3231 rtc;
bool RTC_OK;

//--------------------> AGREGADO PARA PANTALLA NEXTION

String Estado_red = "";
#define NEXTION_TX D4 // D4 CON RX DEL SIM808
#define NEXTION_RX D3 // D6 CON TX DEL SIM808
#include <SoftwareSerial.h>
SoftwareSerial Nextion(NEXTION_RX, NEXTION_TX); // RX, TX
String Nex_pag_actual = "";
int Preguntar_Pagina();
String Devolver_Pagina(String);
String Leer_Nextion_Serie();
int Nex_Leer_Pass_Ssid(String);
int LeerPagina();
void Nex_Pantalla_Inicio();
void Nex_Pantalla_Conectando();
void Leer_Sensado(String);
void Leer_Nextion();
void Nex_Pantalla_Inicio_Mediciones();
void Enviar_Nextion(String);
void Vaciar_Serie();
int Nex_Mostrar_Pag(int);

int ID_vacio = 0;
int ID_inicio = 1;
int ID_configuracion = 2;
int ID_escaneo = 3;
int ID_password = 4;
int ID_conectando = 5;
int ID_iniciando = 6;
int ID_conftiempos = 7;

int Sensado_tiempo = 0;
int flag_rojo_negro = 0;
int flag_conf = 0;
int flag_inicio = 0;
int flag_escanear = 0;
int flag_wifi = 0;
int flag_password = 0;
int flag_nextion = 0;

//--------------------> FIN DE PANTALLA NEXTION

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET LED_BUILTIN // 4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C    ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int flg_guardar_dato = 0;
int Status = WL_IDLE_STATUS;
int Status_ant = 99;

int cont_display = 0;

// FIN AGREGADO OLED Y NEOPIXEL

#define RX_PIN 13     // D7 - Rx pin which the AEM1000 Tx pin is attached to
#define TX_PIN 15     // D8 - Tx pin which the AEM1000 Rx pin is attached to
#define BAUDRATE 9600 // Device to AEM1000 Serial baudrate (should not be changed)

// #define LED_D6 12

const String SensorID = String(ESP.getChipId(), HEX);

HTTPClient http_post;
WiFiClient client_post;
HTTPClient http_post2;
WiFiClient client_post2;
String respuestaPost = "";
String respuestaPostLocal = "";
String IP_Puerto_local = "";

const int RSSI_MAX = -50;  // define maximum strength of signal in dBm
const int RSSI_MIN = -100; // define minimum strength of signal in dBm

////////////////////// Comunicacion Modulo AEM1000 //////////////////////////////////////////
SoftwareSerial AEM1000_COM(RX_PIN, TX_PIN); // (Uno example) create device to MH-Z19 serial
#define DIR_485 14
byte reg_temp[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};
byte reg_humedad[] = {0x01, 0x03, 0x00, 0x01, 0x00, 0x01, 0xD5, 0xCA};
byte reg_TVOC[] = {0x01, 0x03, 0x00, 0x02, 0x00, 0x01, 0x25, 0xCA};
byte reg_CO2[] = {0x01, 0x03, 0x00, 0x03, 0x00, 0x01, 0x74, 0x0A};
byte reg_PM2_5[] = {0x01, 0x03, 0x00, 0x04, 0x00, 0x01, 0xC5, 0xCB};
byte nro_sensor = 0;
float valor_temp = 0;
float valor_humedad = 0;
float valor_TVOC = 0;
int valor_CO2 = 0;
int valor_PM2_5 = 0;
////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////// Comunicacion Sensores Gas DFROBOT //////////////////////////////
#define NH3_GAS_ADDRESS 0x74
#define H2S_GAS_ADDRESS 0x75
#define SO2_GAS_ADDRESS 0x76

DFRobot_GAS_I2C NH3_gas(&Wire, NH3_GAS_ADDRESS);
float valor_NH3;
DFRobot_GAS_I2C H2S_gas(&Wire, H2S_GAS_ADDRESS);
float valor_H2S;
DFRobot_GAS_I2C SO2_gas(&Wire, SO2_GAS_ADDRESS);
float valor_SO2;

////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////// Comunicacion GPRS SIM800L /////////////////////////////////////
#define SIM808_TX D5                         // D5 CON RX DEL SIM808
#define SIM808_RX D6                         // D6 CON TX DEL SIM808
SoftwareSerial SIM808(SIM808_RX, SIM808_TX); // RX, TX

String apn_SIM = "wap.gprs.unifon.com.ar"; // APN: "wap.gprs.unifon.com.ar" FUNCIONA BIEN PARA MOVISTAR!
String apn_user_SIM = "wap";               // APN-Username
String apn_pass_SIM = "wap";               // APN-Password
////////////////////////////////////////////////////////////////////////////////////////////

DateTime fechaUltimoDato;

ADC_MODE(ADC_VCC); // este modo sirve para habilitar el divisor interno y
// y poder medir correctamente el BUS de 3.3v

long connect_tic = 120000; // interval at which to blink (milliseconds)
long OLED_tic = 0;
long AEM1000_tic = 0;
long fin_AP_tic = 0;
long DFRobot_gas_tic = 0;

#define MAX_BYTES 1500000

void Timer_1ms()
{
  if (connect_tic > 0)
    connect_tic--;
  if (OLED_tic > 0)
    OLED_tic--;
  if (AEM1000_tic > 0)
    AEM1000_tic--;
  if (fin_AP_tic > 0)
    fin_AP_tic--;
  if (DFRobot_gas_tic > 0)
    DFRobot_gas_tic--;
}

//-------------------VARIABLES GLOBALES--------------------------
int contconexion = 0;

char ssid[50];
char pass[50];

int flag_guardar = 0;
int flag_fin_AP = 0;

const char *passConf = "12345678";

String mensaje = "";

//-----------CODIGO HTML PAGINA DE CONFIGURACION---------------
String pagina = "";
String paginafin = "</body>"
                   "</html>";
//-------------------------------------------------------------

//--------------------------- DECLARACION DE FUNCIONES----------------------
void connect();
void gprs_connect();
String Enviar_Verificar(String, String, String);
String leer(int);
void Index();
void guardar_conf();
void escanear();
void UltimoDatoEnviado();
void DatoEnviado();
void grabar(int, String);
void File_Download();
void Config_Date();
void Config_Sensed();
byte leer_AEM1000(byte *, byte *);
void testdrawstyles(void);
void borrar_archivos_viejos();
int dBmtoPercentage(long);
void SendHTML_Header();
void SendHTML_Content();
void SendHTML_Stop();
void SD_file_download(String);
void SD_file_borrar(String);
void SelectInput(String, String, String, String);
void Set_Sensado(const String);
void SetDatosEnvio(const String);
void Set_Date_Time(String);
void ReportFileNotPresent(String);
void append_page_header();

//------------------------SETUP WIFI-----------------------------
void setup_wifi()
{
  Nex_Mostrar_Pag(ID_iniciando);
  // Conexión WIFI
  leer(0).toCharArray(ssid, 50);
  leer(50).toCharArray(pass, 50);
  WiFi.mode(WIFI_STA); // para que no inicie el SoftAP en el modo normal
  //  WiFi.mode(WIFI_AP_STA); /*1*/ //ESP8266 works in both AP mode and station mode
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED and contconexion < 50)
  { // Cuenta hasta 50 si no se puede conectar lo cancela
    ++contconexion;
    delay(100);
    Serial.print(".");
    digitalWrite(2, HIGH);
    delay(100);
    digitalWrite(2, LOW);
  }
  if (contconexion < 50)
  {
    Serial.println("");
    Serial.println("WiFi conectado");
    Serial.print("IP que asignada en la red: ");
    Serial.println(WiFi.localIP());
    digitalWrite(2, HIGH);
    contconexion = 0;
    Estado_red = "conectado";
  }
  else
  {
    Serial.println("");
    Serial.println("Error de conexion");
    digitalWrite(2, LOW);
    contconexion = 0;
    Estado_red = "desconectado";
  }
  //----> Agregado para Nextion
  Nex_pag_actual = "Inicio";
}

//-------------------PAGINA DE CONFIGURACION--------------------
void Wificonf()
{
  pagina = "<!DOCTYPE html>"
           "<html>"
           "<head>"
           "<title>Configuración WiFi</title>"
           "<meta charset='UTF-8'>"
           "</head>"
           "<body>";
  pagina += "<p>SSID: " + String(ssid) + "</p>";
  pagina += "<p>IP asignada por la red: " + WiFi.localIP().toString() + "</p>";
  pagina += "</form>"
            "<form action='guardar_conf' method='get'>"
            "SSID:<br><br>"
            "<input class='input1' name='ssid' type='text'><br>"
            "PASSWORD:<br><br>"
            "<input class='input1' name='pass' type='password'><br><br>"
            "<input class='boton' type='submit' value='GUARDAR'/><br><br>"
            "</form>"
            "<a href='escanear'><button class='boton'>ESCANEAR</button></a><br>"
            "<br><a href='/'>[Back]</a><br>";

  WebServer.send(200, "text/html", pagina + mensaje + paginafin);
}

//--------------------MODO_CONFIGURACION------------------------
void modoconf()
{

  delay(100);
  digitalWrite(2, HIGH);
  delay(100);
  digitalWrite(2, LOW);
  delay(100);
  digitalWrite(2, HIGH);
  delay(100);
  digitalWrite(2, LOW);

  WiFi.softAP(("Sensor_" + SensorID), passConf);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("IP del access point: ");
  Serial.println(myIP);
  Serial.println("WebServer iniciado...");

  WebServer.on("/", Index); // Index

  WebServer.on("/wifi", Wificonf); // Configuración WiFi

  WebServer.on("/guardar_conf", guardar_conf); // Graba en la eeprom la configuracion

  WebServer.on("/escanear", escanear); // Escanean las redes wifi disponibles

  WebServer.on("/ultimodato", UltimoDatoEnviado); // Podemos ver el ultimo dato enviado

  WebServer.on("/envio", DatoEnviado); // Enviar un dato

  WebServer.begin();
}

//---------------------GUARDAR CONFIGURACION-------------------------
void guardar_conf()
{
  Serial.println(WebServer.arg("ssid")); // Recibimos los valores que envia por GET el formulario web
  grabar(0, WebServer.arg("ssid"));
  Serial.println(WebServer.arg("pass"));
  grabar(50, WebServer.arg("pass"));

  mensaje = "Configuracion Guardada...";
  Wificonf();
  mensaje = "";
  flag_guardar = 1;
}

//----------------Función para grabar en la EEPROM-------------------
void grabar(int addr, String a)
{
  int tamano = a.length();
  char inchar[50];
  a.toCharArray(inchar, tamano + 1);
  for (int i = 0; i < tamano; i++)
  {
    EEPROM.write(addr + i, inchar[i]);
  }
  for (int i = tamano; i < 50; i++)
  {
    EEPROM.write(addr + i, 255);
  }
  EEPROM.commit();
}

//-----------------Función para leer la EEPROM------------------------
String leer(int addr)
{
  byte lectura;
  String strlectura;
  for (int i = addr; i < addr + 50; i++)
  {
    lectura = EEPROM.read(i);
    if (lectura != 255)
    {
      strlectura += (char)lectura;
    }
  }
  return strlectura;
}
//---------------------------ESCANEAR----------------------------
void escanear()
{
  int n = WiFi.scanNetworks(); // devuelve el número de redes encontradas
  Serial.println("escaneo terminado");
  if (n == 0)
  { // si no encuentra ninguna red
    Serial.println("no se encontraron redes");
    mensaje = "no se encontraron redes";
  }
  else
  {
    Serial.print(n);
    Serial.println(" redes encontradas");
    mensaje = "";
    for (int i = 0; i < n; ++i)
    {
      // agrega al STRING "mensaje" la información de las redes encontradas
      mensaje = (mensaje) + "<p>" + String(i + 1) + ": " + WiFi.SSID(i) + " </p>\r\n";
      //      mensaje = (mensaje) + "<p>" + String(i + 1) + ": " + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ") Ch: " + WiFi.channel(i) + " Enc: " + WiFi.encryptionType(i) + " </p>\r\n";
      // WiFi.encryptionType 5:WEP 2:WPA/PSK 4:WPA2/PSK 7:open network 8:WPA/WPA2/PSK
      delay(20);
    }
    Serial.println(mensaje);
    Wificonf();
    mensaje = "";
  }
}
//--------------------------------------------------------------------------

// --- Redondear una float o una double
float redondear(float valor, int decimales)
{
  double _potencia = pow(10, decimales);
  return (roundf(valor * _potencia) / _potencia);
};
//--------------------------------------------------------------------------

void setup()
{

  Serial.begin(115200);

  //-----------> Pantalla NEXTION:
  Nextion.begin(115200);
  Nex_Mostrar_Pag(ID_vacio);

  // pinMode(LED_D6, OUTPUT);
  // pinMode(2, OUTPUT); // D7
  pinMode(DIR_485, OUTPUT);
  digitalWrite(DIR_485, HIGH);

  EEPROM.begin(512);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.display();

  display.setCursor(0, 55); // Start at top-left corner

  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.println(firmVer);

  display.setCursor(15, 0); // Start at top-left corner

  display.setTextSize(4); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.println(F("ADOX"));

  display.setCursor(0, 30); // Start at top-left corner

  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.println(F("MonitorAmb"));
  display.display();

  WebServer.init();

  setup_wifi();

  modoconf();

  WebServer.on("/borrar_memoria", HTTP_GET, [&]()
               {
    if (SPIFFS.format()) WebServer.send(200, "text/plain", "Memoria borrada OK");
    else WebServer.send(200, "text/plain", "Error al borrar Memoria"); });

  WebServer.on("/download", File_Download);
  WebServer.on("/date", Config_Date);
  WebServer.on("/sensed", Config_Sensed);

  datos_set = leer(100);
  IP_Puerto_local = leer(150);

  delay(2000);

  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text

  display.setCursor(0, 0); // Start at top-left corner
  display.println(F("SensorID:"));

  display.setCursor(0, 16); // Start at top-left corner
  display.println(SensorID);

  display.display();
  delay(2000);

  display.setTextSize(1); // Draw 2X-scale text

  display.setCursor(0, 32); // Start at top-left corner
  display.print(F("IP: "));
  display.println(WiFi.localIP().toString());

  display.setCursor(0, 40); // Start at top-left corner
  display.println(F("SSID: "));

  display.setCursor(0, 50); // Start at top-left corner
  display.println(WiFi.SSID());

  display.display();
  delay(3000);

  AEM1000_COM.begin(BAUDRATE); // (Uno example) device to MH-Z19 serial start

  ////////////int a = connect();
  // connect();

  timer_1ms.attach_ms(1, Timer_1ms);

  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    RTC_OK = 0;
  }
  else
    RTC_OK = 1;

  if (!SPIFFS.begin())
  {
    Serial.println("[Storage] Couldn't mount file system.");
  }

  fin_AP_tic = 900000; // 15 minutos con AP

  // Inicializacion de Sensores de gas DFROBOT
  if (!NH3_gas.begin())
  {
    Serial.println("NH3 gas sensor no connected");
  }
  else
    Serial.println("NH3 gas sensor connected");

  NH3_gas.changeAcquireMode(NH3_gas.PASSIVITY);
  NH3_gas.setTempCompensation(NH3_gas.OFF);

  if (!H2S_gas.begin())
  {
    Serial.println("H2S gas sensor no connected");
  }
  else
    Serial.println("H2S gas sensor connected");

  H2S_gas.changeAcquireMode(H2S_gas.PASSIVITY);
  H2S_gas.setTempCompensation(H2S_gas.OFF);

  if (!SO2_gas.begin())
  {
    Serial.println("SO2 gas sensor no connected");
  }
  else
    Serial.println("SO2 gas sensor connected");

  SO2_gas.changeAcquireMode(SO2_gas.PASSIVITY);
  SO2_gas.setTempCompensation(SO2_gas.OFF);

  /////////////////////////////// Inicializar comunicacion con SIM800L ////////////////////////////
  SIM808.begin(19200);
  /////////////////////////////////////////////////////////////////////////////////////////////////

  
}

void loop()
{

  WebServer.handleClient();

  if (!AEM1000_tic)
  {
    AEM1000_tic = 500;
    byte data[10];
    byte nro_bytes = 0;
    int valor_aux = 0;

    if (nro_sensor == 0)
    {
      nro_bytes = leer_AEM1000(reg_temp, data);
      if (nro_bytes == 7)
      {
        valor_aux = (data[3] << 8) + data[4];
        valor_temp = (float)valor_aux / 10;
        valor_temp = redondear(valor_temp, 1);
      }
      nro_sensor = 1;
    }
    else if (nro_sensor == 1)
    {
      nro_bytes = leer_AEM1000(reg_humedad, data);
      if (nro_bytes == 7)
      {
        valor_aux = (data[3] << 8) + data[4];
        valor_humedad = (float)valor_aux / 10;
        valor_humedad = redondear(valor_humedad, 1);
      }
      nro_sensor = 2;
    }
    else if (nro_sensor == 2)
    {
      nro_bytes = leer_AEM1000(reg_TVOC, data);
      if (nro_bytes == 7)
      {
        valor_aux = (data[3] << 8) + data[4];
        valor_TVOC = (float)valor_aux / 1000;
        valor_TVOC = redondear(valor_TVOC, 3);
      }
      nro_sensor = 3;
    }
    else if (nro_sensor == 3)
    {
      nro_bytes = leer_AEM1000(reg_CO2, data);
      if (nro_bytes == 7)
      {
        valor_aux = (data[3] << 8) + data[4];
        valor_CO2 = valor_aux;
      }
      nro_sensor = 4;
    }
    else if (nro_sensor == 4)
    {
      nro_bytes = leer_AEM1000(reg_PM2_5, data);
      if (nro_bytes == 7)
      {
        valor_aux = (data[3] << 8) + data[4];
        valor_PM2_5 = valor_aux;
      }
      nro_sensor = 0;
    }
    //    Serial.write(data, nro_bytes);

    //    long CO2_aux = myMHZ19.getCO2();  // Request CO2 (as ppm
  }

  if (!DFRobot_gas_tic)
  {
    DFRobot_gas_tic = 2000;
    valor_NH3 = NH3_gas.readGasConcentrationPPM();
    valor_H2S = H2S_gas.readGasConcentrationPPM();
    valor_SO2 = SO2_gas.readGasConcentrationPPM();
    //    valor_NH3 = NH3_gas.getSensorVoltage();
  }

  if (!OLED_tic)
  {
    OLED_tic = 2000;
    testdrawstyles();
    //-----> Agregado para Nextion
    Nex_Pantalla_Inicio_Mediciones();
  }

  if (!connect_tic)
  {
    Serial.println("+");
    Serial.println(datos_set);
    Serial.println("+");
    if (datos_set == "5_m")
      connect_tic = 300000;
    else if (datos_set == "10_m")
      connect_tic = 600000;
    else if (datos_set == "15_m")
      connect_tic = 900000;
    else if (datos_set == "20_m")
      connect_tic = 1200000;
    else if (datos_set == "30_m")
      connect_tic = 1800000;
    else if (datos_set == "45_m")
      connect_tic = 2700000;
    else if (datos_set == "1_h")
      connect_tic = 3600000;
    else if (datos_set == "2_h")
      connect_tic = 7200000;
    else if (datos_set == "5_h")
      connect_tic = 18000000;
    else if (datos_set == "12_h")
      connect_tic = 43200000;
    else
    {
      datos_set = "5_m";
      connect_tic = 300000;
    }

    if (WiFi.status() == WL_CONNECTED && client_post.connect(host_test_connection, 80))
    {
      client_post.stop();
      connect();
    }
    else
    {
      Serial.println("NO INTERNET CONNECTION");
      client_post.stop();
      gprs_connect();
    }

    flg_guardar_dato = 1;
  }

  if (flag_guardar)
  {
    leer(0).toCharArray(ssid, 50);
    leer(50).toCharArray(pass, 50);
    WiFi.mode(WIFI_AP_STA); // para que no inicie el SoftAP en el modo normal
    WiFi.begin(ssid, pass);
    Serial.println("------------------------------------------------");
    Wificonf();
    flag_guardar = 0;
    fin_AP_tic = 300000; // 5 minutos con AP
  }

  if (!fin_AP_tic && !flag_fin_AP)
  {
    if (Status == WL_CONNECTED)
    {
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, pass);
      flag_fin_AP = 1;
    }
    else
      fin_AP_tic = 300000;
  }

  Status = WiFi.status();

  if (Status != Status_ant)
  {
    Status_ant = Status;
    Serial.println("****************************");
    if (Status == WL_IDLE_STATUS)
      status_string = "Estado IDLE"; // 0
    else if (Status == WL_NO_SSID_AVAIL)
      status_string = "No encuentra el SSID"; // 1
    else if (Status == WL_SCAN_COMPLETED)
      status_string = "Escaneo completado"; // 2
    else if (Status == WL_CONNECTED)
      status_string = "Conectado"; // 3
    else if (Status == WL_CONNECT_FAILED)
      status_string = "Error al conectar"; // 4
    else if (Status == WL_CONNECTION_LOST)
      status_string = "Conexion perdida"; // 5
    else if (Status == WL_DISCONNECTED)
      status_string = "Desconectado"; // 6
    else
      status_string = String(Status);
    Serial.println(status_string);
    Serial.println(ssid);
    Serial.println(pass);
  }

  if (flg_guardar_dato && RTC_OK == 1)
  {

    borrar_archivos_viejos();

    FSInfo fsInfo;
    SPIFFS.info(fsInfo);
    Serial.print("FS Total Bytes: ");
    Serial.println(fsInfo.totalBytes);
    Serial.print("FS Used Bytes: ");
    Serial.println(fsInfo.usedBytes);

    DateTime now = rtc.now();
    Serial.println(now.timestamp(DateTime::TIMESTAMP_FULL));

    if (fsInfo.usedBytes < MAX_BYTES)
    {
      File file = SPIFFS.open("/Log_" + now.timestamp(DateTime::TIMESTAMP_DATE) + "_" + SensorID + ".csv", "a");

      if (file)
      {
        file.print(now.timestamp(DateTime::TIMESTAMP_TIME));
        file.print(';');
        file.print(valor_temp, 1);
        file.print(';');
        file.print(valor_humedad, 1);
        file.print(';');
        file.print(valor_TVOC, 3);
        file.print(';');
        file.print(valor_CO2);
        file.print(';');
        file.print(valor_PM2_5);
        file.print(';');
        file.print(valor_NH3);
        file.print(';');
        file.print(valor_SO2);
        file.print(';');
        file.println(valor_H2S);

        Serial.println(file.size());
        file.close();
      }
      else
      {
        Serial.println("Fallo al abrir archivo");
      }
    }
    else
      Serial.println("Memoria llena");

    flg_guardar_dato = 0;
  }

  Leer_Nextion();

} // cierre del loop

void connect()
{

  Serial.print("SensorID:\n");
  Serial.print(SensorID + "\n");

  fechaUltimoDato = rtc.now();

  long rssi = WiFi.RSSI();
  Serial.print("RSSI:");
  Serial.println(rssi);

  int quality = dBmtoPercentage(rssi);
  Serial.println(quality);

  /*****************************************Envio al sistema nuevo por metodo POST****************************************/
  respuestaPost = "";
  String url_post = "http://data.ambientecontrolado.com.ar/device/measure";
  String data_post = "{\"data\":{\"extraData\":{\"chipId\":\"" + SensorID + "\",\"temperature\":" + String(valor_temp, 1) + ",\"humidity\":" + String(valor_humedad, 1) + ",\"tvoc\":" + String(valor_TVOC, 3) + ",\"co2\":" + String(valor_CO2) + ",\"pm2_5\":" + String(valor_PM2_5) + ",\"nh3\":" + String(valor_NH3) + ",\"so2\":" + String(valor_SO2) + ",\"h2s\":" + String(valor_H2S) + ",\"ssid\":\"" + WiFi.SSID() + "\",\"ip\":\"" + WiFi.localIP().toString() + "\",\"signal\":" + String(quality) + ",\"firmwareVersion\":\"" + firmVer + "\"}}}";

  Serial.println("URL: " + url_post);
  Serial.println("DATA: " + data_post);

  Serial.print("[HTTP] begin...\n");
  // configure traged server and url
  http_post.begin(client_post, url_post); // HTTP
  http_post.addHeader("Content-Type", "application/json");

  Serial.print("[HTTP] POST...\n");
  // start connection and send HTTP header and body
  int httpCode_post = http_post.POST(data_post);

  // httpCode will be negative on error
  if (httpCode_post > 0)
  {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] POST... code: %d\n", httpCode_post);

    // file found at server
    if (httpCode_post == HTTP_CODE_OK)
    {
      const String &payload = http_post.getString();
      Serial.println("received payload:\n<<");
      Serial.println(payload);
      Serial.println(">>");
    }
    respuestaPost = String(httpCode_post) + "<br>" + http_post.getString();
  }
  else
  {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http_post.errorToString(httpCode_post).c_str());
  }

  http_post.end();

  /*****************************************Envio al local por metodo POST ****************************************/

  respuestaPostLocal = "";

  if (IP_Puerto_local != "")
  {
    url_post = "http://" + IP_Puerto_local + "/device/measure";

    Serial.println("URL: " + url_post);
    Serial.println("DATA: " + data_post);

    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    http_post2.begin(client_post2, url_post); // HTTP
    http_post2.addHeader("Content-Type", "application/json");

    Serial.print("[HTTP] POST...\n");
    // start connection and send HTTP header and body
    httpCode_post = http_post2.POST(data_post);

    // httpCode will be negative on error
    if (httpCode_post > 0)
    {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode_post);

      // file found at server
      if (httpCode_post == HTTP_CODE_OK)
      {
        const String &payload = http_post2.getString();
        Serial.println("received payload:\n<<");
        Serial.println(payload);
        Serial.println(">>");
      }
      //    respuestaPostLocal = "";//****************************************************
      respuestaPostLocal = String(httpCode_post) + "<br>" + http_post2.getString();
    }
    else
    {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http_post2.errorToString(httpCode_post).c_str());
    }

    http_post2.end();
  }
}

void gprs_connect()
{
  String ImprimirSerie = "MOSTRAR";
  /*Si ponemos  String ImprimirSerie = "MOSTRAR"; va a ir mostrando en el serie cada comando que envia */

  String respuesta = "";
  String retorno_envio = "ERROR";

  Serial.print("SensorID:\n");
  Serial.print(SensorID + "\n");

  fechaUltimoDato = rtc.now();

  long rssi = 0;
  respuesta = Enviar_Verificar("AT+CSQ", "", "MOSTRAR_TODO");

  Serial.print("RSSI:");
  // Serial.println(rssi);
  Serial.println(respuesta);

  int quality = dBmtoPercentage(rssi);
  Serial.println(quality);

  /**************************** Envio al sistema por metodo POST usando gprs ************************************/
  respuestaPost = "";
  String url_post = "http://data.ambientecontrolado.com.ar/device/measure";
  String data_post = "{\"data\":{\"extraData\":{\"chipId\":\"" + SensorID + "\",\"temperature\":" + String(valor_temp, 1) + ",\"humidity\":" + String(valor_humedad, 1) + ",\"tvoc\":" + String(valor_TVOC, 3) + ",\"co2\":" + String(valor_CO2) + ",\"pm2_5\":" + String(valor_PM2_5) + ",\"nh3\":" + String(valor_NH3) + ",\"so2\":" + String(valor_SO2) + ",\"h2s\":" + String(valor_H2S) + ",\"ssid\":\"" + apn_SIM + "\",\"ip\":\"" + WiFi.localIP().toString() + "\",\"signal\":" + String(quality) + ",\"firmwareVersion\":\"" + firmVer + "\"}}}";

  Serial.println("URL: " + url_post);
  Serial.println("DATA: " + data_post);

  Serial.print("[HTTP] begin...\n");

  // Vacio puerto serie
  while (SIM808.available() > 0)
  {
    SIM808.read();
    delay(1);
  }

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
                    respuesta = Enviar_Verificar("AT+HTTPPARA=URL," + url_post, "OK", ImprimirSerie);
                    if (respuesta.compareTo("OK") == 0)
                    {
                      respuesta = Enviar_Verificar("AT+HTTPPARA=\"CONTENT\",\"application/json\"", "OK", ImprimirSerie);
                      if (respuesta.compareTo("OK") == 0)
                      {
                        respuesta = Enviar_Verificar("AT+HTTPDATA=" + (String)data_post.length() + ",10000", "DOWNLOAD", ImprimirSerie);
                        if (respuesta.compareTo("OK") == 0)
                        {
                          respuesta = Enviar_Verificar(data_post, "OK", ImprimirSerie);
                          if (respuesta.compareTo("OK") == 0)
                          {
                            respuesta = Enviar_Verificar("AT+HTTPACTION=1", "OK", ImprimirSerie);
                            if (respuesta.compareTo("OK") == 0)
                            {
                              respuesta = Enviar_Verificar("", "", "MOSTRAR_TODO");

                              //**** Aqui es cuando el envio fue correcto!
                              retorno_envio = "ENVIADO";
                              respuesta = Enviar_Verificar("AT+HTTPREAD", "", "MOSTRAR_TODO");

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
  else
  {
  }
}

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
  int tiempo_espera = 2000;
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
  if (texto_enviar != "")
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
      retorno = stringLeido;
    }
    else
    {
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

/*
   Written by Ahmad Shamshiri
    with lots of research, this sources was used:
   https://support.randomsolutions.nl/827069-Best-dBm-Values-for-Wifi
   This is approximate percentage calculation of RSSI
   Wifi Signal Strength Calculation
   Written Aug 08, 2019 at 21:45 in Ajax, Ontario, Canada
*/

int dBmtoPercentage(long dBm)
{
  int quality;
  if (dBm <= RSSI_MIN)
  {
    quality = 0;
  }
  else if (dBm >= RSSI_MAX)
  {
    quality = 100;
  }
  else
  {
    quality = 2 * (dBm + 100);
  }

  return quality;
} // dBmtoPercentage

void testdrawstyles(void)
{
  display.clearDisplay();

  display.setCursor(0, 0);             // Start at top-left corner
  display.setTextColor(SSD1306_WHITE); // Draw white text

  if (cont_display == 0)
  {
    display.setTextSize(2); // Draw 2X-scale text
    display.println(F("Temp."));
    display.setTextSize(3); // Draw 2X-scale text
    display.println(valor_temp, 1);
    display.println(F("C"));
    cont_display = 1;
  }
  else if (cont_display == 1)
  {
    display.setTextSize(2); // Draw 2X-scale text
    display.println(F("Humedad"));
    display.setTextSize(3); // Draw 2X-scale text
    display.println(valor_humedad, 1);
    display.println(F("%"));
    cont_display = 2;
  }
  else if (cont_display == 2)
  {
    display.setTextSize(2); // Draw 2X-scale text
    display.println(F("TVOC"));
    display.setTextSize(3); // Draw 2X-scale text
    display.println(valor_TVOC, 3);
    display.println(F("ppm"));
    cont_display = 3;
  }
  else if (cont_display == 3)
  {
    display.setTextSize(2); // Draw 2X-scale text
    display.println(F("CO2"));
    display.setTextSize(3); // Draw 2X-scale text
    display.println(valor_CO2);
    display.println(F("ppm"));
    cont_display = 4;
  }
  else if (cont_display == 4)
  {
    display.setTextSize(2); // Draw 2X-scale text
    display.println(F("PM2.5"));
    display.setTextSize(3); // Draw 2X-scale text
    display.println(valor_PM2_5);
    display.println(F("ug/m3"));
    cont_display = 5;
  }
  else if (cont_display == 5)
  {
    display.setTextSize(2); // Draw 2X-scale text
    display.println(NH3_gas.queryGasType());
    display.setTextSize(3); // Draw 2X-scale text
    display.println(valor_NH3);
    display.println(F("ppm"));
    cont_display = 6;
  }
  else if (cont_display == 6)
  {
    display.setTextSize(2); // Draw 2X-scale text
    display.println(H2S_gas.queryGasType());
    display.setTextSize(3); // Draw 2X-scale text
    display.println(valor_H2S);
    display.println(F("ppm"));
    cont_display = 7;
  }
  else if (cont_display == 7)
  {
    display.setTextSize(2); // Draw 2X-scale text
    display.println(SO2_gas.queryGasType());
    display.setTextSize(3); // Draw 2X-scale text
    display.println(valor_SO2);
    display.println(F("ppm"));
    cont_display = 8;
  }
  else if (cont_display == 8)
  {
    int pos_med = 0;
    pos_med = (((SCREEN_WIDTH - status_string.length() * 6)) / 2);

    display.setTextSize(1);  // Draw 2X-scale text
    display.setCursor(0, 0); // Start at top-left corner
    display.println(F("SensorID:"));
    display.setCursor(55, 0); // Start at top-left corner
    display.println(SensorID);
    display.setCursor(0, 10); // Start at top-left corner
    display.print(F("IP:"));
    display.println(WiFi.localIP().toString());
    display.setCursor(0, 20); // Start at top-left corner
    display.println(F("SSID:"));
    display.setCursor(35, 20); // Start at top-left corner
    display.println(WiFi.SSID());
    display.setCursor(pos_med, 45); // Start at top-left corner
    display.println(status_string);
    cont_display = 0;
  }

  display.display();
  //  delay(2000);
}

void borrar_archivos_viejos(void)
{
  String filename = "";
  String filename_aux = "";
  Dir dir = SPIFFS.openDir("/");
  DateTime now = rtc.now();

  while (dir.next())
  {
    filename = dir.fileName();
    filename_aux = filename.substring(4, 14) + "T00:00:00";
    DateTime fecha_archivo(filename_aux.c_str());
    //    Serial.print(fecha_archivo.timestamp(DateTime::TIMESTAMP_FULL));
    TimeSpan antiguedad_archivo = now - fecha_archivo;
    //    Serial.print("  ");
    //    Serial.print(antiguedad_archivo.days(), DEC);
    //    Serial.print(" days ");
    if (antiguedad_archivo.days() > 30)
    {
      if (SPIFFS.remove(filename))
        Serial.println("El archivo " + filename + " se borro con exito");
      else
        Serial.println("Error al borrar archivo: " + filename);
    }
    //    Serial.println();
  }
}

byte leer_AEM1000(byte *AEM1000_reg, byte *data)
{
  byte nro_bytes = 0;

  AEM1000_COM.write(AEM1000_reg, 8);
  digitalWrite(DIR_485, LOW);
  delay(50);

  while (AEM1000_COM.available() > 0)
  {
    *data++ = AEM1000_COM.read();
    nro_bytes++;
  }
  digitalWrite(DIR_485, HIGH);

  return nro_bytes;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Index()
{
  SendHTML_Header();
  webpage += F("<h2>Indice</h2>");
  webpage += F("<h3>Chip ID: ") + SensorID + F("</h3>");
  webpage += F("<h3>Versión ") + firmVer + F("</h3>");
  webpage += F("<a href='/download'>[Download]</a><br><br>");
  webpage += F("<a href='/date'>[Fecha]</a><br><br>");
  webpage += F("<a href='/wifi'>[WiFi]</a><br><br>");
  webpage += F("<a href='/sensed'>[Configuración de envío]</a><br><br>");
  webpage += F("<a href='/ultimodato'>[Último dato enviado]</a><br><br>");
  webpage += F("</body></html>");
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void DatoEnviado()
{
  SendHTML_Header();
  if (Status == WL_CONNECTED)
  {
    connect_tic = 0;
    webpage += F("<h2>Dato enviado con exito.</h2>");
  }
  else
  {
    connect_tic = 0;
    webpage += F("<h2>No hay conexión, dato guardado.</h2>");
  }

  webpage += F("<a href='/ultimodato'>[Back]</a><br><br>");

  webpage += F("</body></html>");
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void UltimoDatoEnviado()
{

  SendHTML_Header();

  webpage += F("<h3>");
  webpage += F("El dato enviado fue: <br><br> Temperatura: ") + String(valor_temp, 1) + F(" °C");
  webpage += F("<br> Humedad: ") + String(valor_humedad, 1) + F(" %");
  webpage += F("<br> TVOC: ") + String(valor_TVOC, 3) + F(" ppm");
  webpage += F("<br> CO2: ") + String(valor_CO2) + F(" ppm");
  webpage += F("<br> PM2.5: ") + String(valor_PM2_5) + F(" ug/m3");
  webpage += F("<br> NH3: ") + String(valor_NH3) + F(" ppm");
  webpage += F("<br> SO2: ") + String(valor_SO2) + F(" ppm");
  webpage += F("<br> H2S: ") + String(valor_H2S) + F(" ppm");
  webpage += F("<br><br>Fecha: ") + fechaUltimoDato.timestamp(DateTime::TIMESTAMP_DATE) + F(" Hora: ") + fechaUltimoDato.timestamp(DateTime::TIMESTAMP_TIME);
  webpage += F("</h3>");
  webpage += F("<p>");
  webpage += F("Respuesta Servidor Post:<br>") + respuestaPost;
  webpage += F("<br><br>Respuesta Servidor Local Post:\n<br>") + respuestaPostLocal;
  webpage += F("</p>");

  webpage += F("<a href='/envio'>[Enviar un nuevo dato]</a><br><br>");

  webpage += F("<a href='/'>[Back]</a><br><br>");

  webpage += F("</body></html>");
  SendHTML_Content();
  SendHTML_Stop();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Download()
{ // This gets called twice, the first pass selects the input, the second pass then processes the command line arguments
  if (WebServer.args() > 0)
  { // Arguments were received
    if (WebServer.hasArg("download"))
      SD_file_download(WebServer.arg(0));
    if (WebServer.hasArg("borrar"))
      SD_file_borrar(WebServer.arg(0));
  }
  else
    SelectInput("Descarga de Archivos", "Presione \"descargar\" para bajar el archivo o \"borrar\" para borrarlo", "download", "download");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Config_Sensed()
{ // This gets called twice, the first pass selects the input, the second pass then processes the command line arguments
  if (WebServer.args() > 0)
  { // Arguments were received
    if (WebServer.hasArg("Actualizar"))
      Set_Sensado(WebServer.arg(0));
    else if (WebServer.hasArg("Guardar"))
      SetDatosEnvio(WebServer.arg(0));
  }
  else
  {
    SendHTML_Header();

    webpage += F("<FORM action='/sensed'>"); // Must match the calling argument e.g. '/chart' calls '/chart' after selection but with arguments!
    webpage += F("<h1>Configuracion de envio al servidor:</h1>");
    webpage += F("<p>http://XXX.XXX.X.XXX:XXXX/device/measure</p>");
    webpage += F("<p>IP_Puerto_local: ");
    webpage += F("<input type='text' name='IP_Puerto_local' value='") + IP_Puerto_local + F("'><br></p>");
    webpage += F("<input type='submit' name='Guardar' value='Guardar'><br><br>");
    webpage += F("</FORM>");

    webpage += F("<FORM action='/sensed'>"); // Must match the calling argument e.g. '/chart' calls '/chart' after selection but with arguments!
    webpage += F("<h1>Seleccione el intervalo de sensado:</h1>");
    webpage += F("<p>(Actualmente se esta sensando cada ");
    if (datos_set == "5_m")
      webpage += F("5 minutos)</p>");
    else if (datos_set == "10_m")
      webpage += F("10 minutos)</p>");
    else if (datos_set == "15_m")
      webpage += F("15 minutos)</p>");
    else if (datos_set == "20_m")
      webpage += F("20 minutos)</p>");
    else if (datos_set == "30_m")
      webpage += F("30 minutos)</p>");
    else if (datos_set == "45_m")
      webpage += F("45 minutos)</p>");
    else if (datos_set == "1_h")
      webpage += F("1 hora)</p>");
    else if (datos_set == "2_h")
      webpage += F("2 horas)</p>");
    else if (datos_set == "5_h")
      webpage += F("5 horas)</p>");
    webpage += F("<select name='Sensado' id='Sensado'>");
    webpage += F("<option value='5_m'>5 min</option>");
    webpage += F("<option value='10_m'>10 min</option>");
    webpage += F("<option value='15_m'>15 min</option>");
    webpage += F("<option value='20_m'>20 min</option>");
    webpage += F("<option value='30_m'>30 min</option>");
    webpage += F("<option value='45_m'>45 min</option>");
    webpage += F("<option value='1_h'>1 hora</option>");
    webpage += F("<option value='2_h'>2 horas</option>");
    webpage += F("<option value='5_h'>5 horas</option>");
    webpage += F("</select><br>");
    webpage += F("<input type='submit' id='Actualizar' name='Actualizar' value='Actualizar'><br><br>");
    webpage += F("</FORM>");

    webpage += F("<a href='/'>[Back]</a><br><br>");
    webpage += F("</body></html>");

    SendHTML_Content();
    SendHTML_Stop();
  }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Config_Date()
{ // This gets called twice, the first pass selects the input, the second pass then processes the command line arguments
  if (WebServer.args() > 0)
  { // Arguments were received
    if (WebServer.hasArg("currentDateTime"))
      Set_Date_Time(WebServer.arg(0));
  }
  else
  {
    SendHTML_Header();
    webpage += F("<h2>Ajustar Fecha y Hora</h2>");

    DateTime now = rtc.now();
    webpage += F("<p>Fecha y Hora actual: ") + now.timestamp(DateTime::TIMESTAMP_DATE) + F("&nbsp;&nbsp;&nbsp;&nbsp;") + now.timestamp(DateTime::TIMESTAMP_TIME) + F("</p>");
    webpage += F("<p>Seleccione nueva Fecha y Hora: </p>");
    webpage += F("<FORM action='/date'>"); // Must match the calling argument e.g. '/chart' calls '/chart' after selection but with arguments!
    webpage += F("<input type='datetime-local' id='currentDateTime' name='currentDateTime'>");
    webpage += F("<input type='submit'><br>");
    webpage += F("<a href='/'>[Back]</a><br><br>");

    webpage += F("</FORM></body></html>");

    SendHTML_Content();
    SendHTML_Stop();
  }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SD_file_download(String filename)
{
  File download = SPIFFS.open("/" + filename, "r");
  if (download)
  {
    WebServer.sendHeader("Content-Type", "text/text");
    WebServer.sendHeader("Content-Disposition", "attachment; filename=" + filename);
    WebServer.sendHeader("Connection", "close");
    WebServer.streamFile(download, "application/octet-stream");
    download.close();
  }
  else
    ReportFileNotPresent("download");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SD_file_borrar(String filename)
{
  SendHTML_Header();
  if (SPIFFS.remove("/" + filename))
    webpage += F("<h3>El archivo se borro con exito</h3>");
  else
    webpage += F("<h3>Error al borrar archivo</h3>");
  webpage += F("<a href='/download'>[Back]</a><br><br>");
  webpage += F("</body></html>");
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Set_Date_Time(const String currentDateTime)
{
  rtc.adjust(DateTime(currentDateTime.c_str()));
  //    rtc.adjust(DateTime("2021-11-29T13:45"));
  SendHTML_Header();
  webpage += F("<h3>Fecha y hora actulizada</h3>");
  webpage += F("<a href='/date'>[Back]</a><br><br>");
  webpage += F("</body></html>");
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Set_Sensado(const String sensado_value)
{
  grabar(100, sensado_value);
  datos_set = leer(100);
  //    datos_tic = 0;
  SendHTML_Header();
  webpage += F("<h1>Intervalo de sensado actualizado</h1>");
  webpage += F("<a href='/sensed'>[Back]</a><br><br>");
  webpage += F("</body></html>");
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SetDatosEnvio(const String dato1)
{
  grabar(150, dato1);
  IP_Puerto_local = leer(150);

  SendHTML_Header();
  webpage += F("<h1>Datos de envío guardado</h1>");
  webpage += F("<a href='/sensed'>[Back]</a><br><br>");
  webpage += F("</body></html>");
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SendHTML_Header()
{
  WebServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  WebServer.sendHeader("Pragma", "no-cache");
  WebServer.sendHeader("Expires", "-1");
  WebServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  WebServer.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  append_page_header();
  WebServer.sendContent(webpage);
  webpage = "";
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SendHTML_Content()
{
  WebServer.sendContent(webpage);
  webpage = "";
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SendHTML_Stop()
{
  WebServer.sendContent("");
  WebServer.client().stop(); // Stop is needed because no content length was sent
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SelectInput(String heading1, String heading2, String command, String arg_calling_name)
{
  SendHTML_Header();
  webpage += F("<h3 class='rcorners_m'>");
  webpage += heading1 + "</h3>";

  webpage += F("<h3>   Nombre  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;  Tamaño   </h3>");
  webpage += F("<FORM action='/");
  webpage += command + "' method='post'>"; // Must match the calling argument e.g. '/chart' calls '/chart' after selection but with arguments!

  String filename = "";
  Dir dir = SPIFFS.openDir("/");
  long usedbytes = 0;
  while (dir.next())
  {
    usedbytes += dir.fileSize();
    filename = dir.fileName();
    //    filename.remove(0,1);
    webpage += F("<p>");
    webpage += dir.fileName() + F("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;") + dir.fileSize() + F(" bytes &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
    webpage += F("<button type='submit' name='download' value='") + filename + F("'>descargar</button> ");
    webpage += F("<button type='submit' name='borrar' value='") + filename + F("'>borrar</button> </p>");
  }

  webpage += F("<h3>");
  webpage += heading2 + "</h3>";

  FSInfo fsInfo;
  SPIFFS.info(fsInfo);
  Serial.println(fsInfo.totalBytes);
  webpage += F("<p>Bytes usados: ");
  webpage += String(usedbytes) + F(" / ") + String(fsInfo.totalBytes) + F("</p>");

  if (fsInfo.usedBytes >= MAX_BYTES)
    webpage += F("<h3 style='color:Tomato;'>Memoria llena, por favor borre archivos para seguir grabando información</h3>");

  //  webpage += F("<FORM action='/"); webpage += command + "' method='post'>"; // Must match the calling argument e.g. '/chart' calls '/chart' after selection but with arguments!
  //  webpage += F("<input type='text' name='"); webpage += arg_calling_name; webpage += F("' value=''><br>");
  //  webpage += F("<type='submit' name='"); webpage += arg_calling_name; webpage += F("' value=''><br><br>");
  webpage += F("<br><a href='/'>[Back]</a><br><br>");

  webpage += F("</FORM></body></html>");
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ReportFileNotPresent(String target)
{
  SendHTML_Header();
  webpage += F("<h3>El archivo no existe</h3>");
  webpage += F("<a href='/");
  webpage += target + "'>[Back]</a><br><br>";
  webpage += F("</body></html>");
  SendHTML_Content();
  SendHTML_Stop();
}

void append_page_header()
{
  webpage = F("<!DOCTYPE html><html>");
  webpage += F("<head>");
  webpage += F("<title>File Server</title>"); // NOTE: 1em = 16px
  webpage += F("<meta name='viewport' content='user-scalable=yes,initial-scale=1.0,width=device-width'>");
  webpage += F("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/>");
  webpage += F("<style>");
  webpage += F("</style></head><body>");
}

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-----------------------> Funciones para pantalla Nextion <---------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------

void Leer_Nextion()
{
  int dato_nuevo = 0;
  int i;
  String StringRecibido = "";
  //-------

  // Serial.print("\nLeyendo probando");
  //-----> Verifico si Nextion envio datos por serie
  if (Nextion.available() > 0)
  {
    // Serial.print("\n Probando, dato nuevo");
    int tam = 100;
    int Datos[tam];
    int cont = 0;
    //-----> Vaciamos el vector Datos
    for (int j = 0; j < tam; j++)
    {
      Datos[j] = 0;
    }
    i = 0;
    while (Nextion.available() > 0 && i < tam)
    {
      int k = 0;
      if (Nextion.available() > 0)
      {
        int aux = Nextion.read();
        if (aux != 0)
        {
          Datos[i] = aux;
          i++;
          cont++;
        }
      }

      while (Nextion.available() == 0 && k < 10)
      {
        k++;
        delay(1);
      }
    }

    for (i = 0; i < cont; i++)
    {
      if (Datos[i] != 255 && (Datos[i] != 0))
      {
        StringRecibido += char(Datos[i]);
        dato_nuevo = 1;
      }
    }
  }
  //-----> Solo entramos si hay un dato en el puerto serie

  //-----> Si hay un dato nuevo, analizamos que formato tiene.
  if (dato_nuevo == 1)
  {
    Nex_Leer_Pass_Ssid(StringRecibido);
    Devolver_Pagina(StringRecibido);
    Leer_Sensado(StringRecibido);

    if (StringRecibido == "<Reescanear>")
    {
      Nex_pag_actual = "Escanear";
      flag_escanear = 0;
    }

    //-----------------------PAGINA INICIO
    if (Nex_pag_actual == "Inicio" && flag_inicio == 0)
    {
      Nex_Pantalla_Inicio();
      Nex_Pantalla_Inicio_Mediciones();
      flag_inicio = 1;
    }

    //-----------------------PAGINA CONFIGURACION
    if (Nex_pag_actual == "Conf" && flag_conf == 0)
    {
      flag_conf = 1;
      Serial.print("\n en el if de config");
      String aux = "";
      Nex_Mostrar_Pag(ID_configuracion);
      Serial.print("\n Estado de Red: " + Estado_red);
      for (int j = 0; j < 3; j++)
      {
        if (Estado_red == "desconectado")
        {
          // No se pudo conectar:
          Serial.print("\n\nNextion Configuracion: desconectado");

          Enviar_Nextion("0");
          Enviar_Nextion("Texto7.txt=\"Estado: desconectado\"");

          Enviar_Nextion("Texto2.txt=\"" + String(SensorID) + "\"");
          aux = "Sensor_" + String(SensorID);
          Enviar_Nextion("Texto4.txt=\"" + aux + "\"");

          aux = "Texto6.txt=\"" + WiFi.softAPIP().toString() + "\"";
          Enviar_Nextion(aux);
        }

        if (Estado_red == "conectado")
        {
          // Conectado:
          Serial.print("\n\nNextion Configuracion: conectado");

          Enviar_Nextion("0");
          Enviar_Nextion("Texto7.txt=\"Estado: conectado\"");

          Enviar_Nextion("Texto2.txt=\"" + String(SensorID) + "\"");

          Enviar_Nextion("Texto4.txt=\"" + String(ssid) + "\"");

          Enviar_Nextion("Texto6.txt=\"" + WiFi.localIP().toString() + "\"");
        }

    
      }
    }
    //---------------------------------------------------------------------------------

    //------- PAGINA PASSWORD
    if (Nex_pag_actual == "Password" && flag_password == 0)
    {
      for (int i = 0; i < 4; i++)
      {
        String aux1 = "texto2.txt=\"Red seleccionada: " + String(ssid) + "\"";
        Enviar_Nextion(aux1);
        Serial.print("\nenviando la red seleccionada");
      }
      flag_password = 1;
    }
    //---------------------------------------------------------------------------------

    //----------- PAGINA ESCANEAR
    if (Nex_pag_actual == "Escanear" && flag_escanear == 0) //---- En la pagina ESCANEO
    {
      flag_escanear = 1;
      Nex_Mostrar_Pag(ID_escaneo);
      Serial.println("\nIniciando escaneo");
      int cant_redes = WiFi.scanNetworks(); // devuelve el número de redes encontradas
      // Serial.println("\nEscaneo terminado");
      if (cant_redes == 0) // Si no encuentra ninguna red
      {
        Serial.println("no se encontraron redes");
      }
      else // Se encontraron redes
      {
        for (int j = 0; j < 3; j++)
        {
          Serial.print("\n " + String(cant_redes) + " redes encontradas!");

          Enviar_Nextion("vis Texto,0");
          Enviar_Nextion("vis BotonMostrar,1");
          Enviar_Nextion("vis BotonVolver,1");
          Enviar_Nextion("vis BotonRecargar,1");
          for (int i = 0; i < cant_redes; ++i)
          {
            String str_aux = "";
            str_aux = "Red" + String(i + 1) + ".txt=\"< " + String(i + 1) + " > : " + WiFi.SSID(i) + "\"";
            Enviar_Nextion(str_aux);
            str_aux = "vis Red" + String(i + 1) + ",1";
            Enviar_Nextion(str_aux);
          }
        }
      }
    } // fin de escanear
    //---------------------------------------------------------------------------------
  }
  //-----> FIN DE DATO NUEVO.

  // Nex_Pantalla_Inicio_Mediciones();

} //-----> Fin de la funcion Leer Nextion

/* Funcion ENVIAR A NEXTION */
void Enviar_Nextion(String str)
{
  Nextion.write(0xff);
  Nextion.write(0xff);
  Nextion.write(0xff);

  if (str.compareTo("0") != 0)
  {
    // Serial.print("\n Enviando comando: " + str);
    Nextion.print(str);
  }
  Nextion.write(0xff);
  Nextion.write(0xff);
  Nextion.write(0xff);
}
/* Fin de funcion */

/* Funcion Vaciar Puerto Serie */
void Vaciar_Serie()
{
  while (Nextion.available() > 0)
  {
    Nextion.read();
    delay(10);
  }
}
/* Fin de funcion */

/* Funcion PREGUNTAR PAGINA */
// Devuelve el numero de pagina en una variable tipo "int"
// No hay que mandarle ningun parametro
// Si hay algun error retorna -1
int Preguntar_Pagina()
{
  // Enviamos 0xFF 0xFF 0xFF y despues vaciamos el puerto!
  /*Enviar_Nextion("0");
  while (Nextion.available() > 0)
    Nextion.read();
  */
  // Declaracion de variables:
  int retorno = -1;
  String str = "";
  int j = 0;
  int fin = 0;
  int cont = 0;

  while (Nextion.available() > 0)
  {
    Nextion.read();
    delay(1);
  }

  while (fin == 0 && cont < 3)
  {
    // Enviamos el comando:
    Enviar_Nextion("sendme");

    // Mientras no haya una respuesta esperamos:
    
    while (Nextion.available() == 0 && j < 1000)
    {
      delay(1);
      j++;
    }


    // Usando Serial.readString() funciona bien!
    str = Nextion.readString();
    int tam = str.length();
    int i = 0;

    //Serial.print("\nstr leido del puerto: " + str);

    // Verificamos que el formato sea el correcto!
    if (j < 1000)    {
      while (i < tam && fin == 0)
      {
        if (str[i] == 102)
          if (str[i + 2] == 255)
          // if (str[3] == 255)
          // if (str[4] == 255)
          {
            int pagina = str[i + 1];
            retorno = pagina;
            fin = 1;
          }
        i++;
      }
    }
    cont++;
  }
  //Serial.print("\nRetorno antes de salir: " + String(retorno));
  return retorno;
}
/* Fin de la funcion */

// Funcion para buscar en el string recibido la palabra "<PAGE:"
// Devuelve el nombre de la pagina
String Devolver_Pagina(String StringRecibido)
{
  // Serial.print("\nString recibido en funcion: "+StringRecibido);
  String page = Nex_pag_actual;
  int retorno_1 = StringRecibido.indexOf("<P:");
  if (retorno_1 != -1)
  {
    int indice = StringRecibido.indexOf("<P:");
    indice = StringRecibido.indexOf(":", indice);
    int indice_2 = StringRecibido.indexOf(">", indice);
    page = StringRecibido.substring(indice + 1, indice_2);
    // Serial.print("\nIndice 1: "+String(indice));
    // Serial.print("\nIndice 2: "+String(indice_2));
    Nex_pag_actual = page;
    Serial.print("\nPAGE Leida (en funcion): " + Nex_pag_actual);
    //----------
    flag_rojo_negro = 0;
    flag_conf = 0;
    flag_inicio = 0;
    flag_escanear = 0;
    flag_password = 0;
    //----------
  }
  return page;
}

String Leer_Nextion_Serie()
{
  int i;
  int tam = 100;
  int Datos[tam];
  String StringRecibido = "";
  int cont = 0;

  //-----> VACIAMOS EL VECTOR DE DATOS
  for (int j = 0; j < tam; j++)
  {
    Datos[j] = 0;
  }

  if (Nextion.available() > 0)
  {

    i = 0;
    while (Nextion.available() > 0 && i < tam)
    {
      int k = 0;

      if (Nextion.available() > 0)
      {
        int aux = Nextion.read();
        if (aux != 0)
        {
          Datos[i] = aux;
          i++;
          cont++;
        }
      }

      while (Nextion.available() == 0 && k < 1000)
      {
        k++;
        delay(1);
      }
    }

    for (i = 0; i < cont; i++)
    {
      if (Datos[i] != 255 && (Datos[i] != 0))
      {
        StringRecibido += char(Datos[i]);
      }
    }
  }
  return StringRecibido;
}

int Nex_Leer_Pass_Ssid(String StringRecibido)
{
  int retorno = -1;
  int retorno_2 = StringRecibido.indexOf("<PASS:");
  if (retorno_2 != -1)
  {
    int indice = StringRecibido.indexOf(':');
    int indice_2 = StringRecibido.indexOf(">", indice);
    String aux = StringRecibido.substring(indice + 1, indice_2);

    for (int i = 0; i < 50; i++)
      pass[i] = 0;

    Serial.print("\n Pass seleccionada: " + aux);
    int tam = aux.length();
    for (int i = 0; i < tam; i++)
    {
      pass[i] = aux[i]; // Copiamos el string a la cadena
    }
    retorno = 1;
    Serial.print("\nPass grabada: " + String(pass));
    EEPROM.put(50, pass);
    EEPROM.commit();
    EEPROM.put(0, ssid);
    EEPROM.commit();

    Nex_Pantalla_Conectando();
  }
  else
  {
    int retorno_1 = StringRecibido.indexOf("<Red:");
    if (retorno_1 != -1)
    {
      int indice = StringRecibido.indexOf(':');
      int indice_2 = StringRecibido.indexOf(">", indice);
      String red = StringRecibido.substring(indice + 1, indice_2);

      String aux = WiFi.SSID(red.toInt() - 1);
      Serial.print("\n Red seleccionada: " + aux);
      int tam = aux.length();

      for (int i = 0; i < 50; i++)
        ssid[i] = 0;

      for (int i = 0; i < tam; i++)
      {
        ssid[i] = aux[i]; // Copiamos el string a la cadena
      }
      Serial.print("\nRed grabada: " + String(ssid));
      retorno = 1;
    }
  }
  return retorno;
}

void Leer_Sensado(String StringRecibido)
{

  int retorno_2 = StringRecibido.indexOf("<Sens:");
  if (retorno_2 != -1)
  {
    int indice = StringRecibido.indexOf(':');
    int indice_2 = StringRecibido.indexOf(">", indice);
    String aux = StringRecibido.substring(indice + 1, indice_2);

    Serial.print("\n Sensado configurado: " + aux);
    //---------------

    if (aux == "2")
    {
      datos_set == "2_m";
      connect_tic = 120000;
    }
    else if (aux == "3")
    {
      datos_set == "3_m";
      connect_tic = 180000;
    }
    else if (aux == "5")
    {
      datos_set == "5_m";
      connect_tic = 300000;
    }
    else if (aux == "10")
    {
      datos_set == "10_m";
      connect_tic = 600000;
    }
    else if (aux == "15")
    {
      datos_set == "15_m";
      connect_tic = 900000;
    }
    else if (aux == "20")
    {
      datos_set == "20_m";
      connect_tic = 1200000;
    }
    else if (aux == "30")
    {
      datos_set == "30_m";
      connect_tic = 1800000;
    }
    else if (aux == "45")
    {
      datos_set == "45_m";
      connect_tic = 2700000;
    }
    else if (aux == "60")
    {
      datos_set == "1_h";
      connect_tic = 3600000;
    }
    else if (aux == "120")
    {
      datos_set == "2_h";
      connect_tic = 7200000;
    }
    else if (aux == "300")
    {
      datos_set == "5_h";
      connect_tic = 18000000;
    }
    else if (aux == "720")
    {
      datos_set == "12_h";
      connect_tic = 43200000;
    }
    //**************/
  }
}

void Nex_Pantalla_Inicio()
{
  String aux = "";
  delay(100);
  Nex_Mostrar_Pag(ID_inicio);
  Nex_pag_actual = "Inicio";
  for (int j = 0; j < 3; j++)
  {
    if (Estado_red == "desconectado")
    {
      // No se pudo conectar:
      // Serial.print("\n\nNextion inicio: desconectado");

      Enviar_Nextion("0");
      Enviar_Nextion("Estado.txt=\"Estado: desconectado\"");
    }

    if (Estado_red == "conectado")
    {
      // Conectado:
      // Serial.print("\n\nNextion inicio: conectado");

      Enviar_Nextion("0");
      Enviar_Nextion("Estado.txt=\"Estado: conectado\"");
    }

    Enviar_Nextion("vis BotonConf,1");
    Enviar_Nextion("vis p0,1");
  }
}

void Nex_Pantalla_Conectando()
{
  Nex_Mostrar_Pag(ID_conectando);
  /*
  for (int j = 0; j < 3; j++)
  {
    Enviar_Nextion("Texto2.txt=\"" + String(ssid) + "\"");
  }
  */
  delay(3000);
  setup_wifi();
}

void Nex_Pantalla_Inicio_Mediciones()
{

  for (int i = 0; i < 3; i++)
  {
    Enviar_Nextion("0");

    Enviar_Nextion("Dato1.txt=\"" + String(valor_temp) + "\"");

    Enviar_Nextion("Dato2.txt=\"" + String(valor_humedad) + "\"");

    Enviar_Nextion("Dato3.txt=\"" + String(valor_TVOC) + "\"");

    Enviar_Nextion("Dato4.txt=\"" + String(valor_CO2) + "\"");

    Enviar_Nextion("Dato5.txt=\"" + String(valor_PM2_5) + "\"");

    Enviar_Nextion("Dato6.txt=\"" + String(valor_NH3) + "\"");

    Enviar_Nextion("Dato7.txt=\"" + String(valor_H2S) + "\"");

    Enviar_Nextion("Dato8.txt=\"" + String(valor_SO2) + "\"");
  }
}

int Nex_Mostrar_Pag(int pag_enviar)
{

  int retorno = -1;
  int cont = 0;
  String aux = "page " + String(pag_enviar);
  Serial.print("\nSe envia pag: " + aux);
  Enviar_Nextion(aux);

  int pag = Preguntar_Pagina();
  //Serial.print("\nPag Leida: " + String(pag));
  while (pag != pag_enviar && cont < 4)
  {
    Enviar_Nextion(aux);
    pag = Preguntar_Pagina();
    //Serial.print("\nPag Leida en el while: " + String(pag));
    cont++;
  }
  if (cont == 4)
  {
    Serial.print("\nError al cargar la pagina!");
  }
  else
  {
    retorno = 1;
  }
  return retorno;
}