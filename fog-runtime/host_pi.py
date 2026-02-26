import paho.mqtt.client as mqtt
import wasmtime
import time
import json

# -----------------------------
# CONFIGURAÇÕES
# -----------------------------
BROKER = "localhost"
ESP_TOPIC = "ic/esp32/#"
FOG_TOPIC = "ic/fog/processed"
DEVICE_ID = "RaspberryPi-Fog1"

# -----------------------------
# MQTT CALLBACKS
# -----------------------------
def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT Broker with result code", rc)
    client.subscribe(ESP_TOPIC)

def on_message(client, userdata, msg):
    try:
        payload = msg.payload.decode()
        data = json.loads(payload) if payload.startswith("{") else {"raw": payload}
        umidade = data.get("umidade", 0)

        filtered = guest_filter_func(umidade)

        result = {
            "device_id": DEVICE_ID,
            "timestamp": int(time.time()),
            "raw": umidade,
            "filtered": filtered,
            "original_msg": payload
        }

        client.publish(FOG_TOPIC, json.dumps(result))
        print("Processed:", result)

    except Exception as e:
        print("Error processing message:", e)

# -----------------------------
# WASM INIT
# -----------------------------
store = wasmtime.Store()
engine = store.engine
linker = wasmtime.Linker(engine)

# Função de log do WASM
def wasm_log_caller():
    print("GUEST WASM LOG called")

# Criar Func
wasm_log_func = wasmtime.Func(store, wasmtime.FuncType([], []), wasm_log_caller)
linker.define("env", "wasm_log", wasm_log_func)

# Carrega módulo WASM
module = wasmtime.Module.from_file(engine, "guest_pi.wasm")

# Instancia via linker
instance = linker.instantiate(store, module)

# Função exportada do WASM
guest_filter_func = instance.exports(store)["filter_value"]

# -----------------------------
# MQTT CLIENT
# -----------------------------
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(BROKER, 1883, 60)

# -----------------------------
# LOOP PRINCIPAL
# -----------------------------
client.loop_forever()
