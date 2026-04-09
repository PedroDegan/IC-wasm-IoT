import paho.mqtt.client as mqtt
from wasmtime import Store, Engine, Linker, Module, FuncType, ValType, Func
import time, json, csv, os

BROKER    = "localhost"
ESP_TOPIC = "ic/esp32/#"
FOG_TOPIC = "ic/fog/processed"
DEVICE_ID = "RaspberryPi-Fog1"
LOG_FILE  = "dados_umidade.csv"

def on_connect(client, userdata, flags, rc):
    print("Connected:", rc)
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
        if not payload.startswith("{"):
            print(f"ESP32 INFO: {payload}")
            return
        data = json.loads(payload)
        if "raw" not in data:
            return
        raw     = data["raw"]
        seco    = data.get("seco",    2600)
        molhado = data.get("molhado", 1100)
        limiar  = data.get("limiar",  1700)
        result_packed = wasm_process(store, raw, seco, molhado, limiar)
        pct     = result_packed & 0xFF
        irrigar = (result_packed >> 8) & 0x1
        result = {
            "timestamp": time.strftime('%Y-%m-%d %H:%M:%S'),
            "device":    DEVICE_ID,
            "raw":       raw,
            "pct":       pct,
            "irrigar":   irrigar,
        }
        save_to_csv(result)
        client.publish(FOG_TOPIC, json.dumps(result))
        print(f"Processed: raw={raw} pct={pct}% irrigar={irrigar}")
    except Exception as e:
        print(f"Error: {e}")

# WASM setup
engine  = Engine()
linker  = Linker(engine)
store   = Store(engine)

log_type     = FuncType([ValType.i32()], [])
log_int_type = FuncType([ValType.i32()], [])

linker.define("env", "wasm_log", Func(store, log_type,     lambda p: None))
linker.define("env", "log_int",  Func(store, log_int_type, lambda v: print(f"[WASM] {v}%")))

module       = Module.from_file(engine, "guest.wasm")
instance     = linker.instantiate(store, module)
wasm_process = instance.exports(store)["process"]

# MQTT
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
client.on_connect = on_connect
client.on_message = on_message
client.connect(BROKER, 1883, 60)
client.loop_forever()
