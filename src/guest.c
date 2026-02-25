// guest.c

// ============================================================
// IMPORTACOES DO HOST
// ============================================================

extern int read_sensor(int sensor_type);
extern void set_output(int pin, int state);
extern void delay_ms(int ms);
extern void wasm_log(const char* msg);
extern void log_int(int value);

// >>> ADDED: nova funcao MQTT exportada pelo host
extern int mqtt_publish(const char* topic, const char* msg);


// ============================================================
// DEFINICAO DE PINOS (conforme host)
// ============================================================

#define LED_RED   18
#define LED_GREEN 19
#define LED_BLUE  21


// ============================================================
// CALIBRACAO
// ============================================================

#define LIMIAR_SECO 1700 


// ============================================================
// BUFFERS GLOBAIS  >>> ADDED
// Necessarios porque WASM nao deve retornar ponteiros locais
// ============================================================

char mqtt_buffer[128];


// ============================================================
// FUNCOES AUXILIARES (SEM LIBC)
// ============================================================

void int_to_str(int value, char *buffer) {
    int i = 0;
    int temp = value;

    if (temp == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }

    char rev[16];
    int j = 0;

    while (temp > 0) {
        rev[j++] = (temp % 10) + '0';
        temp /= 10;
    }

    while (j > 0) {
        buffer[i++] = rev[--j];
    }

    buffer[i] = '\0';
}

void build_json(char *buffer, int umidade, int seco) {
    char u_str[16];
    char s_str[4];

    int_to_str(umidade, u_str);
    int_to_str(seco, s_str);

    int i = 0;

    buffer[i++] = '{';

    buffer[i++] = '"';
    buffer[i++] = 'u'; buffer[i++] = 'm'; buffer[i++] = 'i';
    buffer[i++] = 'd'; buffer[i++] = 'a'; buffer[i++] = 'd';
    buffer[i++] = 'e';
    buffer[i++] = '"';
    buffer[i++] = ':';

    int j = 0;
    while (u_str[j]) buffer[i++] = u_str[j++];

    buffer[i++] = ',';

    buffer[i++] = '"';
    buffer[i++] = 's'; buffer[i++] = 'e'; buffer[i++] = 'c';
    buffer[i++] = 'o';
    buffer[i++] = '"';
    buffer[i++] = ':';

    j = 0;
    while (s_str[j]) buffer[i++] = s_str[j++];

    buffer[i++] = '}';
    buffer[i] = '\0';
}

// ============================================================
// MAIN
// ============================================================

int main() {

    wasm_log("Iniciando Sistema de Irrigacao WASM...");
    mqtt_publish("ic/esp32/status", "Sistema iniciado");   // >>> ADDED

    while(1) {

        int umidade = read_sensor(0);  // 0 = Umidade
        log_int(umidade);

        // >>> ADDED: monta JSON simples manualmente
        // Evitamos sprintf pesado; usamos formato simples
        int seco = (umidade > LIMIAR_SECO) ? 1 : 0;

        // JSON simples:
        // {"umidade":1234,"seco":1}
        build_json(mqtt_buffer, umidade, seco);

        mqtt_publish("ic/esp32/data", mqtt_buffer);  // >>> ADDED


        if (umidade > LIMIAR_SECO) {

            // SOLO SECO
            wasm_log("Status: Solo Seco! Ligando Bomba.");

            set_output(LED_GREEN, 0);
            set_output(LED_RED, 1);

            mqtt_publish("ic/esp32/status", "Solo seco - irrigando"); // >>> ADDED

            delay_ms(3000);  // Rega por 3 segundos

            wasm_log("Rega finalizada.");
            mqtt_publish("ic/esp32/status", "Rega finalizada"); // >>> ADDED

        } else {

            // SOLO UMIDO
            wasm_log("Status: Solo OK.");

            set_output(LED_RED, 0);
            set_output(LED_GREEN, 1);

            mqtt_publish("ic/esp32/status", "Solo OK"); // >>> ADDED
        }

        delay_ms(3000);  // Nova leitura
    }

    return 0;
}

