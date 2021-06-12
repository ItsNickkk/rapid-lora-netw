from django.shortcuts import render
from django.http import JsonResponse, QueryDict
from django.db.models import Q
from main.models import entries, dim_entry_status, dim_node

def home(request):

	return render(request, 'main/home.html')

def manageEntries(request):
	json = {"data":[]}
	nodes = dim_node.objects
	ents = entries.objects.order_by('-id')
	entry_stat = dim_entry_status.objects
	if request.method == 'GET':
		for entry in ents:
			dict = {}
			node_arr = {}
			dict["id"] = entry.id
			dict["name"] = entry.name
			dict["address"] = entry.address
			dict["hpno"] = entry.hpno
			dict["occupants"] = entry.occupants
			dict["danger"] = entry.danger
			dict["water"] = entry.water
			dict["food"] = entry.food
			dict["hug"] = entry.hug
			status_filter = Q(pk=entry.status_id)
			dict["status"] = entry_stat.filter(status_filter)[0].status
			dict["datetime"] = entry.datetime
			dict["firstaid"] = entry.firstaid
			dict["node"] = node_arr
			reqNodeID = Q(pk=entry.node_id)
			node = nodes.filter(reqNodeID)[0]
			node_arr["id"] = node.id
			node_arr["node_lat"] = node.node_lat
			node_arr["node_lon"] = node.node_lon
			node_arr["node_status_id"] = node.node_status_id
			node_arr["node_desc"] = node.node_desc
			json["data"].append(dict)
	elif request.method == 'PATCH':
		data = QueryDict(request.body)
		dict = {}
		id = data.get('id')
		status = data.get('status')
		entry = ents.get(pk=id)
		entry.status_id = status
		entry.save()
		dict['status'] = 1
		json["data"].append(dict)
	return JsonResponse(json)

