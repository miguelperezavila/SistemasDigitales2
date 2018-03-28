/*
 * juego.c
 *
 *  Created on: 11 de may. de 2016
 *      Authors: Víctor Quión Ranera & Miguel Pérez Ávila
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wiringPi.h>
#include "fsm.h"
#include "tmr.h"

//FLAG
#define FLAG_BOTON_START_END	0x01
#define FLAG_BOTON 				0x01
#define FLAG_TIMER				0x02
//ALIMENTACION +
#define GPIO_ALIMENTACION	0 // pin 1 salida
//BOTONES
//BOTON START/END
#define GPIO_BUTTON_START_END	19 //pin 8
//BOTONES JUGADOR 1
#define GPIO_BUTTON_1			20 //pin 14
#define GPIO_BUTTON_2			21 //pin 15
#define GPIO_BUTTON_3			26 //pin 16
#define GPIO_BUTTON_4 			27 //pin 17
//BOTONES JUGADOR 2
#define GPIO_BUTTON_5 			5 // pin 1
#define GPIO_BUTTON_6			6 // pin 2
#define GPIO_BUTTON_7			12 // pin 3
#define GPIO_BUTTON_8 			13 // pin 4

//LEDS
//LED ESTART/END
#define GPIO_LED_START	18 // pin 17 salida
//LEDS JUGADOR 1
#define GPIO_LED_1		22 //pin 18 salida
#define GPIO_LED_2		23 // pin 19 salida
#define GPIO_LED_3		24 //pin 20 salida
#define GPIO_LED_4		25 // pin 21 salida
//LEDS JUGADOR 2
#define GPIO_LED_5		1 // pin 2 salida
#define GPIO_LED_6		2 // pin 3 salida
#define GPIO_LED_7		3 // pin 4 salida
#define GPIO_LED_8		4 // pin 5 salida

//CONSTANTES
#define CLK_MS 				10
#define BTN_FAIL_MAX		3
#define INTERVALO		250

//ESTADOS
enum fsm_state {
	START,
	PLAY,
	END
};

//VARIABLES GLOBALES
int ronda = -1;		//RONDA ACTUAL
int flag_start=0;	//FLAG BOTON START
int flags = 0;		//FLAG BOTONES JUGADOR 1
int flags2 = 0;		//FLAG BOTONES JUGADRO 2
int flag_timer=0;	//FLAG TIMER
int led = -1;		//LED ENCENDIDO
int boton1 = -1;	//BOTON PULSADO POR EL JUGADOR 1
int boton2 = -1;	//BOTON PULSADO POR EL JUGADOR 2
int fallos = 0;		//FALLOS DEL JUGADOR 1
int fallos2 = 0;	//FALLOS DEL JUGADOR 2

//VARIABLE JUGADOR QUE ACABA DE PULSAR(UNO O DOS DEPENDIENDO DEL JUGADOR)
int jugador=-1;

//FLAG CERROJOS, BLOQUEO DE INTERRUPCIONES DE PULSAR
int pulsar=1;
int pulsar2=1;
int last =0;
int last2 = 0;
int contador=0;


//VARIABLES POR DEFECTO
int timeout = 5000;
int penalty_time = 3000;
int decremento = 0;
int jugadores = 1;
int complejo=0;
int leds = 4;
int rondas = 10;

//Esto es lo que he cambiado para hacer el array dinamico, ponemos un puntero que se llama
//*tiempos y mas adelante en el main, despues
//de haber pedido el numero de rondas, creamos un array del
//tamaño que se busca y ahi apuntamos este puntero al comienzo de ese array
int *tiempos;
int start =0;
int t = 0;
int t2 = 0;
//SE INCREMENTE EN EL while(1) QUE HAY EN EL MAIN, SE USA PARA SABER EL TIMEOUT, VA INCREMENTANDOSE CADA VEZ QUE HACE UN SALTO
//DE EVENTOS Y CUANDO EN LAS CONDICIONES DE LOS EVENTOS EL RESTO DE ESTA VARIBALE Y LA VARIABLE GLOBAL DE TIMEOUT SEA 0 HA HABIDO TIMEOUT
int timeoutc_ms=0;

//METODOS
//CON SCANS PROCESA Y CONFIGURA LAS VARIBLES GLOBALES PARA NÚMERO DE JUGADORE, RONDAS, TIMEOUT, PENALTY_TIME, COMPLEJIDAD(DECREMENTO DEL TIMEOUT) Y NUMERO DE LEDS
void entrada();
//DECREMENTA EL TIMEOUT
void timeout_decre();
//HACE PARAPDEAR LOS LEDS SECUENCIALMENTE UNO DETRAS DE OTRO EN ORDEN HASTA QUE PARA EN EL LED ALEATORIO
void parpadea2();

//FLAGS
//FLAG TIMER
void timer_out (union sigval value) {flag_timer |= FLAG_TIMER; }
//FLAGS BOTONES
//SI SE PULSA ALGÚN BOTÓN SE LANZAN LOS SIGUIENTES METODOS
//COMPARA SI DESDE QUE SE PULSA UN BOTÓN Y VUELVE A DETECTAR UNA PULSACION, DEBIDO A UN REBOTE O PULSAR DOS A LA VEZ
//SI ESE INTERVALO NO ES SUPERIROR A LA VARIBALE INTERVALOR=250 ms NO SE METE. PARA LOS BOTONES DE JUEGO SOLO SE PUEDEN PULSAR
//SI SE ESTA EN JUEGO PARA EL BOTON START ES ALREVES, POR ESO SE COMPRUEBA SI ronda=-1 O DISTINTO, CUANDO ronda INICIALMENTE ES -1
//Respuesta a la interrupcion del boton START/END
void boton_start_end(void) {
	int now= millis();//ultima vez pulsado
	if(pulsar==1 && now-last>INTERVALO && ronda==-1){//comparar anterior pulsado
		last=now;//ultima vez pulsado
		pulsar=-1;
		pulsar2=-1;
		printf("\nBoton START\n\n");
		flag_start |= FLAG_BOTON_START_END;
	}
}
//GESTIÓN DE LAS INTERRUPCIONES DE LOS BOTONES DEL JUGADOR 1
void boton_1_p1(void) {
	int now= millis();//ultima vez pulsado
	if(pulsar==1 && ronda<=rondas && ronda!=-1 && now-last>INTERVALO){
		last=now;//ultima vez pulsado
		pulsar=-1;
		printf("\n\nBoton1 JUGADOR1\n\n");
		boton1=1;
		flags |= FLAG_BOTON;

	}
}
void boton_2_p1(void) {
	int now= millis();//tiempo ahora pulsado
	if(pulsar==1&& ronda<=rondas && ronda!=-1 && now-last>INTERVALO){
		last=now;//ultima vez pulsado
		pulsar=-1;
		printf("\n\nBoton2 JUGADOR1\n\n");
		boton1=2;
		flags |= FLAG_BOTON;

	}
}
void boton_3_p1(void) {
	int now= millis();//tiempo ahora pulsado
	if(pulsar==1&& ronda<=rondas && ronda!=-1 && now-last>INTERVALO){
		last=now;//ultima vez pulsado
		pulsar=-1;
		printf("\n\nBoton3 JUGADOR1\n\n");
		boton1=3;
		flags |= FLAG_BOTON;

	}
}
void boton_4_p1(void) {
	int now= millis();//ultima vez pulsado
	if(pulsar==1&& ronda<=rondas && ronda!=-1 && now-last>INTERVALO){
		last=now;//ultima vez pulsado
		pulsar=-1;
		printf("\n\nBoton4 JUGADOR1\n\n");
		boton1=4;
		flags |= FLAG_BOTON;

	}
}
//GESTIÓN DE LAS INTERRUPCIONES DE LOS BOTONES DEL JUGADOR 1
void boton_1_p2(void) {
	int now= millis();//ultima vez pulsado
	if(pulsar2==1 && ronda<=rondas && ronda!=-1 && now-last>INTERVALO){
		last=now;//ultima vez pulsado
		pulsar2=-1;
		printf("\n\nBoton1 JUGADOR2\n\n");
		boton2=1;
		flags2 |= FLAG_BOTON;
	}
}
void boton_2_p2(void) {
	int now= millis();//ultima vez pulsado
	if(pulsar2==1 && ronda<=rondas && ronda!=-1 && now-last>INTERVALO){
		last=now;//ultima vez pulsado
		pulsar2=-1;
		printf("\n\nBoton2 JUGADOR2\n\n");
		boton2=2;
		flags2 |= FLAG_BOTON;
	}
}
void boton_3_p2(void) {
	int now= millis();//ultima vez pulsado
	if(pulsar2==1 && ronda<=rondas && ronda!=-1 && now-last>INTERVALO){
		last=now;//ultima vez pulsado
		pulsar2=-1;
		printf("\n\nBoton3 JUGADOR2\n\n");
		boton2=3;
		flags2 |= FLAG_BOTON;
	}
}
void boton_4_p2(void) {
	int now= millis();//ultima vez pulsado
	if(pulsar2==1 && ronda<=rondas && ronda!=-1 && now-last>INTERVALO){
		last=now;//ultima vez pulsado
		pulsar2=-1;
		printf("\n\nBoton4 JUGADOR2\n\n");
		boton2=4;
		flags2 |= FLAG_BOTON;
	}
}

//1==true
//0==false

//EVENTOS
//EVENTO AL INICIAR UNA PARTIDA,DESDE EL ESTADO START INICIALIZA LA VARIBALE ronda=1 Y DEVUELVE 1 SI SE PULSA START Y SI NO 0, CAMBIA EL ESTADO A PLAY
int event_btn_start_end(fsm_t* this) {
	//printf("\nS");
	if(flag_start & FLAG_BOTON_START_END){
		flag_start=0;//NO SE METE EN event_btn_start_end
		printf("---------------------------\n");
		printf("EVENT_BTN_START_END\n\n");
		digitalWrite(GPIO_LED_START,LOW);//Se apaga el led start
		ronda=1;
		return 1;
	}
	return 0;
}
//EVENTO QUE SI CUMPLE LAS CONDICIONES DE BIEN PULSADO SE METE EN ÉL MODIFICA LAS VARIABLES QUE NECESITA play_ok, APAGA LEDS
//Y DEVUELVE 1 SI NO 0, DESPUÉS SE METE EN play_ok y continua en el estado PLAY
//CUMPLE LAS CONDICIONES DE BIEN PULSADO SI:
//->pulsa un boton
//->& no hay timeout
//->& esta dentro de ronda válida
//->& no ha llegado al máximo de fallos
//VARAIBLES QUE GUARDA PARA play_fail:
//->t : tiempo que tarda en pulsar el botón
//->jugador: jugador que pulsa el boton bien
//->pulsar : bloque las interrupciones de los botones del jugador 1
//PARA DOS JUGADORES
//->t2: tiempo que tarda en pulsar el botón
//->jugador : jugador que pulsa el botón bien
//->pulsar2 : bloquea las interrupciones de los botones del jugador 2
int event_btn_ok_player(fsm_t* this){
	//printf("O");
	if(jugadores==1){//Hay un jugador en la partida
		if(((flags & FLAG_BOTON) && (timeoutc_ms*10)%timeout) && ronda<=rondas && fallos<BTN_FAIL_MAX){
			pulsar=-1;
			jugador=1;
			if((boton1==led) && ((timeoutc_ms*10)%timeout)){
				printf("---------------------------\n");
				printf("EVENT_BTN_OK_PLAYER1\n");
				printf("start %i ms\n",start);
				printf("end %i ms\n",millis());
				t =(millis()-start);//tiempo desde que se enciende hasta que se pulsa el boton
				printf("Tiempo %i: %i ms\n",ronda,t);
				digitalWrite((led+21),LOW);
				printf("BIEN PULSADO JUGADOR 1\n\n");
				return 1;
			}
		}
	}
	else{//Si son dos jugadores
		if(((flags & FLAG_BOTON) && (timeoutc_ms*10)%timeout) && ronda<=rondas && fallos<BTN_FAIL_MAX){
			pulsar=-1;
			jugador=1;
			if((boton1==led) && ((timeoutc_ms*10)%timeout)){
				printf("---------------------------\n");
				printf("EVENT_BTN_OK_PLAYER1\n");
				printf("start %i ms\n",start);
				printf("end %i ms\n",millis());
				t =(millis()-start);//tiempo desde que se enciende hasta que se pulsa el boton
				printf("Tiempo %i: %i ms\n",ronda,t);
				digitalWrite((led+21),LOW);
				digitalWrite((led),LOW);
				printf("BIEN PULSADO JUGADOR 1\n\n");
				return 1;
			}
		}
		if(((flags2 & FLAG_BOTON) && (timeoutc_ms*10)%timeout)&& ronda<=rondas && fallos2<BTN_FAIL_MAX){
			pulsar2=-1;
			jugador=2;
			if((boton2==led) && ((timeoutc_ms*10)%timeout)){
				printf("---------------------------\n");
				printf("EVENT_BTN_OK_PLAYER2\n");
				printf("start %i ms\n",start);
				printf("end %i ms\n",millis());
				t2 =(millis()-start);//tiempo desde que se enciende hasta que se pulsa el boton
				printf("Tiempo %i: %i ms\n",ronda,t2);
				digitalWrite((led),LOW);
				digitalWrite((led+21),LOW);
				printf("BIEN PULSADO JUGADOR 2\n\n");
				return 1;
			}
		}
	}return 0;
}
//EVENTO QUE SI CUMPLE LAS CONDICIONES DE MAL PULSADO SE METE EN ÉL MODIFICA LAS VARIABLES QUE NECESITA play_fail, APAGA LEDS
//Y DEVUELVE 1 SI NO 0, DESPUÉS SE METE EN play_fail y continua en el estado PLAY
//CUMPLE LAS CONDICIONES DE MAL PULSADO SI:
//->pulsa un boton Ó hay timeout
//->Y esta dentro de ronda válida
//->Y no ha llegado al máximo de fallos
//VARAIBLES QUE GUARDA PARA play_fail:
//->t : tiempo que tarda en pulsar el botón
//->jugador: jugador que pulsa el boton bien
//->pulsar : bloque las interrupciones de los botones del jugador 1
//PARA DOS JUGADORES
//->t2: tiempo que tarda en pulsar el botón
//->jugador : jugador que pulsa el botón bien
//->pulsar2 : bloquea las interrupciones de los botones del jugador 2
int event_btn_fail_player(fsm_t* this){
	//printf("F");
	if(jugadores==1){
		if(((flags & FLAG_BOTON) || !((timeoutc_ms*10)%timeout)) && ronda<=rondas && fallos<BTN_FAIL_MAX){
			pulsar=-1;
			jugador=1;
			printf("---------------------------\n");
			printf("EVENT_BTN_FAIL_PLAYER\n");
			t =(millis()-start);
			printf("Tiempo %i: %i ms\n",ronda,t);
			if(!((timeoutc_ms*10)%timeout)){
				digitalWrite((led+21),LOW);
				printf("DEMASIADO LENTO JUGADOR 1\n\n");
				return 1;
			}
			else{if(boton1!=led){
				printf("MAL PULSADO JUGADOR 1\n\n");
				digitalWrite((led+21),LOW);
				return 1;
			}
			}
		}
	}
	else{//Dos jugadores
		//TIMEOUT
		if(!((timeoutc_ms*10)%timeout) && ronda<=rondas && (fallos<BTN_FAIL_MAX || fallos2<BTN_FAIL_MAX)){
			digitalWrite((led+21),LOW);
			digitalWrite((led),LOW);
			//Pulso mal el jugador anterior
			if(jugador==2){
				contador=2;//Contador cuando falla el 1 por timeout después de que haya fallado al pulsar el jugador 2
			}
			if(jugador==1){
				contador=3;//Contador cuando falla el 2 por timeout después de que haya fallado al pulsar el jugador 1
			}
			jugador=0;
			pulsar=-1;
			pulsar2=-1;
			printf("---------------------------\n");
			printf("EVENT_BTN_FAIL_PLAYER\n");
			t =(millis()-start);//tiempo desde que se enciende hasta que se pulsa el boton
			printf("Tiempo %i: %i ms\n",ronda,t);
			printf("DEMASIADO LENTO JUGADORES, contador %i \n",contador);
			return 1;
		}
		else{
			if((flags & FLAG_BOTON) && ronda<=rondas && fallos<BTN_FAIL_MAX){
				pulsar=-1;//No se puede pulsar boton
				jugador=1;
				printf("\n---------------------------\n");
				printf("EVENT_BTN_FAIL_PLAYER\n");
				t =(millis()-start);//tiempo desde que se enciende hasta que se pulsa el boton
				printf("Tiempo %i: %i ms\n",ronda,t);
				if(boton1!=led){
					printf("MAL PULSADO JUGADOR 1\n\n");
					digitalWrite((led+21),LOW);
					return 1;
				}

			}
			if((flags2 & FLAG_BOTON) && ronda<=rondas && fallos2<BTN_FAIL_MAX){
				pulsar2=-1;
				jugador=2;
				printf("\n---------------------------\n");
				printf("EVENT_BTN_FAIL_PLAYER\n");
				t2 =(millis()-start);//tiempo desde que se enciende hasta que se pulsa el boton
				printf("Tiempo %i: %i ms\n",ronda,t2);
				if(boton2!=led){
					printf("MAL PULSADO JUGADOR 2\n\n");
					digitalWrite((led),LOW);
					return 1;
				}
			}
		}
	}
	return 0;
}
//EVENTO QUE DEVUELVE 1 CUANDO SE SUPERA EL NUMERO DE RONDAS O SE LLEGA AL NUMERO TOTAL DE FALLOS CAMBIA EL ESTADO A END
int event_end_game(fsm_t* this) {
	//printf("E\n");
	if(ronda>=(rondas+1) || (fallos>=BTN_FAIL_MAX)){
		digitalWrite((led+21),LOW);
		printf("\n---------------------------\n");
		printf("EVENT_END_GAME\n");

		if(fallos>=BTN_FAIL_MAX ){
			printf("El jugador 1 ha fallado mas de tres veces\n\n");
			return 1;
		}
		else{
			printf("Se ha completado el numero de rondas\n\n");
			return 1;
		}
	}
	if(ronda>=(rondas+1) || (fallos2>=BTN_FAIL_MAX)){
		digitalWrite((led),LOW);
		printf("\n---------------------------\n");
		printf("EVENT_END_GAME\n");

		if(fallos2>=BTN_FAIL_MAX){
			printf("El jugador 2 ha fallado mas de tres veces\n\n");
			return 1;
		}
		else{
			printf("Se ha completado el numero de rondas\n\n");
			return 1;
		}
	}
	return 0;
}

//ACCIONES
//ACCION DE INICIO DE JUEGO INICIA LOS FLAGS A CERO DE PULSACIONES GENERA UN LED ALEATORIO, LANZA EL METODO DE
//PARAPDEAR, CAMBIA LA VARIABLE DE INICIO DE TIEMPO start Y LIBERA BLOQUEO DE LOS BOTONES
void start_play(fsm_t* this) {
	//PRIMERA VEZ DETECTAR Y NO GENERAR OTRO LED, SOLO APAGARLO Y GUARDAR EL TIEMPO
	printf("START_PLAY\n");
	flags=0;//No se pueden pulsar botones JUGADOR 1
	flags2=0;//No se pueden pulsar botones JUGADOR 2
	printf("Empieza ronda %i\n",ronda);
	//Empezar ronda 1
	led = 1 + rand() % (leds);//Generar numero aleatorio
	//NUEVO
	pulsar=-1;
	if(jugadores==2){
		pulsar2=-1;
	}
	parpadea2();
	pulsar=1;
	if(jugadores==2){
		pulsar2=1;
	}
	//NUEVO
	printf("ENCENDER LED:%i\n",led);
	//COMENTADO POR PROBAR
	//	digitalWrite((led+21),HIGH);//Enciendo el led aleatorio
	//	if(jugadores==2){
	//		digitalWrite((led),HIGH);//Enciendo el led aleatorio jugador 2
	//	}
	start=millis();//INICIO RELOG
	printf("Tiempo inicio: %i ms\n",start);
	printf("---------------------------\n\n");
	pulsar=1;
	if(jugadores==2){
		pulsar2=1;
	}
	timeoutc_ms=0;
}
//ACCION DE BOTON BIEN PULSADO INICIA LOS FLAGS A CERO DE PULSACIONES, GUARDA EL TIEMPO DE PULSACION DEL JUGADOR MAS RAPIDO
//Y AL OTRO LE PONE EL SUYO MAS LA PENALIZACION GENERA UN LED ALEATORIO, LANZA EL METODO DE PARAPDEAR, CAMBIA
//LA VARIABLE DE INICIO DE TIEMPO start,DECREMENTA EL timeout SI EL MODO COMPLEJO ESTA ACTIVADO
//Y LIBERA BLOQUEO DE LOS BOTONES
void play_ok(fsm_t* this) {
	printf("PLAY_OK\n");
	flags=0;//No se pueden pulsar botones JUGADOR 1
	flags2=0;//No se pueden pulsar botones JUGADOR 2
	if(jugador==1){//Si pulsa el jugador 1 antes
		//Guarda t en la memoria de jugador 1 por no haber pulsado antes que el jugador 2
		printf("Guardar tiempo del jugador 1\n");
		*(tiempos+ronda-1)= t;
		//Guarda t + penalty_time en la memoria de jugador 2 por no haber pulsado antes que el jugador 1
		if(jugadores==2){
			if(contador==0){//Le añade el tiempo del 1 mas penalty si antes el no habia pulsado mal
				printf("Guardar tiempo del jugador 1+penalty para el 2\n");
				*(tiempos+rondas-1+ronda)=t+penalty_time;
			}//Si no esque anteriormente el jugador 2 fallo
		}
	}
	if(jugador==2){//SI PULSA EL JUGADOR 2 ANTES
		printf("Guardar tiempo del jugador 2\n");
		//Guarda t2 en la memoria de jugador 2 por no haber pulsado antes que el jugador 1
		*(tiempos+rondas-1+ronda)=t2;
		//Guarda t2 + penalty_time en la memoria de jugador 1 por no haber pulsado antes que el jugador 2
		if(jugadores==2){//Le añade el tiempo del 1 mas penalty si antes el no habia pulsado mal
			if(contador==0){
				printf("Guardar tiempo del jugador 2+penalty para el 1\n");
				*(tiempos+ronda-1)= t2 + penalty_time;
			}//Si no esque anteriormente el jugador 1 fallo
		}
	}
	ronda++;//Incrementa ronda para pasar a la siguiente
	if(ronda<=rondas){//ronda es menor o igual que el menor de rondas
		//Empieza otra ronda
		printf("Empieza ronda %i\n",ronda);
		//Led aleatorio
		led = 1 + rand() % (leds);//Generar numero aleatorio
		//NUEVO
		pulsar=-1;
		if(jugadores==2){
			pulsar2=-1;
		}
		parpadea2();
		pulsar=1;
		if(jugadores==2){
			pulsar2=1;
		}
		//NUEVO
		printf("ENCENDER LED:%i\n",led);
		//COMENTADO POR PROBAR
		/*digitalWrite((led+21),HIGH);//Enciendo el led aleatorio
		if(jugadores==2){
			digitalWrite((led),HIGH);//Enciendo el led aleatorio
		}*/
		if(complejo==1){
			timeout_decre();
		}
		start=millis();//inicio reloj
		printf("Tiempo inicio: %i ms\n",start);
		printf("---------------------------\n\n");
	}
	timeoutc_ms=0;//Reinicializa el timeout
	pulsar=1;//Abre el cerrojo para que se puedan pulsar botones jugador 1
	pulsar2=1;//Abre el cerrojo para que se puedan pulsar botones jugador 2
	contador=0;
	jugador=-1;//PONER A -1 PARA QUE SI HACIERTA REINICIAR LA VARIBALE
	//EXCEPTO EN play_fail QUE SE NECESITA SABER QUE JUGADOR FALLO ANTES PORQUE SI EL PROXIMO HACE UN TIMEOUT SABER A
	//QUIEN SE LE AÑADE SOLO EL TIEMPO DE timeout+penalty_time Y AL PRIMERO SU TIEMPO MAS EL penalty_time POR FALLAR
	//	printf("final play_ok\n");
}
//ACCION DE BOTON BIEN PULSADO INICIA LOS FLAGS A CERO DE PULSACIONES, GUARDA EL TIEMPO DE PULSACION DEL JUGADOR QUE FALLO MAS LA PENALIZACION
//Y TERMINA HASTA QUE EL OTRO PULSE GENERA UN LED ALEATORIO, LANZA EL METODO DE PARAPDEAR, CAMBIA
//LA VARIABLE DE INICIO DE TIEMPO start,DECREMENTA EL timeout SI EL MODO COMPLEJO ESTA ACTIVADO
//Y LIBERA BLOQUEO DE LOS BOTONES
void play_fail(fsm_t* this) {
	printf("PLAY_FAIL\n");
	flags=0;//No se pueden pulsar botones JUGADOR 1
	flags2=0;//No se pueden pulsar botones JUGADOR 2
	if(contador<1&&jugadores==2){//La primera vez y dos jugadores
		printf("contador = %i\n",contador);
		printf("Dentro del primer if\n");
		if(jugador==0){//Cuando jugador es 0 significa que ha habido un timeout antes de que otro fallase y se penaliza a los
			//dos obteniendo el tiempo de penalización mas el timeout, y iniciando otra ronda
			printf("NINGUNO HA CONSEGUIDO DARLE ANTES DE TIEMPO\n");
			//LA PRIMERA VEZ SI HAY TIMEOUT LOS DOS FALLAN
			*(tiempos+ronda-1)=timeout+penalty_time;
			fallos++;
			*(tiempos+rondas + ronda-1)= timeout+ penalty_time;
			fallos2++;
			if(complejo==1){
				timeout_decre();
			}
			ronda++;
			contador=0;//Reinicia contador
			//pulsar = 1;COMENTADO POR PROBAR
			if((fallos < BTN_FAIL_MAX ||fallos2 <BTN_FAIL_MAX) && ronda<=rondas){
				printf("Empieza ronda %i\n",ronda);
				//Empieza otra ronda
				led = 1 + rand() % (leds);//Generar numero aleatorio
				//NUEVA
				pulsar=-1;
				if(jugadores==2){
					pulsar2=-1;
				}
				parpadea2();
				pulsar=1;
				if(jugadores==2){
					pulsar2=1;
				}
				//NUEVA
				printf("ENCENDER LED:%i\n",led);
				start=millis();//inicio reloj
				printf("Tiempo inicio: %i ms\n",start);
				printf("---------------------------\n\n");
			}
		}
		if(jugador==1){
			//JUGADOR 1
			printf("HA FALLADO EL JUGADOR 1 LA PRIMERA VEZ\n");
			// no incrementa ronda
			//Se guarda el tiempo que se tardo en cometer fallo y se añade penalizacion
			*(tiempos+ronda-1)=t+penalty_time;
			// tiempo + penalty
			contador++;//Incrementa el contador para meterse despues en el segundo fallo si lo hay
			fallos++;
			printf("fallos %i",fallos);
			pulsar = -1;
			pulsar2 = 1;
			//jugador=0;//COMENTADO POR PROBRAR
		}
		if(jugador==2){
			//JUGADOR 2
			printf("HA FALLADO EL JUGADOR 2 LA PRIMERA VEZ\n");
			// No incrementa ronda
			// tiempo2 + penalty
			*(tiempos+rondas + ronda-1)= t2+ penalty_time;
			contador++;//Incrementa el contador para meterse despues en el segundo fallo si lo hay
			fallos2++;
			pulsar = 1;//Se admite pulsar al jugador uno
			pulsar2 = -1;//Se niega pulsar al jugador dos
			//jugador=0;//COMENTADO POR PROBAR
		}
	}
	else{//La segunda vez que falle el otro jugador O TIMEOUT DEL SEGUNDO DESPUES DE QUE EL PRIMERO FALLASE
		if(jugador==0){//Cuando jugador es 0 significa que ha habido un timeout
			if(contador==2){
				printf("HAS SIDO DEMASIADO LENTO JUGADOR 2\n");
				*(tiempos+ronda-1)=timeout+penalty_time;
				fallos++;
				printf("fallos 2 %i",fallos);
			}
			if(contador==3){
				printf("HAS SIDO DEMASIADO LENTO JUGADOR 1\n");
				*(tiempos+ 2*ronda-1)= timeout+penalty_time;
				fallos2++;
				printf("fallos %i",fallos2);
			}
		}
		if(jugador==1){
			printf("HA FALLADO EL JUGADOR 1 LA SEGUNDA VEZ\n");
			*(tiempos+ronda-1)=t+penalty_time;
			fallos++;
		}
		if(jugador==2){
			printf("HA FALLADO EL JUGADOR 2 LA SEGUNDA VEZ\n");
			*(tiempos+rondas + ronda-1)= t2+ penalty_time;
			fallos2++;
		}

		if(complejo==1){
			timeout_decre();
		}
		ronda++;
		contador=0;//Reinicia contador
		//pulsar = 1;COMENTADO POR PROBAR
		if((fallos < BTN_FAIL_MAX ||fallos2 <BTN_FAIL_MAX) && ronda<=rondas){
			printf("Empieza ronda %i\n",ronda);
			//Empieza otra ronda
			led = 1 + rand() % (leds);//Generar numero aleatorio
			//NUEVA
			pulsar=-1;
			if(jugadores==2){
				pulsar2=-1;
			}
			parpadea2();
			pulsar=1;
			if(jugadores==2){
				pulsar2=1;
			}
			//NUEVA
			printf("ENCENDER LED:%i\n",led);
			start=millis();//inicio reloj
			printf("Tiempo inicio: %i ms\n",start);
			printf("---------------------------\n\n");
		}
	}
	timeoutc_ms=0;//Reinicia el timeout al final del play_fail
}
//ACCION QUE SE CAMBIA AL ESTADO END SI SE CUMPLE LA CONDICION EN EL EVENTO IMPRIME LOS TIEMPOS OBTENIDOS EN LAS RONDAS JUGADAS PARA UNO O DOS JUGADORES
void play_end(fsm_t* this) {
	printf("---------------------------\n");
	printf("PLAY_END\n");
	flags=0;
	flags2=0;
	digitalWrite(led+21,LOW);
	digitalWrite(led,LOW);
	//Calcular estadisticas y encender led de start_end
	if(jugadores==1){
		printf("ESTADÍSTICAS\n");
		int i;
		int t_max=0;
		int t_min=*tiempos;
		int t_medio=0;
		ronda--;//DECREMENTAR RONDA PARA PONER EL VALOR IGUAL A NUMERO TOTAL DE RONDAS, YA QUE ACABA CON UN VALOR MAS PORQUE
		//SE INCREMENTA ANTES DE INICIAR LA SIGUIENTE EN LOS MÉTODOS play_ok y play_fail DONDE ANTES DE ENTRAR COMPRUEBA
		//SI LA RONDA INCREMENTADA (LA SIGUIENTE) ES IGUAL O MENOR AL NÚMERO TOTAL DE RONDAS
		for(i=0;i<ronda;i++){
			printf("Tiempo %i:%i\n",i+1,*(tiempos+i));
			if(*(tiempos+i)>t_max){
				//				printf("eureka1\n");
				t_max = *(tiempos+i);
			}
			if(*(tiempos+i)<t_min){
				//				printf("eureka2\n");
				t_min = *(tiempos+i);
			}
			t_medio+=*(tiempos+i);
			//			printf("eureka3\n");
		}

		timeout +=decremento*rondas;
		t_medio/=ronda;
		//		printf("eureka4\n");

		//ESTADÍSTICAS
		printf("El tiempo mas rapido ha sido: %i s\n",t_min);
		printf("El tiempo mas lento ha sido %i s\n",t_max);
		printf("El tiempo de media ha sido %i s\n",t_medio);
		printf("El numero de fallos es %i\n\n\n",fallos);
		pulsar=1;
	}
	else {
		printf("ESTADISTICAS DOS JUGADORES\n");
		int i;
		int t_max1=0;
		int t_max2=0;
		int t_min1=*tiempos;
		int t_min2=*(tiempos+rondas);
		int t_medio1=0;
		int t_medio2=0;
		ronda--;//DECREMENTMOS RONDA PORQUE ACABA CON VALOR
		for(i=0;i<ronda;i++){
			printf("Tiempo del jugador 1 %i:%i\n",i+1,*(tiempos+i));
			printf("Tiempo del jugador 2 %i:%i\n",i+1,*(tiempos+rondas+i));
			if(*(tiempos+i)>t_max1){
				//				printf("eureka11\n");
				t_max1=*(tiempos+i);
			}
			if(*(tiempos+rondas+i)>t_max2){
				//				printf("eureka12\n");
				t_max2=*(tiempos+rondas+i);
			}
			if(*(tiempos+i)<t_min1){
				//				printf("eureka21\n");
				t_min1 = *(tiempos+i);
			}
			if(*(tiempos+rondas+i)<t_min2){
				//				printf("eureka22\n");
				t_min2 = *(tiempos+rondas+i);
			}
			t_medio1=*(tiempos+i)+t_medio1;
			//			printf("eureka31\n");
			t_medio2+=*(tiempos+rondas+i);
			//			printf("eureka32\n");
		}
		t_medio1/=ronda;
		t_medio2/=ronda;
		//		printf("eureka4\n");

		//ESTADISTICAS
		printf("ESTADÃƒï¿½STICAS JUGADOR 1\n");
		printf("El tiempo más rápido del jugador 1 ha sido %i ms\n",t_min1);
		printf("El tiempo más lento del jugador 1 ha sido %i ms\n",t_max1);
		printf("El tiempo de media del jugador 1 ha sido %i ms\n",t_medio1);
		printf("El número de fallos del jugador 1 ha sido %i\n", fallos);
		printf("ESTADÍSTICAS JUGADOR 2\n");
		printf("El tiempo más rápido del jugador 2 ha sido %i ms\n",t_min2);
		printf("El tiempo más lento del jugador 2 ha sido %i ms\n",t_max2);
		printf("El tiempo de media del jugador 2 ha sido %i ms\n",t_medio2);
		printf("El número de fallos del jugador 2 ha sido %i\n\n\n", fallos2);

		pulsar=1;
		pulsar2=1;
	}
	timeoutc_ms=0;
	ronda=-1;
}
//ACCION QUE SE LANZA SI SE PULSA EL BOTON START ESTANDO EN EL EVENTO end REINICIALIZA LAS VARIABLES AL ESTADO INICIAL
void end_start(fsm_t* this) {
	printf("END_START\n");

	digitalWrite(GPIO_LED_START,HIGH);

	int i;
	for(i=0;i<rondas;i++){
		*(tiempos+i)=0;
	}
	if(jugadores==2){
		for(i=0;i<rondas;i++){
			*(tiempos+rondas+i)=0;
		}
		fallos2=0;
		t2=0;
		flags2=0;
		boton2=-1;
		pulsar2=1;
		contador=0;
	}
	flags = 0;
	led=-1;
	boton1=-1;
	fallos=0;
	start=0;
	t=0;
	if(complejo==1){
		timeout_decre();
	}
	ronda = -1;
	pulsar=1;
	printf("---------------------------\n\n");
}

