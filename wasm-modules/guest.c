// ============================================================
// IMPORTACOES DO HOST
// ============================================================
extern int read_sensor(int sensor_type);
extern void set_output(int pin, int state);
extern void delay_ms(int ms);
extern void wasm_log(const char* msg);
extern void log_int(int value);
extern int mqtt_publish(const char* topic, const char* msg);

// ============================================================
// DEFINICOES E CALIBRACAO
// ============================================================
#define LED_RED     18
#define LED_GREEN   19
#define LED_BLUE    21

#define LIMIAR_SECO 1700
#define ADC_SECO    2600  // Valor lido no ar
#define ADC_MOLHADO 1100  // Valor lido na agua

char mqtt_buffer[128];

// ============================================================
// FUNCOES AUXILIARES
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

// Funcao corrigida para usar as constantes definidas no topo
int raw_to_percentage(int raw) {
    int pct = 100 - ((raw - ADC_MOLHADO) * 100 / (ADC_SECO - ADC_MOLHADO));
    if (pct > 100) pct = 100;
    if (pct < 0) pct = 0;
    return pct;
}

// Adicionado o argumento 'porcentagem' que faltava na assinatura
void build_json(char *buffer, int umidade, int seco, int porcentagem) {
    char u_str[16], s_str[4], p_str[16];

    int_to_str(umidade, u_str);
    int_to_str(seco, s_str);
    int_to_str(porcentagem, p_str);

    int i = 0;
    int j = 0;

    buffer[i++] = '{';
    
    // Campo "umidade"
    buffer[i++] = '"'; buffer[i++] = 'u'; buffer[i++] = 'm'; buffer[i++] = 'i';
    buffer[i++] = 'd'; buffer[i++] = 'a'; buffer[i++] = 'd'; buffer[i++] = 'e';
    buffer[i++] = '"'; buffer[i++] = ':';
    j = 0; while (u_str[j]) buffer[i++] = u_str[j++];

    buffer[i++] = ',';

    // Campo "seco"
    buffer[i++] = '"'; buffer[i++] = 's'; buffer[i++] = 'e'; buffer[i++] = 'c';
    buffer[i++] = 'o'; buffer[i++] = '"'; buffer[i++] = ':';
    j = 0; while (s_str[j]) buffer[i++] = s_str[j++];

    buffer[i++] = ',';

    // Campo "pct"
    buffer[i++] = '"'; buffer[i++] = 'p'; buffer[i++] = 'c'; buffer[i++] = 't';
    buffer[i++] = '"'; buffer[i++] = ':';
    j = 0; while (p_str[j]) buffer[i++] = p_str[j++];

    buffer[i++] = '}';
    buffer[i] = '\0';
}

// ============================================================
// MAIN
// ============================================================

int main() {
    wasm_log("Iniciando Sistema de Irrigacao WASM...");
    mqtt_publish("ic/esp32/status", "Sistema iniciado");

    while(1) {
        // Lendo o sensor (Isso faltava no seu loop!)
        int umidade = read_sensor(0);

        // Calculando a porcentagem usando a funcao auxiliar
        int porcentagem = raw_to_percentage(umidade);
        
        // Log da porcentagem no terminal (via Host)
        log_int(porcentagem);

        int seco_flag = (umidade > LIMIAR_SECO) ? 1 : 0;

        // Montando o JSON com todos os campos
        build_json(mqtt_buffer, umidade, seco_flag, porcentagem);
        mqtt_publish("ic/esp32/data", mqtt_buffer);

        if (umidade > LIMIAR_SECO) {
            wasm_log("Status: Solo Seco! Ligando Bomba.");
            set_output(LED_GREEN, 0);
            set_output(LED_RED, 1);
            mqtt_publish("ic/esp32/status", "Solo seco - irrigando");
            delay_ms(3000); 
            wasm_log("Rega finalizada.");
            mqtt_publish("ic/esp32/status", "Rega finalizada");
        } else {
            wasm_log("Status: Solo OK.");
            set_output(LED_RED, 0);
            set_output(LED_GREEN, 1);
            mqtt_publish("ic/esp32/status", "Solo OK");
        }

        delay_ms(3000); 
    }
    return 0;
}
