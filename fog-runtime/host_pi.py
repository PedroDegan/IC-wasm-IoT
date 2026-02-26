import paho.mqtt.client as mqtt
import time
import json
import wasmtime

# Configurações MQTT
BROKER = "localhost"
TOPIC_ESP = "ic/esp32/data"
TOPIC_PROCESSED = "ic/fog/processed"

# Inicializa MQTT
client = mqtt.Client()
client.connect(BROKER)

# Inicializa WASM
store = wasmtime.Store()
module = wasmtime.Module.from_file(store.engine, "guest_pi.wasm")
instance = wasmtime.Instance(store, module, [])

filter_func = instance.exports(store)["filter_value"]

# Callback MQTT
def on_message(client, userdata, msg):
    payload = json.loads(msg.payload.decode())
    raw_value = payload["umidade"]
    seco = payload["seco"]
    device_id = payload.get("device", "ESP32")  # pega device do ESP32 ou default
    
    # Passa para WASM
    filtered = filter_func(store, raw_value)
    
    # Cria JSON completo com timestamp e device
    result = {
        "device": device_id,
        "umidade": int(filtered),
        "seco": seco,
        "timestamp": int(time.time())
    }
    
    client.publish(TOPIC_PROCESSED, json.dumps(result))
    print(result)

client.subscribe(TOPIC_ESP)
client.on_message = on_message
client.loop_forever()