// wait until next_activation (absolute time)
void delay_until (unsigned int next)
{
	unsigned int now = millis();
	if (next > now) {
		delay (next - now);
	}
}

int main ()
{
	srand(time(NULL));
	tmr_t* tmr = tmr_new (timer_out);
	pulsar=-1;
	pulsar2=-1;

	// Máquina de estados: lista de transiciones
	// {EstadoOrigen, CondicionDeDisparo, EstadoFinal, AccionesSiTransicion }
	fsm_trans_t quiz_tmr[] = {
			{ START, event_btn_start_end, PLAY, start_play },
			{ PLAY, event_btn_ok_player, PLAY, play_ok },
			{ PLAY, event_btn_fail_player, PLAY, play_fail },
			{ PLAY, event_end_game, END, play_end },
			{ END,event_btn_start_end, START,end_start },
			{-1, NULL, -1, NULL },
	};

	fsm_t* quiz_tmr_fsm = fsm_new (START, quiz_tmr, tmr);
	unsigned int next;

	//INICIAMOS LOS PINES COMO GPIO
	wiringPiSetupGpio();

	//ALIMENTACION
	//CODIFICAMOS EL PIN DE ALIMENTACION COMO SALIDA Y A NIVEL ALTO
	pinMode (GPIO_ALIMENTACION,	 OUTPUT);
	digitalWrite(GPIO_ALIMENTACION, HIGH);
	//ENTRADAS
	//CODIFICAMOS ENTRADA START/END
	pinMode(GPIO_BUTTON_START_END, INPUT);
	//CODIFICAMOS ENTRADAS DE LOS BOTONES JUGADOR 1
	pinMode(GPIO_BUTTON_1, INPUT);
	pinMode(GPIO_BUTTON_2, INPUT);
	pinMode(GPIO_BUTTON_3, INPUT);
	pinMode(GPIO_BUTTON_4, INPUT);

	//SALIDAS
	//CODIFICAMOS SALIDA LED START/END
	pinMode(GPIO_LED_START, OUTPUT);
	//CODIFICAMOS SALIDA DE LOS LEDS JUGADOR 1
	pinMode(GPIO_LED_1, OUTPUT);
	pinMode(GPIO_LED_2, OUTPUT);
	pinMode(GPIO_LED_3, OUTPUT);
	pinMode(GPIO_LED_4, OUTPUT);
	//CODIFICAMOS SALIDA DE LOS LEDS JUGADOR 2
	pinMode(GPIO_LED_5, OUTPUT);
	pinMode(GPIO_LED_6, OUTPUT);
	pinMode(GPIO_LED_7, OUTPUT);
	pinMode(GPIO_LED_8, OUTPUT);
	//APAGAMOS LEDS JUGADOR 1
	digitalWrite(GPIO_LED_1,LOW);
	digitalWrite(GPIO_LED_2,LOW);
	digitalWrite(GPIO_LED_3,LOW);
	digitalWrite(GPIO_LED_4,LOW);
	//APAGAMOS LEDS JUGADOR 2
	digitalWrite(GPIO_LED_5,LOW);
	digitalWrite(GPIO_LED_6,LOW);
	digitalWrite(GPIO_LED_7,LOW);
	digitalWrite(GPIO_LED_8,LOW);
	//CONTROL PULSACION DE BOTONES
	//INTERRUPCION DEL BOTON START/END
	wiringPiISR(GPIO_BUTTON_START_END, INT_EDGE_RISING, boton_start_end);
	//INTERRUPCION DE LOS BOTONES DEL JUGADOR 1
	wiringPiISR(GPIO_BUTTON_1, INT_EDGE_RISING, boton_1_p1);
	wiringPiISR(GPIO_BUTTON_2, INT_EDGE_RISING, boton_2_p1);
	wiringPiISR(GPIO_BUTTON_3, INT_EDGE_RISING, boton_3_p1);
	wiringPiISR(GPIO_BUTTON_4, INT_EDGE_RISING, boton_4_p1);
	next = millis();
	/*Esta es la llamada al usuario para pedir los valores*/
	entrada();
	//INICIO DE ENCENDIDO
	//ENCENDEMOS LED START/END
	digitalWrite(GPIO_LED_START,HIGH);
	pulsar=1;
	pulsar2=1;
	printf("---------------------------\n");
	printf("Despues entrada\n");
	//CONSTRUIMOS EL ARRAY DINAMICO
	/*Aqui esta la construccion del array que te he dicho antes, arriba esta ya el metodo de entrada(), donde hemos pedido los valores al usuario
	y ya creamos el array como te he puesto arriba en las variables*/
	int tiempo[jugadores*rondas];
	int i;
	for(i =0;i<jugadores*rondas;i++){
		tiempo[i]=0;
	}
	//	printf("Crea array\n");
	tiempos =&tiempo[0];
	if(complejo==1){
		printf("Complejo 1");
		decremento = (timeout - 500)/rondas;
	}
	printf("Antes del if\n");
	//COMIENZA
	if(2==jugadores){
		printf("Dos jugadores\n");
		//CODIFICAMOS ENTRADAS DE LOS BOTONES JUGADOR 2
		pinMode(GPIO_BUTTON_5, INPUT);
		pinMode(GPIO_BUTTON_6, INPUT);
		pinMode(GPIO_BUTTON_7, INPUT);
		pinMode(GPIO_BUTTON_8, INPUT);
		//INTERRUPCION DE LOS BOTONES DEL JUGADOR 2
		wiringPiISR(GPIO_BUTTON_5, INT_EDGE_RISING, boton_1_p2);
		wiringPiISR(GPIO_BUTTON_6, INT_EDGE_RISING, boton_2_p2);
		wiringPiISR(GPIO_BUTTON_7, INT_EDGE_RISING, boton_3_p2);
		wiringPiISR(GPIO_BUTTON_8, INT_EDGE_RISING, boton_4_p2);
	}
	printf("Moore\n");
	while (1) {
		fsm_fire (quiz_tmr_fsm);
		timeoutc_ms++;
		next += CLK_MS;
		delay_until (next);
	}

	tmr_destroy ((tmr_t*)(quiz_tmr_fsm->user_data));
	fsm_destroy (quiz_tmr_fsm);
}

