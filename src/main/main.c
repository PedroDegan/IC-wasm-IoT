//main.c

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wasm_export.h"
#include "bh_platform.h"
#include "test_wasm.h" // arquivo
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

// --- MAPEAMENTO DE PINOS (Lado 3.3V/Vin) ---
#define MOISTURE_ADC_CHAN ADC_CHANNEL_4 // GPIO 32
#define RELAY_GPIO        23
#define LED_RED           18
#define LED_GREEN         19
#define LED_BLUE          21

static adc_oneshot_unit_handle_t adc1_handle = NULL;
#define LOG_TAG "WAMR_IRRIGADOR"

// --- NATIVE FUNCTIONS (Para o Guest chamar) ---

// Lê sensores analógicos (0 = Umidade)
static int32_t host_read_sensor(wasm_exec_env_t exec_env, int32_t sensor_type) {
    int sum = 0;
    int raw = 0;
    adc_channel_t chan = MOISTURE_ADC_CHAN;

    for (int i = 0; i < 16; i++) {
        adc_oneshot_read(adc1_handle, chan, &raw);
        sum += raw;
        sys_delay_ms(10);
    }
    adc_oneshot_read(adc1_handle, chan, &raw);
    return sum / 16;
}


// Controla saídas digitais (Relé e LEDs)
static void host_set_output(wasm_exec_env_t exec_env, int32_t pin, int32_t state) {
    gpio_set_level((gpio_num_t)pin, state);
}

// Delay usando FreeRTOS
static void host_delay_ms(wasm_exec_env_t exec_env, int32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

static void host_log_int(wasm_exec_env_t exec_env, int32_t value) {
    ESP_LOGI("WASM_VAL", "Sensor = %d", value);
}

// Printf customizado para o WASM não precisar de stdio.h
static void host_print(wasm_exec_env_t exec_env, uint32_t msg_offset) {
    wasm_module_inst_t module_inst =
        wasm_runtime_get_module_inst(exec_env);

    if (!wasm_runtime_validate_app_str_addr(module_inst, msg_offset)) {
        ESP_LOGE("WASM_LOG", "String invalida recebida do WASM");
        return;
    }

    const char *native_msg =
        wasm_runtime_addr_app_to_native(module_inst, msg_offset);

    ESP_LOGI("WASM_LOG", "%s", native_msg);
}


// --- TABELA DE SÍMBOLOS ---
static NativeSymbol extended_native_symbols[] = {
    { "read_sensor", host_read_sensor, "(i)i", NULL },
    { "set_output",  host_set_output,  "(ii)", NULL },
    { "delay_ms",    host_delay_ms,    "(i)",  NULL },
    { "wasm_log",    host_print,       "(i)",  NULL },
    { "log_int",     host_log_int,     "(i)",  NULL },

};

// --- INICIALIZAÇÃO DE HARDWARE ---
void init_hardware() {
    // Configura Saídas (Relé e LEDs)
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL<<RELAY_GPIO) | (1ULL<<LED_RED) | (1ULL<<LED_GREEN) | (1ULL<<LED_BLUE),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = 0, .pull_up_en = 0, .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // Configura ADC (Unit 1)
    adc_oneshot_unit_init_cfg_t init_config = { .unit_id = ADC_UNIT_1, .clk_src = 0 };
    adc_oneshot_new_unit(&init_config, &adc1_handle);
    adc_oneshot_chan_cfg_t config = { .bitwidth = ADC_BITWIDTH_12, .atten = ADC_ATTEN_DB_12 };
    adc_oneshot_config_channel(adc1_handle, MOISTURE_ADC_CHAN, &config);
}

// --- IWASM MAIN (Lógica do WAMR) ---
void *iwasm_main(void *arg) {
    uint8_t *wasm_file_buf = os_malloc(test_wasm_len);
    if (!wasm_file_buf) {
        ESP_LOGE(LOG_TAG, "Falha ao alocar buffer do WASM");
        return NULL;
    }

    memcpy(wasm_file_buf, test_wasm, test_wasm_len);
    unsigned wasm_file_buf_size = test_wasm_len;
    
    wasm_module_t wasm_module = NULL;
    wasm_module_inst_t wasm_module_inst = NULL;
    char error_buf[128];
    RuntimeInitArgs init_args;

    memset(&init_args, 0, sizeof(RuntimeInitArgs));
    init_args.mem_alloc_type = Alloc_With_Allocator;
    init_args.mem_alloc_option.allocator.malloc_func = (void *)os_malloc;
    init_args.mem_alloc_option.allocator.realloc_func = (void *)os_realloc;
    init_args.mem_alloc_option.allocator.free_func = (void *)os_free;

    if (!wasm_runtime_full_init(&init_args)) return NULL;

    wasm_runtime_register_natives("env", extended_native_symbols, sizeof(extended_native_symbols) / sizeof(NativeSymbol));

    if (!(wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_buf_size, error_buf, sizeof(error_buf)))) goto fail;

    if (!(wasm_module_inst = wasm_runtime_instantiate(wasm_module, 8*1024, 8*1024, error_buf, sizeof(error_buf)))) goto fail;

    wasm_application_execute_main(wasm_module_inst, 0, NULL);

    wasm_runtime_deinstantiate(wasm_module_inst);
    wasm_runtime_unload(wasm_module);
fail:
    os_free(wasm_file_buf);
    wasm_runtime_destroy();
    return NULL;
}

void app_main(void) {
    init_hardware();
    pthread_t t;
    pthread_attr_t tattr;
    pthread_attr_init(&tattr);
    pthread_attr_setstacksize(&tattr, 5120);
    pthread_create(&t, &tattr, iwasm_main, NULL);
    pthread_join(t, NULL);
}