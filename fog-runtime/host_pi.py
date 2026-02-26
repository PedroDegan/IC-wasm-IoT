import paho.mqtt.client as mqtt
import wasmtime
import time
import json

BROKER = "localhost"
ESP_TOPIC = "ic/esp32/#"
FOG_TOPIC = "ic/fog/processed"
DEVICE_ID = "RaspberryPi-Fog1"

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

# ================================
# WASM INIT
# ================================
store = wasmtime.Store()
engine = store.engine
linker = wasmtime.Linker(engine)

# função de log do WASM
def wasm_log_caller():
    print("GUEST WASM LOG called")

# Aqui é a correção para wasmtime>=40
linker.define_func("env", "wasm_log", wasm_log_caller)

module = wasmtime.Module.from_file(engine, "guest_pi.wasm")
instance = linker.instantiate(store, module)

# acessa função exportada
guest_filter_func = instance.exports(store)["filter_value"]

# ================================
# MQTT
# ================================
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(BROKER, 1883, 60)

client.loop_forever()
