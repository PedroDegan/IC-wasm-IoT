import paho.mqtt.client as mqtt
from wasmtime import Store, Engine, Linker, Module, FuncType, ValType
import time
import json
import csv

BROKER = "localhost"
ESP_TOPIC = "ic/esp32/#"
FOG_TOPIC = "ic/fog/processed"
DEVICE_ID = "RaspberryPi-Fog1"
LOG_FILE = "dados_umidade.csv"

def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT Broker with result code", rc)
    client.subscribe(ESP_TOPIC)

def save_to_csv(data_dict):
    file_exists = os.path.isfile(LOG_FILE)
    with open(LOG_FILE, 'a', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=data_dict.keys())
        if not file_exists:
            writer.writeheader()
        writer.writerow(data_dict)


def on_message(client, userdata, msg):
    try:
        payload = msg.payload.decode()
        data = json.loads(payload) if payload.startswith("{") else None
        
        if data and "umidade" in data:
            umidade_bruta = data["umidade"]
            
            # Chama o filtro exponencial do C: ALPHA * raw + (1 - ALPHA) * filtered_value
            umidade_filtrada = guest_filter_func(store, umidade_bruta)
            
            result = {
                "timestamp": time.strftime('%Y-%m-%d %H:%M:%S'),
                "device": DEVICE_ID,
                "raw": umidade_bruta,
                "filtered": round(umidade_filtrada, 2),
                "is_dry": data.get("seco", 0)
            }
            
            save_to_csv(result)
            client.publish(FOG_TOPIC, json.dumps(result))
            print(f"Processed: {umidade_bruta} -> {result['filtered']}")
        else:
            print(f"ESP32 INFO: {payload}")
            
    except Exception as e:
        print(f"Error: {e}")

# ================================
# WASM SETUP
# ================================
engine = Engine()
linker = Linker(engine)
store = Store(engine)

def wasm_log_caller(caller, ptr):
    # Lê a string da memória do WASM (o ptr é o endereço)
    mem = caller.get_export("memory").memory()
    # Busca a string até encontrar o caractere nulo \0
    data = mem.read(caller, ptr, 64).split(b'\0')[0]
    print(f"📥 [WASM LOG]: {data.decode('utf-8')}")

log_type = FuncType([ValType.i32()], [])
linker.define_func("env", "wasm_log", log_type, wasm_log_caller)

module = Module.from_file(engine, "guest_pi.wasm")
instance = linker.instantiate(store, module)
guest_filter_func = instance.exports(store)["filter_value"]

start = instance.exports(store).get("_start") or instance.exports(store).get("main")


# ================================
# MQTT
# ================================
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(BROKER, 1883, 60)

client.loop_forever()
