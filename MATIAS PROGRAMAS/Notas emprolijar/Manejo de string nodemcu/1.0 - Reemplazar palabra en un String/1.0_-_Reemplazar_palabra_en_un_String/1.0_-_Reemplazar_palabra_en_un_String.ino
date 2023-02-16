void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.print("\nIngrese una oracion: ");
  while (Serial.available() == 0)
    delay(1);
  String texto = Serial.readStringUntil('\n');
  Serial.print("\nSe ingreso: " + texto);

  Serial.print("\nIngrese que palabra quiere reemplazar: ");
  while (Serial.available() == 0)
    delay(1);
  String palabra = Serial.readStringUntil('\n');
  Serial.print("\nSe ingreso: " + palabra);

  Serial.print("\nIngrese con que reemplazarla: ");
  while (Serial.available() == 0)
    delay(1);
  String reemplazo = Serial.readStringUntil('\n');
  Serial.print("\nSe ingreso: " + reemplazo);

  String nuevo = Reemplazar_String(texto, palabra, reemplazo);
  Serial.print("\nEl string quedo asi:" + nuevo);

  delay(5000);
}

//---------- Funcion Reemplazar ----------
//Devuelve el string con el cambio realizado!
//Si no encuentra la palabra a reemplazar devuelve ERROR

String Reemplazar_String(String string_origen, String palabra_reemplazar, String reemplazo) {

  //Declaracion de variables:
  int indice_1;
  String string_aux_1;
  int largo_palabra;
  int indice_2;
  String string_aux_2;
  String string_final = "ERROR";

  if (string_origen != "" && palabra_reemplazar != "" && reemplazo != "" ) {
    indice_1 = string_origen.indexOf(palabra_reemplazar);

    if (indice_1 != -1) {

      string_aux_1 = string_origen.substring(0, indice_1);

      largo_palabra = palabra_reemplazar.length();

      indice_2 = indice_1 + largo_palabra;
      string_aux_2 = string_origen.substring(indice_2);

      string_final = string_aux_1 + reemplazo + string_aux_2;

      Serial.print("\n\n\n\n");
      Serial.print("\nString origen:\t" + string_origen);
      Serial.print("\nPalabra a reemplazar:\t" + palabra_reemplazar);
      Serial.print("\nReemplazo:\t" + reemplazo);
      Serial.print("\nIndice_1:\t" + String(indice_1));
      Serial.print("\nAux 1:\t" + string_aux_1);
      Serial.print("\nAux 2:\t" + string_aux_2);
      Serial.print("\nTamaño:\t" + String(largo_palabra));
      Serial.print("\nIndice_2:\t" + String(indice_2));
      Serial.print("\nString final:\t" + string_final);
      Serial.print("\n\n\n\n");
    }
  }
  return string_final;
}
