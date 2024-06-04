import paho.mqtt.client as mqtt
import time

# Define the data structure
DATA = {
    "RUB": "0.011USD",
    "EUR": "1.09USD",
    "CZK": "0.044USD",
    "BTC": "42'155.80USD",
    "ETH": "2'247.84USD",
}

# Define MQTT message prefixes
PREFIX_DATA = "[DATA]"
PREFIX_CURRENCY = "[CURRENCY]"

# Set the initial data type
CURRENCY = list(DATA.keys())[0]

# Callback function when the MQTT client connects to the broker
def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    # Subscribe to the "test" topic
    client.subscribe("test")

# Callback function when a message is received from the broker
def on_message(client, userdata, msg):
    global CURRENCY
    message = msg.payload.decode()
    print(message)
    if PREFIX_CURRENCY in message:
        # Extract the currency from the message
        data = message.split(" ")[1]
        if data in DATA.keys():
            CURRENCY = data

# Create an MQTT client instance
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Connect to the MQTT broker
client.connect("broker.emqx.io", 1883, 60)

# Start the MQTT client loop
client.loop_start()

# Wait for the client to be connected
while not client.is_connected():
    time.sleep(1)

try:
    # Main loop to publish data every 5 seconds
    while True:
        time.sleep(5)
        # Publish data and data types to the "test" topic
        client.publish("test", f"{PREFIX_CURRENCY} {CURRENCY}")
        client.publish("test", f"{PREFIX_DATA} {DATA[CURRENCY]}")
except KeyboardInterrupt:
    # Stop the MQTT client loop
    client.loop_stop()

# Stop the MQTT client loop
client.loop_stop()
