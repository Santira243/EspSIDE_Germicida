//--- VERSIÓN: 26/04/2020 ----//
#define PIR1 2//----PIN Sensor de movimiento 1
#define PIR2 3//----PIN Sensor de movimiento 2
#define LED_TESTIGO 5//----PIN Led de BOTÓN, indica lámparas prendidas y alarmas.
#define LED_RADIACION 12//----PIN Led indica tensión al inicio de ciclo y lámparas prendidas
#define BUZZER 6//----PIN Buzzer
#define BOTON 10//----PIN Boton Inicio de ciclo
//----Tiempos definidos en milisegundos----//
#define TIEMPO_CICLO 300000 //----Duración de CICLO (lámparas encendidas)
#define TIEMPO_HISTERESIS 7000 //----Demora en estabilizar los PIR (para inhabilitar sensado)
#define TIEMPO_ALARMA_FIN 4000 //----Duración alarma final
#define TIEMPO_SALIR 30000 //----Duración de alarma previo a iniciar el CICLO
#define TIEMPO_PAUSA 30000 //----Duración de alarma PAUSAS (movimiento/serial)
//----Banderas----//
bool ciclo=false;//----ciclo activado/desactivado
bool comenzado=false;//----comienzo de nuevo ciclo
bool lectura_boton=false; //----boton apretado/no apretado
bool lectura_serial_A=false; //----letra 'A-Apagar' recibida/no recibida
bool lectura_serial_E=false; //----letra 'E-Encender' recibida/no recibida
bool lectura_serial_P=false; //----letra 'P-Pausar' recibida/no recibida
bool fin_histeresis=false;//----fin de alarma histéresis
bool luz_encendida=false;//----encendido/apagado de las lámparas
bool pausa=false;//----dar inicio a la pausa
//----Tiempos en los que sucede algún evento: millis()----//
unsigned long tiempo;//----actual
unsigned long tiempo_inicio; //----evento inicio (serial/boton)
unsigned long tiempo_pausa; //---pausa (movimiento/serial)
unsigned long tiempo_encendido; //----encendido lámparas
unsigned long tiempo_ciclo;//----inicio de ciclo
unsigned long tiempo_alarma;//----fin de alarma
unsigned long tiempo_final;//----inicio de ciclo
//----Contadores para control de tiempos varios----//
unsigned long contador_ciclo; //----tiempo con luces encendidas
unsigned long pausa_acumulada; //----tiempo en pausa
unsigned int contador_boton;//----registro de presión sostenida en botón


void setup() 
 {
  Serial.begin(115200);
  pinMode(LED_TESTIGO, OUTPUT);
  pinMode(LED_RADIACION, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BOTON, INPUT_PULLUP); 
  pinMode(PIR1, INPUT);
  pinMode(PIR2, INPUT);
  contador_boton=0;
  pausa_acumulada=0;
  contador_ciclo=0;
  fin_histeresis=false;
  digitalWrite(LED_RADIACION, HIGH); //Avisa que esta con tensión.
 }
void alarma(unsigned int t, unsigned long t_prender)
  {
    if((tiempo - t_prender)<floor(t/3))
    { 
      beep(500);
    }
    if(((tiempo - t_prender)>floor(t/3)) &&  ((tiempo - t_prender) < floor(2*t/3) ))
    {
      beep(300);
    }
    if((tiempo - t_prender)>floor(2*t/3))
    { 
      beep(100);
    }
  }
void beep(int periodo)
  {
    if (abs(tiempo - tiempo_alarma) < periodo)
        {
        analogWrite(BUZZER, 440);
        digitalWrite(LED_TESTIGO, HIGH);
        }
    if(abs(tiempo - tiempo_alarma) > periodo)
        {
        analogWrite(BUZZER, 0);
        digitalWrite(LED_TESTIGO, LOW);   
        }

    if(abs(tiempo - tiempo_alarma) > 2*periodo)
        {
        tiempo_alarma=millis();
        }
  }
void botonEvent(bool btninit)
  {
  if(!btninit)
      {  
          delay(300);//----si estoy pulsando, cuento 300 ms
          contador_boton++;//---contador de pulsado
      }
      if (contador_boton > 6)//---al superar 1800 ms de pulsación, levanta bandera de evento boton
      {
        contador_boton=0;
        lectura_boton=true;
        tiempo_inicio=millis();
        digitalWrite(LED_RADIACION, LOW); // Leyó el botón
      }
  }
void serialEvent() 
  {
    char inChar;
    if (Serial.available())//----Si está leyendo el serial, traemos el char y seteamos banderas----//
    {
      inChar=(char)Serial.read();//----ESP envía 'A' para APAGAR
      if(inChar=='A')
      {
        lectura_serial_E=false;
        lectura_serial_P=false;
        lectura_serial_A=true;
      }
      if(inChar=='E')//----ESP envía 'E' para ENCENDER
      {
        lectura_serial_E=true;
        lectura_serial_P=false;
        lectura_serial_A=false;
        tiempo_inicio=millis();
        digitalWrite(LED_RADIACION, LOW);
      }
      if(inChar=='P')//----ESP envía 'P' para PAUSAR
      {
        lectura_serial_E=false;
        lectura_serial_P=true;
        lectura_serial_A=false;
        tiempo_pausa=millis();
      }
    }
  }
