//prueba botones

#define CANT_BOTONES 4
//{ Subir , Bajar , ON , OFF }
int BOTONES[CANT_BOTONES] = {D5, D6, D7};

#define BOTON_SUBIR 0
#define BOTON_BAJAR 1
#define BOTON_ON 2

int j=0;

void setup() {
  Serial.begin(9600);
  pinMode(D5, INPUT_PULLUP);
  pinMode(D6, INPUT_PULLUP);
  pinMode(D7, INPUT_PULLUP);

}

void loop() {

  int subir = Funcion_Boton_Retardo(BOTON_SUBIR, BOTON_BAJAR);

}

//******************************************
//--- Funcion para leer el estado del botón CON/SIN RETARDO.
//--- La funcion devuelve 1 si se presiona el botón
//--- La funcion devuelve 2 si me mantiene presionado un
// tiempo mayor a "retardo_ms"
//--- devuelve 0 si no está presionado.

int Funcion_Boton_Retardo (int boton1, int boton2) {

  int retardo_ms = 1500; // Cantidad de milisegundos que quiero
  //que espere cuando mantengo presionado el boton.
  int boton_presionado = 0, retorno = 0;
  int i = 0;
  int uno, dos,x;
  uno = 0;
  dos = 0;

  //Miro si estan ambos presionados al mismo tiempo
  if (digitalRead(BOTONES[boton1]) == LOW && digitalRead(BOTONES[boton2]) == LOW) {
    while (digitalRead(BOTONES[boton1]) == LOW && digitalRead(BOTONES[boton2]) == LOW && dos < retardo_ms) {
      delay(1);
      dos++;
    }
    if (dos == retardo_ms) { //Se mantuvieron los dos presionados x segundos
      //carpeta menu
      Serial.println("Menu");
    }
  } else if (digitalRead(BOTONES[boton1]) == LOW) {
    //funcion boton 1
    Serial.println("Funcion boton 1");
    Funcion_Boton_Subir(boton1,boton2);
  } else if (digitalRead(BOTONES[boton2]) == LOW) {
    //funcion boton 2
    Serial.println("Funcion boton 2");
    Funcion_Boton_Bajar(boton1,boton2);
  }

  delay(50);
  //*****************
  //} //cierre del if del boton presionado.

  return retorno;
}
//--- Fin de la función


//************ FUNCION SUBIR ***********************
void Funcion_Boton_Subir(int boton1, int boton2) {
  int flag = 0;
  int retardo = 400;
  int uno = 0;
  int x,i;
  i=0;
  x=0;
  
  //*** Subo litros +1
  
  //***

  while (digitalRead(BOTONES[boton1]) == LOW && digitalRead(BOTONES[boton2]) == HIGH && uno < retardo) {
    delay(1);
    uno++;
  }

  if (uno == retardo) { //Se presiono uno solo
    //cont automatico.

    //****************************** conteo automatico
    while (digitalRead(BOTONES[boton1]) == LOW && flag == 0) {
      delay (1);
      i++;
      if (digitalRead(BOTONES[boton1]) == LOW && digitalRead(BOTONES[boton2]) == LOW) {
        flag = 1;
        Funcion_Boton_Retardo (boton1, boton2);
      } else {
        if (i > 1000 && i < 3000) {
          x++;
          if (x == 300) {
            j++;
            Serial.print(j);
            Serial.print("\t");
            x = 0;
          }
        }
        if (i > 3000 && i < 8000) {
          if (x > 170)
            x = 0;
          x++;
          if (x == 170) {
            j++;
            Serial.print(j);
            Serial.print("\t");
            x = 0;
          }
        }
        if (i > 8000) {
          if (x > 100)
            x = 0;
          x++;
          if (x == 100) {
            j++;
            Serial.print(j);
            Serial.print("\t");
            x = 0;
          }
        }
      } //llave else
    } //llave while

    //******************************
  } // if uno>400
} //cierre funcion

//**************************************************
//************ FUNCION BAJAR ***********************
void Funcion_Boton_Bajar(int boton1, int boton2) {
  int flag = 0;
  int retardo = 400;
  int uno = 0;
  int x,i;
  i=0;
  x=0;
  
  //*** Subo litros -1

  //***

  while (digitalRead(BOTONES[boton2]) == LOW && digitalRead(BOTONES[boton1]) == HIGH && uno < retardo) {
    delay(1);
    uno++;
  }

  if (uno == retardo) { //Se presiono uno solo
    //cont automatico.

    //****************************** conteo automatico
    while (digitalRead(BOTONES[boton2]) == LOW && flag == 0) {
      delay (1);
      i++;
      if (digitalRead(BOTONES[boton1]) == LOW && digitalRead(BOTONES[boton2]) == LOW) {
        flag = 1;
        Funcion_Boton_Retardo (boton1, boton2);
      } else {
        if (i > 1000 && i < 3000) {
          x++;
          if (x == 300) {
            j--;
            Serial.print(j);
            Serial.print("\t");
            x = 0;
          }
        }
        if (i > 3000 && i < 8000) {
          if (x > 170)
            x = 0;
          x++;
          if (x == 170) {
            j--;
            Serial.print(j);
            Serial.print("\t");
            x = 0;
          }
        }
        if (i > 8000) {
          if (x > 100)
            x = 0;
          x++;
          if (x == 100) {
            j--;
            Serial.print(j);
            Serial.print("\t");
            x = 0;
          }
        }
      } //llave else
    } //llave while

    //******************************
  } // if uno>400
} //cierre funcion
