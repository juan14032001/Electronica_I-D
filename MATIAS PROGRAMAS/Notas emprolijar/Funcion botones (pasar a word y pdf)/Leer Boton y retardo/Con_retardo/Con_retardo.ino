//prueba botones

#define CANT_BOTONES 4
//{ Subir , Bajar , ON , OFF }
int BOTONES[CANT_BOTONES] = {D5, D7, D6};

#define BOTON_SUBIR 0
#define BOTON_BAJAR 1
#define BOTON_ON 2

void setup() {
  Serial.begin(9600);
  pinMode(D5, INPUT_PULLUP);
  pinMode(D6, INPUT_PULLUP);
  pinMode(D7, INPUT_PULLUP);
}

void loop() {

  int subir = Funcion_Boton_Retardo(BOTON_SUBIR);
  if (subir == 1) {
    Serial.println("SUBIR");
  }
  else if (subir == 2) {
    Serial.println("SUBIR con retardo");
  }

  int bajar = Funcion_Boton_Retardo(BOTON_BAJAR);
  if (bajar == 1) {
    Serial.println("BAJAR");
  }
  else if (bajar == 2) {
    Serial.println("BAJAR con retardo");
  }

  int onoff = Funcion_Boton_Retardo(BOTON_ON);
  if (onoff == 1) {
    Serial.println("ON");
  }
  else if (onoff == 2) {
    Serial.println("ON con retardo");
  }
}

//******************************************
//--- Funcion para leer el estado del botón CON/SIN RETARDO.
//--- La funcion devuelve 1 si se presiona el botón
//--- La funcion devuelve 2 si me mantiene presionado un
// tiempo mayor a "retardo_ms"
//--- devuelve 0 si no está presionado.

int Funcion_Boton_Retardo (int codigo_boton) {

  int retardo_ms = 1500; // Cantidad de milisegundos que quiero
  //que espere cuando mantengo presionado el boton.
  int boton_presionado = 0, retorno = 0;
  int i;

  //******
  i = 0;

  if (digitalRead(BOTONES[codigo_boton]) == LOW)
  {
    boton_presionado = 1;
    while (digitalRead(BOTONES[codigo_boton]) == LOW && i < retardo_ms) {
      delay (1);
      i++;
    }
  }
  //------
  // Sin retardo
  if (boton_presionado == 1 && i < retardo_ms)
  {
    boton_presionado = 0;
    retorno = 1;
    while (digitalRead(BOTONES[codigo_boton]) == LOW)
      delay (1);
  }

  //------
  // Con Retardo
  if (boton_presionado == 1 && i == retardo_ms)
  {
    boton_presionado = 0;
    retorno = 2;
    while (digitalRead(BOTONES[codigo_boton]) == LOW)
      delay (1);
  }

  return retorno;
}
//--- Fin de la función
//***********************************************
