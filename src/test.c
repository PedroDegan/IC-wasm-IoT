//test.c

// Importacoes do Host
extern int read_sensor(int sensor_type);
extern void set_output(int pin, int state);
extern void delay_ms(int ms);
extern void wasm_log(const char* msg);
extern void log_int(int value);

// Definicao de pinos conforme o Host
//#define RELAY_PIN 23
#define LED_RED   18
#define LED_GREEN 19
#define LED_BLUE  21

// CALIBRACAO
#define LIMIAR_SECO 1700 

int main() {
    wasm_log("Iniciando Sistema de Irrigacao WASM...");

    while(1) {
        int umidade = read_sensor(0); // 0 = Umidade (GPIO 32)
        log_int(umidade);
        
        if (umidade < LIMIAR_SECO) {
            // SOLO SECO
            wasm_log("Status: Solo Seco! Ligando Bomba.");
            set_output(LED_GREEN, 0);
            set_output(LED_RED, 1);
            //set_output(RELAY_PIN, 0); // Ativa Rele
            
            delay_ms(3000); // Rega por 3 segundos
            
            //set_output(RELAY_PIN, 1);
            wasm_log("Rega finalizada.");
        } else {
            // SOLO ÚMIDO
            wasm_log("Status: Solo OK.");
            set_output(LED_RED, 0);
            set_output(LED_GREEN, 1);
            }

        delay_ms(3000); // Aguarda 3 segundos para nova leitura
    }
    return 0;
}