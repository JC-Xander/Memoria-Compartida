#include <string>
/*
 * Manejo de cadenas de caracteres
 */ // ---------- ---------- ---------- ---------- ----------
#include<cstring>

// ----- LIBRERIAS PARA IMPRESIÓN EN CONSOLA -----
#include <iostream>
/*
 * - cout << : Para impreción en consola
 */ // ---------- ---------- ---------- ---------- ----------
#include <cstdio>
/*
 * - perror() : impreción de errores en conjunto de: */
#include <errno.h>
// ---------- ---------- ---------- ---------- ----------

// ------- LIBRERIAS DE MEMORIA COMPARTIDA -------
#include <sys/shm.h>
/*
 * - ftok () Crea una @Clave
 * - shmget ( | IPC_CREAT) : Reserva el espacio en memoria 
 *   y devuelve el ID
 * - (int *)shmat (Id_Memoria, (char *)0, 0) : Crear el mapeado
 * 
 * 
 * - shmdt() : Desadjuntar el segmento de memoria
 *      compartida del espacio de direcciones del proceso.
 * - shmctl() : Libera la memoria compartida.
 */ // ---------- ---------- ---------- ---------- ----------

#include<stdlib.h>
/*
 * - exit()
 */ // ---------- ---------- ---------- ---------- ----------

#include<unistd.h>
/*
 * - fork()
 */ // ---------- ---------- ---------- ---------- ----------

// ------ CONTEO DE LOS PROCESOS HIJOS ------
#include<sys/wait.h>
/*
  - wait()
*/ // ---------- ---------- ---------- ---------- -----------
using namespace std;

#define TOTAL_PROCESOS 5
const int MEMORIA_TOTAL = TOTAL_PROCESOS;
const int TAMANO_CADENA = 50;


int main() {
    key_t claveMemoria;
    int idMemoria;
    char* datosDeMemoria = nullptr;

    claveMemoria = ftok("/bin/ls", 33);
    if (claveMemoria == -1) {
        perror("No se consiguió clave para memoria compartida");
        exit(1);
    }

    idMemoria = shmget(claveMemoria, TAMANO_CADENA * MEMORIA_TOTAL, 0777 | IPC_CREAT);
    if (idMemoria == -1) {
        perror("No se pudo obtener el Id de la memoria");
        exit(1);
    }

    datosDeMemoria = (char*)shmat(idMemoria, nullptr, 0);
    if (datosDeMemoria == (char*)-1) {
        perror("No se pudo adjuntar la memoria compartida");
        exit(1);
    }

    for (int i = 0; i < TOTAL_PROCESOS; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("No se pudo crear el nuevo proceso");
            exit(1);
        }

        if (pid == 0) {
            // Proceso hijo
            cout << "Hola soy el proceso: " << i << " ; PID: " << getpid() << endl;
            sleep(1);
            string cadena = "Estoy alamcenando la memoria compartida " + to_string(getpid());
            sleep(2);
            strncpy(datosDeMemoria + (i * TAMANO_CADENA), cadena.c_str(), TAMANO_CADENA - 1);
            cout << "Finaliza el proceso hijo -> " << i << endl;
            exit(0);
            // Los hijos no pasan de este linea por el exit por lo tanto solo el proceso padre
            // es capaz de continuar con el flujo
        }
    }

    int status;
    cout << "-- Proceso padre esperando a que terminen los hijo para continuar --" << endl;
    for (int i = 0; i < TOTAL_PROCESOS; i++) {
        pid_t hijoPID = wait(&status);

        if (status == -1)
            perror("Error en la espera de finalización del hijo");
        else
            cout << "Proceso hijo " << hijoPID << " terminó." << endl;
    }
    // Continua hasya que fiinalcem los TOTAL_PROCESOS o hasta que le poceso padre
    // se quede sin hijos.

    cout << "---Proceso padre imprimiendo los Datos en Memoria ---" << endl;
    for (int i = 0; i < TOTAL_PROCESOS; i++) {
        cout << i << ") " << datosDeMemoria + i * TAMANO_CADENA << endl;
    }

    cout << "El proceso padre ha terminado" << endl;

    // ------ ELIMINACIÓN DE LA MEMORIA COMPARTIDA -----
    if (shmdt(datosDeMemoria) == -1) {
        perror("No se pudo desadjuntar la memoria compartida");
    }
    if (shmctl(idMemoria, IPC_RMID, nullptr) == -1) {
        perror("No se pudo eliminar la memoria compartida");
    }

    return 0;
}