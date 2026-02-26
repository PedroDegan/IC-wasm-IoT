// guest_pi.c

// Declaração da função de log do host
extern void wasm_log(const char* msg)
    __attribute__((import_module("env"), import_name("wasm_log")));

static float filtered_value = 0.0;
#define ALPHA 0.3

// função exportada para filtrar o valor
float filter_value(int raw) {
    filtered_value = ALPHA * raw + (1 - ALPHA) * filtered_value;
    return filtered_value;
}

int main() {
    wasm_log("Guest Pi iniciado!");
    while(1) {
        // Loop vazio, host passa valores para filter_value
    }
    return 0;
}