void iniciar() 
  {
    if((tiempo - tiempo_inicio)<TIEMPO_SALIR)//----si estoy dentro del tiempo de salida acciona la alarma
      {
        alarma(TIEMPO_SALIR,tiempo_inicio);
      }
    else//----si terminó el tiempo de salida inicia el ciclo
    {
      tiempo_ciclo=millis();
      digitalWrite(LED_TESTIGO, HIGH);   
      ciclo=true;
      comenzado=false;
    }
  }
void pausar()
  {
    if(luz_encendida)
      {
        apagarLuz();
      }
    tiempo=millis();
    if((tiempo - tiempo_pausa)<TIEMPO_PAUSA)
      {
        alarma(TIEMPO_PAUSA,tiempo_pausa);
      }
    else
      {
        pausa_acumulada+=TIEMPO_PAUSA;
        pausa=false;
      }
  }
void finCiclo()
  {
    analogWrite(BUZZER, 440);
    digitalWrite(LED_TESTIGO, HIGH);
    delay(2000);
    analogWrite(BUZZER, 0);
    //---Reiniciamos las variables
    digitalWrite(LED_RADIACION, HIGH);
    digitalWrite(LED_TESTIGO, LOW);
    ciclo=false;
    comenzado=false;
    lectura_boton=false;
    luz_encendida=false;
    pausa=false;
    fin_histeresis=false;
  }
void encenderLuz()
  {
    digitalWrite(LED_RADIACION, HIGH);
    digitalWrite(LED_TESTIGO, HIGH);
    Serial.write('O');
    fin_histeresis=false;
    tiempo_encendido=millis();
    luz_encendida=true;
  }
void apagarLuz()
 {
    Serial.write('S');
    luz_encendida=false;
    lectura_serial_E=false;
    digitalWrite(LED_TESTIGO,LOW);
    digitalWrite(LED_RADIACION, LOW);
    fin_histeresis=false;
 }      
void loop() 
  {
    tiempo=millis();
    botonEvent(digitalRead(BOTON));
    serialEvent();
    //---LECTURA DE EVENTOS DE INICIO DE CICLO
    if (lectura_boton || lectura_serial_E)
      {
        tiempo=millis();
        comenzado=true;
      }  
    //---ACTIVACIÓN DE ALARMA INICIAL Y HABILITACIÓN DE CICLO
    if (comenzado && (!ciclo))
      {
        iniciar();
        pausa_acumulada=0;
        contador_ciclo=0;
      }
    //---CONTROL GENERAL DE ESTADOS EN EL CICLO
    if(ciclo)
      {
        tiempo=millis();
        contador_ciclo=tiempo-tiempo_ciclo-pausa_acumulada;
        //---VERIFICACIÓN DEL TIEMPO MÁXIMO DE CICLO
        if (contador_ciclo<TIEMPO_CICLO)
          {
            //---VERIFICACIÓN DE EVENTO SERIAL DE APAGADO
            if (lectura_serial_A)
              {
                lectura_serial_A=false;
                apagarLuz();
                finCiclo();
              }
            //---SÓLO SE ENCIENDE LUZ SI NO ESTÁ ENCENDIDA, NI EN UNA PAUSA, CON CICLO INICIADO       
            if ((!pausa)&&(ciclo))
              {
                if (!luz_encendida)
                  {
                    encenderLuz();
                  }
                else //----MANEJAMOS LA HISTERESIS
                  {
                    if (TIEMPO_HISTERESIS<((tiempo - tiempo_encendido)))
                    {
                      fin_histeresis=true;
                    }
                  }
              }
            //---SENSADO DE MOVIMIENTO ACTIVADO AL FINALIZAR HISTÉRESIS SI NO HAY PAUSA INICIADA
            if((fin_histeresis)&&(!pausa))
              {
                if((digitalRead(PIR1)) || (digitalRead(PIR2)))
                  {
                    pausa = true;
                    tiempo_pausa = millis();
                  }
              } 
            //---SENSADO WEB ACTIVADO SI NO HAY PAUSA INICIADA
            if (lectura_serial_P && (!pausa))
              {
                lectura_serial_P=false;
                pausa = true;
                tiempo_pausa = millis();
              }
          if (pausa)
            {
              pausar();//---alarma sonora y fin de pausa
            }
          }
        //---SE FINALIZA AL CUMPLIR TIEMPO_CICLO
        else 
          {
            apagarLuz();//----alarma sonora y envia señal de apagado
            finCiclo();
          }
      } 
  }
