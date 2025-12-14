// test.c

// Declarar as Host Functions fornecidas pelo Host (ESP-IDF)
extern int printf(const char *format, ...);
extern int sensor_read(void);
extern void delay_ms(int ms);           // Novo: Atraso
extern int get_random(void);            // Novo: Randomização

#define LOOP_DELAY_MS 5000 // 5 segundos

int main() {
    
    // Loop principal 
    while (1) {
        
        // 1. Simular Leitura do Sensor (usa o valor fixo 25)
        int valor_sensor = sensor_read();
        
        // 2. Gerar Valor Randomico 
        // Usamos & 0xFFF para manter o número pequeno (0 a 4095, como um ADC)
        int valor_random = get_random() & 0xFFF; 
        
        // 3. Imprimir Log
        printf("WASM: Leitura Fixa: %d | Random (0-4095): %d\n", valor_sensor, valor_random);
        
        // 4. Atrasar
        delay_ms(LOOP_DELAY_MS);
    }
    
    return 0;
}
