import serial
import paho.mqtt.publish as client

broker_address="192.168.0.22" 
topic = "RLN/gateway"
keyword = "Incoming"

if __name__ == '__main__':
	ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
	ser.flush()
	try:
		print("Listening for incoming serial message.")
		while True:
			if ser.in_waiting > 0:
				line = ser.readline().decode('utf-8').rstrip()
				print(line)
				if line.startswith(keyword):
					json = line.split("=")[1]
					if not json.startswith("["):
						print("MQTT sending: "+ json)
						# res = client.publish(topic, json)
						client.single(topic, json,  qos=0, retain=False, hostname=broker_address, port=1883, keepalive=60)
	except KeyboardInterrupt:
		print("Interrupt")

