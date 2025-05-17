# PracticaMeltdown

Este repositorio contiene una práctica para explorar la vulnerabilidad **Meltdown** mediante un ejemplo en lenguaje C. El objetivo es comprender cómo esta vulnerabilidad puede ser explotada para acceder a información privilegiada en sistemas afectados.

## Compilación

Para compilar el programa, utiliza el siguiente comando:

```bash
gcc -O2 -o meltdown meltdown.c

```

Una vez compilado, puedes ejecutar el programa pasando como argumento la dirección de memoria del kernel que deseas intentar leer. Por ejemplo, para leer la dirección del símbolo linux_proc_banner:

```bash
sudo grep linux_proc_banner /proc/kallsyms

```

Finalmente ejecutar con:

```bash
./meltdown <dirección_en_hexadecimal>


```
