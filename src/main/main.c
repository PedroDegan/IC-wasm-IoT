/*
 * Copyright (C) 2019-21 Intel Corporation and others.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wasm_export.h"
#include "bh_platform.h"
#include "test_wasm.h"

#include "esp_log.h"

#define IWASM_MAIN_STACK_SIZE 5120

#define WASM_STACK_SIZE (8 * 1024) // 8 KB
#define WASM_HEAP_SIZE (8 * 1024)

#define LOG_TAG "wamr"

static void *
app_instance_main(wasm_module_inst_t module_inst)
{
    const char *exception;

    wasm_application_execute_main(module_inst, 0, NULL);
    if ((exception = wasm_runtime_get_exception(module_inst)))
        printf("%s\n", exception);
    return NULL;
}

static int32_t
sensor_read(wasm_exec_env_t exec_env)
{
    // Lógica de simulação: Retorna um valor fixo (25)
    ESP_LOGI(LOG_TAG, "Host Function: Simulação de leitura de sensor [25]");
    return 25; 
}

// Função de atraso (Host)
static void
wasm_delay(wasm_exec_env_t exec_env, int32_t ms)
{
    ESP_LOGI(LOG_TAG, "Host Function: Atrasando por %d ms", (int)ms);
    // Converte milissegundos para Ticks
    vTaskDelay(pdMS_TO_TICKS(ms)); 
}

// Função de randomização (Host)
static int32_t
wasm_rand(wasm_exec_env_t exec_env)
{
    // Retorna um valor randomico de 32 bits
    return (int32_t)esp_random(); 
}

// Estrutura que descreve a função a ser exportada
static NativeSymbol extended_native_symbols[] = {
    {
        // Nome que o código WASM (Guest) usará para importar
        .symbol = "sensor_read", 
        // A função C/C++ real que será chamada
        .func_ptr = (void*)sensor_read, 
        // A assinatura da função (tipo de retorno e argumentos)
        // 'i()' -> retorna i32, sem argumentos
        .signature = "()i",
        
        .attachment = NULL 
    },
    {
    	// Nome que o código WASM (Guest) usará para importar
        .symbol = "delay_ms", 
        // A função C/C++ real que será chamada
        .func_ptr = (void*)wasm_delay, 
        // A assinatura da função (tipo de retorno e argumentos)
        // Assinatura: i32 (argumento) -> nada (void)
        .signature = "(i)",
        
        .attachment = NULL 
    },
    {
    	// Nome que o código WASM (Guest) usará para importar
        .symbol = "get_random", 
        // A função C/C++ real que será chamada
        .func_ptr = (void*)wasm_rand, 
        // A assinatura da função (tipo de retorno e argumentos)
        // 'i()' -> retorna i32, sem argumentos
        .signature = "()i",
        
        .attachment = NULL 
    }
};

void *
iwasm_main(void *arg)
{
    (void)arg; /* unused */
    /* setup variables for instantiating and running the wasm module */
    uint8_t *wasm_file_buf = NULL;
    unsigned wasm_file_buf_size = 0;
    wasm_module_t wasm_module = NULL;
    wasm_module_inst_t wasm_module_inst = NULL;
    char error_buf[128];
    void *ret;
    RuntimeInitArgs init_args;

    /* configure memory allocation */
    memset(&init_args, 0, sizeof(RuntimeInitArgs));
#if WASM_ENABLE_GLOBAL_HEAP_POOL == 0
    init_args.mem_alloc_type = Alloc_With_Allocator;
    init_args.mem_alloc_option.allocator.malloc_func = (void *)os_malloc;
    init_args.mem_alloc_option.allocator.realloc_func = (void *)os_realloc;
    init_args.mem_alloc_option.allocator.free_func = (void *)os_free;
#else
#error The usage of a global heap pool is not implemented yet for esp-idf.
#endif

    ESP_LOGI(LOG_TAG, "Initialize WASM runtime");
    /* initialize runtime environment */
    if (!wasm_runtime_full_init(&init_args)) {
        ESP_LOGE(LOG_TAG, "Init runtime failed.");
        return NULL;
    }
    
    
    /* register the native functions */
    if (!wasm_runtime_register_natives("env",             // Nome do módulo WASM
    				       extended_native_symbols, // Array de símbolos
                                       sizeof(extended_native_symbols) / sizeof(NativeSymbol))) {
        ESP_LOGE(LOG_TAG, "Failed to register native functions.");
        wasm_runtime_destroy();
        return NULL;
    }
    ESP_LOGI(LOG_TAG, "Host function 'sensor_read' registered successfully.");


#if WASM_ENABLE_INTERP != 0
    ESP_LOGI(LOG_TAG, "Run wamr with interpreter");

    wasm_file_buf = (uint8_t *)test_wasm;
    wasm_file_buf_size = test_wasm_len;

    /* load WASM module */
    if (!(wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_buf_size,
                                          error_buf, sizeof(error_buf)))) {
        ESP_LOGE(LOG_TAG, "Error in wasm_runtime_load: %s", error_buf);
        goto fail1interp;
    }

    ESP_LOGI(LOG_TAG, "Instantiate WASM runtime");
    if (!(wasm_module_inst =
              wasm_runtime_instantiate(wasm_module, WASM_STACK_SIZE, // stack size
                                       WASM_HEAP_SIZE,              // heap size
                                       error_buf, sizeof(error_buf)))) {
        ESP_LOGE(LOG_TAG, "Error while instantiating: %s", error_buf);
        goto fail2interp;
    }

    ESP_LOGI(LOG_TAG, "run main() of the application");
    ret = app_instance_main(wasm_module_inst);
    assert(!ret);

    /* destroy the module instance */
    ESP_LOGI(LOG_TAG, "Deinstantiate WASM runtime");
    wasm_runtime_deinstantiate(wasm_module_inst);

fail2interp:
    /* unload the module */
    ESP_LOGI(LOG_TAG, "Unload WASM module");
    wasm_runtime_unload(wasm_module);

fail1interp:
#endif

    /* destroy runtime environment */
    ESP_LOGI(LOG_TAG, "Destroy WASM runtime");
    wasm_runtime_destroy();

    return NULL;
}

void
app_main(void)
{
    pthread_t t;
    int res;

    pthread_attr_t tattr;
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setstacksize(&tattr, IWASM_MAIN_STACK_SIZE);

    res = pthread_create(&t, &tattr, iwasm_main, (void *)NULL);
    assert(res == 0);

    res = pthread_join(t, NULL);
    assert(res == 0);

    ESP_LOGI(LOG_TAG, "Exiting...");
}
