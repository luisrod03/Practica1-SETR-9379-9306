Implementamos una máquina de estados discreta en ESP-IDF nativo utilizando cuatro tareas con distintas prioridades y un Idle Hook. 
Toda la sincronización se logró sin semáforos, utilizando únicamente variables compartidas con el modificador volatile y las funciones de suspensión/reanudación del sistema operativo (vTaskSuspend y vTaskResume). 

•	¿Por qué g_ledRapido debe ser volatile y qué ocurre si se omite? 
Al usar volatile, se obliga al compilador a leer el valor real desde la memoria RAM en cada ciclo, ya que el valor puede ser alterado por otra tarea en cualquier momento. Si se omite, el compilador "optimiza" el código guardando la variable en un registro interno, provocando que la tarea nunca se entere de que el botón fue presionado. ¿En qué momento aparece [IDLE]? Describe el estado de las tareas: Aparece exactamente cuando las cuatro tareas de usuario están simultáneamente en estado Bloqueado (esperando que terminen sus respectivos vTaskDelay). En ese instante, la CPU cede el control a la tarea Idle (prioridad 0). 

•	Diferencia entre vTaskDelay() y vTaskDelayUntil().
vTaskDelay() pausa un tiempo relativo desde que se llama la función. 
vTaskDelayUntil() pausa un tiempo absoluto desde el último despertar, garantizando una frecuencia matemática exacta.

•	¿Por qué el Monitor tiene mayor prioridad (4) que el LED (1)? ¿Qué pasa si se invierten? 
La lectura de hardware es un evento crítico que requiere respuesta inmediata. Si se invirtieran, la tarea del LED acapararía el procesador e ignoraría las pulsaciones del botón, generando retrasos o pérdida de eventos. 

•	Riesgo de leer una variable volatile sin protección:
El riesgo es la corrupción de datos. Una sección crítica es un bloque de código protegido que garantiza que una operación de lectura/escritura sea atómica e ininterrumpible. 


En conclusión, la coordinación de tareas mediante el planificador apropiativo de FreeRTOS permite crear sistemas reactivos sin bloquear el procesador en bucles de espera. 
El uso del Idle Hook demuestra la eficiencia energética del RTOS, aprovechando los tiempos muertos del hardware mientras se garantiza una respuesta inmediata a los eventos críticos gracias a una correcta jerarquía de prioridades.

