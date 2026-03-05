import streamlit as st
import pandas as pd
import json
import paho.mqtt.client as mqtt
import os
from wasmtime import Store, Engine, Linker, Module, FuncType, ValType
from streamlit_autorefresh import st_autorefresh

# --- 1. CONFIGURAÇÃO DA PÁGINA ---
st.set_page_config(page_title="UFABC IC - WASM Cloud Node", layout="wide")

# Refresh de 1 segundo para ler o arquivo de buffer
st_autorefresh(interval=1000, key="cloud_node_refresh")

# --- 2. INICIALIZAÇÃO DE ESTADO ---
if 'history' not in st.session_state:
    st.session_state.history = pd.DataFrame(columns=['timestamp', 'raw', 'pct', 'filtered'])

# --- 3. CONFIGURAÇÃO WASM ---
@st.cache_resource
def load_wasm():
    try:
        engine = Engine()
        store = Store(engine)
        module = Module.from_file(engine, "guest_pi.wasm")
        linker = Linker(engine)
        log_type = FuncType([ValType.i32()], [])
        linker.define_func("env", "wasm_log", log_type, lambda caller, ptr: None)
        instance = linker.instantiate(store, module)
        return engine, store, instance.exports(store)["filter_value"]
    except Exception as e:
        st.error(f"Erro ao carregar WASM: {e}")
        return None, None, None

engine, store, wasm_filter = load_wasm()

# --- 4. CONFIGURAÇÃO MQTT ---
BROKER_IP = "192.168.0.122"
TOPIC = "ic/esp32/data"
BUFFER_FILE = "mqtt_temp_data.json"

def on_message(client, userdata, msg):
    try:
        # Quando chega dado, escrevemos no disco. 
        # Isso evita o erro de ScriptRunContext do Streamlit.
        with open(BUFFER_FILE, "w") as f:
            f.write(msg.payload.decode())
    except:
        pass

if 'mqtt_client' not in st.session_state:
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    client.on_message = on_message
    try:
        client.connect(BROKER_IP, 1883, 60)
        client.subscribe(TOPIC)
        client.loop_start()
        st.session_state.mqtt_client = client
    except Exception as e:
        st.error(f"Erro MQTT: {e}")

# --- 5. INTERFACE (UI) ---
st.title("☁️ Phase 3: WASM Cloud Execution")
st.markdown(f"**Status:** Conectado a `{BROKER_IP}` | **Runtime:** Wasmtime")

# SIDEBAR
st.sidebar.header("⚙️ Calibração Remota")
s_seco = st.sidebar.slider("ADC Ar (Seco)", 2000, 4000, 2600)
s_molhado = st.sidebar.slider("ADC Água (Molhado)", 500, 2000, 1100)
s_limiar = st.sidebar.slider("Limiar de Irrigação", 1000, 3000, 1700)

if st.sidebar.button("Enviar Configuração"):
    config_data = {"seco": s_seco, "molhado": s_molhado, "limiar": s_limiar}
    st.session_state.mqtt_client.publish("ic/esp32/config", json.dumps(config_data))
    st.sidebar.success("Enviado!")

# --- 6. PROCESSAMENTO (Lendo o buffer de disco) ---
if os.path.exists(BUFFER_FILE):
    try:
        with open(BUFFER_FILE, "r") as f:
            payload = json.load(f)
        
        # Deleta o arquivo para indicar que já processamos esse dado
        os.remove(BUFFER_FILE)
        
        raw_val = payload.get("umidade", 0)
        pct_val = payload.get("pct", 0)
        
        # Execução WASM
        if wasm_filter:
            filtered_val = round(wasm_filter(store, raw_val), 2)
        else:
            filtered_val = 0.0

        new_entry = {
            'timestamp': pd.Timestamp.now(),
            'raw': raw_val,
            'pct': pct_val,
            'filtered': filtered_val
        }
        
        # Concatena no histórico
        new_df = pd.DataFrame([new_entry])
        st.session_state.history = pd.concat([st.session_state.history, new_df], ignore_index=True).tail(50)
    except Exception as e:
        pass

# --- 7. VISUALIZAÇÃO ---
if not st.session_state.history.empty:
    df = st.session_state.history
    last = df.iloc[-1]
    
    col1, col2, col3 = st.columns(3)
    col1.metric("ADC Bruto", f"{int(last['raw'])}")
    col2.metric("Umidade", f"{int(last['pct'])}%")
    col3.metric("Filtro WASM", f"{last['filtered']}")
    
    st.subheader("Gráfico de Monitoramento")
    # Plotamos o valor bruto vs o valor filtrado pelo WASM no Laptop
    st.line_chart(df.set_index('timestamp')[['raw', 'filtered']])
else:
    st.info("Aguardando mensagens no tópico ic/esp32/data...")
    st.caption("Dica: Certifique-se que a ESP32 e o Broker estão na mesma rede.")

st.divider()
st.caption("Federal University of ABC - Scientific Initiation")
