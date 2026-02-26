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
        # tenta interpretar como JSON do ESP32
        data = json.loads(payload) if payload.startswith("{") else {"raw": payload}
        umidade = data.get("umidade", 0)

        # chama WASM para filtrar/processar
        filtered = guest_filter_func(umidade)

        # monta JSON final
        result = {
            "device_id": DEVICE_ID,
            "timestamp": int(time.time()),
            "raw": umidade,
            "filtered": filtered,
            "original_msg": payload
        }

        # publica no tópico processado
        client.publish(FOG_TOPIC, json.dumps(result))
        print("Processed:", result)

    except Exception as e:
        print("Error processing message:", e)

# -----------------------------
# WASM INIT
# -----------------------------
store = wasmtime.Store()
module = wasmtime.Module.from_file(store.engine, "guest_pi.wasm")
linker = wasmtime.Linker(store.engine)

# função de log para WASM
def wasm_log():
    print("GUEST WASM LOG called")

# define import "env"
linker.define("env", "wasm_log", item=wasmtime.Func(store, wasmtime.FuncType([], []), wasm_log))

# instancia via linker
instance = linker.instantiate(store, module)

# acessa a função exportada do WASM (ex: filter_value)
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
