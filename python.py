from vpython import *
import paho.mqtt.client as mqtt
import json
import math

# Create a 3D scene
scene = canvas(title="MPU6050 Egg Rotation", width=800, height=600)

# Create an egg shape (ellipsoid)
egg = ellipsoid(pos=vector(0, 0, 0), length=2.5, height=3.5, width=2, color=color.red)

# Initial angles
x_angle, y_angle, z_angle = 0, 0, 0

def rotate_egg(x_angle, y_angle, z_angle):
    """
    Rotates the egg based on x, y, and z angles.
    """
    global egg
    egg.color = color.blue  # Change color while rotating

    # Convert degrees to radians
    x_rad = radians(x_angle)
    y_rad = radians(y_angle)
    z_rad = radians(z_angle)

    # Apply rotation (adjust axis to match MPU6050 orientation)
    egg.axis = vector(cos(y_rad) * cos(z_rad),
                      sin(x_rad),
                      cos(y_rad) * sin(z_rad))

def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT Broker")
    client.subscribe("downlink/rotation")  # Subscribe to topic

def on_message(client, userdata, msg):
    global x_angle, y_angle, z_angle
    try:
        payload = json.loads(msg.payload.decode("utf-8"))  # Parse JSON from ESP32
        x_angle = float(payload.get("x", 0)) * 10  # Scaling for better visualization
        y_angle = float(payload.get("y", 0)) * 10
        z_angle = float(payload.get("z", 0)) * 10

        print(f"✅ Received: X={x_angle:.2f}, Y={y_angle:.2f}, Z={z_angle:.2f}")

    except Exception as e:
        print(f"❌ Error processing MQTT message: {e}")

# MQTT Setup
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("broker.emqx.io", 1883, 60)

# Start MQTT Loop
client.loop_start()

# Keep the 3D visualization running
while True:
    rotate_egg(x_angle, y_angle, z_angle)  # Rotate based on ESP32 data
    rate(30)  # Update at 30 FPS
