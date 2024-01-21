from datetime import datetime
import paho.mqtt.client as mqtt

def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    client.subscribe("sensor/temperature/+") #wildcard
    client.subscribe("sensor/humidity/+") #wildcard


def on_message(client, userdata, msg):
    print(f"Received message on topic {msg.topic}: {msg.payload.decode()} " + datetime.now().strftime(
        "%Y-%m-%d %H:%M:%S"))


client = mqtt.Client()

client.on_connect = on_connect
client.on_message = on_message

client.connect("192.168.137.1", 1883)  # jak nie działa to spróbować 127.0.0.1
client.loop_forever()
