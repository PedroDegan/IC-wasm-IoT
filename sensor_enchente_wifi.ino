#include <WiFi.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include "m3/m3.h"
#include "m3/m3_env.h"

const char* ssid = "SEU_WIFI";
const char* password = "SENHA";

String wasm_url = "http://192.168.0.10:8000/risk.wasm";   // <-- coloque seu IP

// ========================================================
// Inicialização Wasm3
// ========================================================
IM3Environment env;
IM3Runtime runtime;
IM3Module module;
IM3Function fn_calcular;

// ========================================================
// Função: baixa o WASM e salva localmente
// ========================================================
bool baixarWASM() {
  Serial.println("🌐 Baixando WASM...");

  HTTPClient http;
  http.begin(wasm_url);

  int httpCode = http.GET();
  if (httpCode != 200) {
    Serial.println("❌ ERRO no download do WASM");
    return false;
  }

  WiFiClient* stream = http.getStreamPtr();

  File f = LittleFS.open("/risk.wasm", "w");
  if (!f) {
    Serial.println("❌ Erro ao gravar WASM");
    return false;
  }

  uint8_t buffer[512];
  while (http.connected() && stream->available()) {
    int len = stream->readBytes(buffer, sizeof(buffer));
    f.write(buffer, len);
  }

  f.close();
  http.end();

  Serial.println("✔ WASM baixado e salvo!");
  return true;
}

// ========================================================
// Carregar WASM no Wasm3
// ========================================================
bool carregarWASM() {

  File f = LittleFS.open("/risk.wasm", "r");
  if (!f) {
    Serial.println("❌ Não encontrei /risk.wasm");
    return false;
  }

  size_t size = f.size();
  uint8_t *wasm_bytes = (uint8_t*) malloc(size);
  f.read(wasm_bytes, size);
  f.close();

  env = m3_NewEnvironment();
  runtime = m3_NewRuntime(env, 32 * 1024, NULL);

  M3Result r = m3_ParseModule(env, &module, wasm_bytes, size);
  if (r) { Serial.printf("Parse error: %s\n", r); return false; }

  r = m3_LoadModule(runtime, module);
  if (r) { Serial.printf("Load error: %s\n", r); return false; }

  r = m3_FindFunction(&fn_calcular, runtime, "calcular");
  if (r) { Serial.printf("Find error: %s\n", r); return false; }

  Serial.println("✔ WASM carregado e pronto!");
  return true;
}


// ========================================================
// Chamar função calcular do WASM
// ========================================================
int calcularRiscoViaWASM(int s1, int s2, int s3, int s4) {
  const char* args[4];
  static char a1[8], a2[8], a3[8], a4[8];

  sprintf(a1, "%d", s1);
  sprintf(a2, "%d", s2);
  sprintf(a3, "%d", s3);
  sprintf(a4, "%d", s4);

  args[0] = a1;
  args[1] = a2;
  args[2] = a3;
  args[3] = a4;

  M3Result r = m3_CallArgv(fn_calcular, 4, args);
  if (r) { Serial.printf("Exec error: %s\n", r); return -1; }

  int result;
  m3_GetResultsV(fn_calcular, &result);
  return result;
}


// ========================================================
// SETUP
// ========================================================
void setup() {
  Serial.begin(115200);

  LittleFS.begin(true);

  WiFi.begin(ssid, password);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\n✔ WiFi OK");

  baixarWASM();
  carregarWASM();
}


// ========================================================
// LOOP
// ========================================================
void loop() {
  int s1 = 0;
  int s2 = 0;
  int s3 = 0;
  int s4 = 0;

  int risco = calcularRiscoViaWASM(s1, s2, s3, s4);
  Serial.printf("Risco calculado via WASM: %d\n", risco);

  delay(2000);
}

