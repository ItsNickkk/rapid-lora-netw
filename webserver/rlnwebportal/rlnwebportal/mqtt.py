import django
django.setup()
import paho.mqtt.client as mqtt
import json
from datetime import datetime
from main.models import entries, dim_node, dim_entry_status, dim_node_status
import requests


broker_ip = "192.168.0.22"
topic = "RLN/gateway"

def on_connect(client, userdata, rc, properties=None):
	client.subscribe(topic)

def on_message(client, userdata, msg):
	print(msg.payload.decode())
	n = dim_node
	es = dim_entry_status
	try:
		dt = datetime.now()
		json_payload = json.loads(msg.payload.decode())
		if "update" in json_payload:
			pass
		else:
			name = json_payload["n"]
			add = json_payload["add"]
			hpno = json_payload["hp"]
			count = json_payload["cnt"]
			danger = True if "dg" in json_payload else False
			firstaid = True if "fa" in json_payload else False
			water = True if "wt" in json_payload else False
			food = True if "fd" in json_payload else False
			hug = True if "hg" in json_payload else False
			node = n.objects.get(pk=json_payload["node"])
			status = es.objects.get(pk=1)
			entry = entries(name = name, address = add, hpno = hpno, occupants = count, danger = danger, firstaid = firstaid, water = water, food = food, hug = hug, node = node, status = status, datetime = dt)
			entry.save()
	except json.JSONDecodeError:
		print(msg.payload)
	except TypeError:
		print(msg.payload)
	except KeyError:
		print(msg.payload)
	pass

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(broker_ip, 1883, 60)