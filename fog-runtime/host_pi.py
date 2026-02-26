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
# 1. Importe o FuncType
from wasmtime import Store, Engine, Linker, Module, FuncType

engine = Engine()
store = Store(engine)
linker = Linker(engine)

# 2. A função agora recebe um parâmetro 'caller' (obrigatório pelo Wasmtime)
def wasm_log_caller(caller):
    print("GUEST WASM LOG called")

# 3. Defina o tipo da função: FuncType([parâmetros], [retornos])
# Como sua função não recebe nada e não retorna nada, usamos listas vazias.
log_type = FuncType([], [])

# 4. Use a nova assinatura do define_func
linker.define_func(store, "env", "wasm_log", log_type, wasm_log_caller)

module = Module.from_file(engine, "guest_pi.wasm")
instance = linker.instantiate(store, module)

# Acessa função exportada
guest_filter_func = instance.exports(store)["filter_value"]

# ================================
# MQTT
# ================================
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(BROKER, 1883, 60)

client.loop_forever()