//ENTRADA DE VALORES
/*Este es el metodo que va a pedir valores al usuario, este lo he colocado antes de cuando se inicializa la maquina de estados
aqui va pidiendo valores al usuario con printf y scanf, he intentado poner el maximo numero de protecciones que se me ha ocurrido, quizas habria que buscar
lo tipico de si escribe un char en vez de un int estalla el programa, o lo que hace es poner el codigo ascii*/
void entrada(){
	printf("Bienvenido!! \nVas a jugar al increible juego de Victor y Miguel.\nPero antes necesitamos saber una serie de cosas.\nPor favor escribe por teclado tu respuesta y\npulsa enter con tu respuesta\n");
	printf("¿Cuantos jugadores sois?\n");
	scanf("%i", &jugadores);
	printf("jugadores %d\n",jugadores);
	if(jugadores!= 1 && jugadores!=2){
		printf("Solo pueden ser 1 o 2 jugadores, vuelvelo a intentar\n");
		scanf("%i", &jugadores);
		printf("Jugadores: %d \n",jugadores);

		if(jugadores<1 || jugadores>2){
			printf("Pues vas a tener que jugar tu solo...\n");
			jugadores=1;
			printf("Jugadores: %d",jugadores);
		}
	}
	if(jugadores==2){
		printf("¿Cuantas rondas vais a jugar?\n");
	}else{
		printf("¿Cuantas rondas vas a jugar?\n");
	}
	scanf("%i", &rondas);
	printf("rondas %d\n",rondas);
	if(rondas<10){
		printf("No no no, el minimo son 10 partidas\n");
		rondas = 10;
	}else if(rondas>30){
		printf("Uuuff muchas rondas quieres jugar, el limite son 30\n");
		rondas = 30;
	}
	printf("¿Cuánto tiempo de penalización quieres?\n");
	scanf("%i", &penalty_time);
	//MULTIPICAR PARA PONERLO EN MILISEGUNDOS
	penalty_time*=1000;
	if(penalty_time<1000){
		printf("Estás seguro, prueba otra vez\n");
		scanf("%i",&penalty_time);
		penalty_time*=1000;
	}
	if(penalty_time<3000){
		printf("Tramposillo, no se puede tener menos de 3 segundos de penalización\n");
		penalty_time = 3000;
	}
	printf("¿Cuanto tiempo de timeout quieres?\n");
	scanf("%i",&timeout);
	timeout*=1000;
	if(timeout<0){
		printf("Desde ¿Cuando existe el tiempo negativo?, te pondremos el facilillo, 5 segundos\n");
		timeout=5000;
	}else if(timeout>5000){
		printf("Uuu!! ¿Que pasa? ¿Que tienes dedos como morcillas?\n");
	}else if(timeout>3000){
		printf("Bien Bien!! dándole más complejidad\n");
	}else if(timeout<1000){
		printf("Uuu!! nivel demasiado extremo, el mínimo es de un segundo\n");
		timeout = 1000;
	}
	printf("¿Quieres más complejidad?\n");
	printf("¿Quieres que vayamos reduciendo el timeout?\n");
	printf("Escribe un 1 si quieres y un 0 si no quieres\n");
	scanf("%i",&complejo);
	if(complejo!=1 && complejo !=0){
		printf("¿Te has equivocado?\n");
		scanf("%i", &complejo);
		if(complejo!=1 || complejo!=0){
			printf("Vaya veo que quieres destrozar el juego, te pondremos que sí\n");
			complejo = 1;
		}
	}
	if(complejo == 1) {
		decremento = (timeout -500)/rondas;

	}
	printf("¿Con cuantos leds quieres jugar?\n");
	scanf("%i", &leds);
	if(leds<0){
		printf("Lo siento no puedes jugar con leds inexistentes\n");
		printf("Te pondremos cuatro\n");
		leds=4;
	}else if(leds>4){
		printf("El maximo numero de leds es: 4\n");
		printf("Se jugara con 4 leds por participante\n");
		leds = 4;
	}
	printf("EMPIEZA EL JUEGO\n");
}
void timeout_decre(){
	timeout-= decremento;
}

void parpadea2(){
	int i;
	int num_led=1;
	for(i=0;i<(leds*2-1)+led;i++){//poner de 0-7 son 8 leds, multiplo de 4 para que acabe en el led aleatorio
		digitalWrite(num_led+21,1);
		if(jugadores==2){
			digitalWrite(num_led,1);
		}
		delay(100);
		digitalWrite(num_led+21,0);
		if(jugadores==2){
			digitalWrite(num_led,0);
		}
		num_led++;
		if(num_led==(leds+1)){
			num_led=1;
		}
	}
	digitalWrite(num_led+21,1);
	if(jugadores==2){
		digitalWrite(num_led,1);
	}
}
