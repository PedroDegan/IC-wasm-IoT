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
from wasmtime import Store, Engine, Linker, Module, FuncType

engine = Engine()
linker = Linker(engine)
store = Store(engine)

# A função precisa do argumento 'caller'
def wasm_log_caller(caller):
    print("GUEST WASM LOG called")

# Defina o tipo: sem argumentos [], sem retorno []
log_type = FuncType([], [])

# CORREÇÃO: Remova o 'store' do primeiro argumento.
# A assinatura correta para a maioria das versões recentes é:
# linker.define_func(modulo, nome, tipo, callback)
linker.define_func("env", "wasm_log", log_type, wasm_log_caller)

module = Module.from_file(engine, "guest_pi.wasm")
instance = linker.instantiate(store, module)

# Acessa a função exportada
# Nota: em algumas versões é instance.exports(store), em outras é apenas instance.exports
exports = instance.exports(store)
guest_filter_func = exports["filter_value"]

# ================================
# MQTT
# ================================
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(BROKER, 1883, 60)

client.loop_forever()
