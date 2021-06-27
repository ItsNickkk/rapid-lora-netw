from django.shortcuts import render
from django.http import JsonResponse, QueryDict
from django.db.models import Q
from django.db import IntegrityError
from django.core.exceptions import *
from main.models import dim_node, dim_node_status

def home(request):
	node_status = dim_node_status.objects.all()
	return render(request, 'node/node.html', {'node_status':node_status, "title":"Node Management"})

def manageNode(request):
	nodes = dim_node.objects.order_by('id')
	node_status = dim_node_status.objects
	json = {"data":[]}
	if request.method == 'GET':
		if 'node_id' in request.GET: # Get selected node by ID
			try:
				reqNodeID = Q(pk=request.GET.get('node_id'))
				node = nodes.filter(reqNodeID)[0]
				dict = {
					"id": node.id,
					"lat": node.node_lat,
					"lon": node.node_lon,
					"dsc": node.node_desc,
					"stat": node.node_status.id,
					"is_base": node.is_base
				}
				json["data"].append(dict)
			except IndexError:
				dict = {}
				dict["status"] = -1
				json["data"].append(dict)
		if 'latest_id' in request.GET: # Get latest Node ID
			dict={}
			id = nodes.order_by('-id')[0].id
			dict["latest_id"]=id
			json['data'].append(dict)
		else: # Get a list of Node
			for node in nodes:
				if 'active_node' in request.GET and node.node_status_id is 4: # Get a list of active node
					continue
				elif 'disabled_node' in request.GET and node.node_status_id is not 4: # Get a list of disabled node
					continue
				dict = {
					"id": node.id,
					"lat": node.node_lat,
					"lon": node.node_lon,
					"dsc": node.node_desc,
					"stat": node.node_status.id,
					"is_base": node.is_base
				}
				json["data"].append(dict)
	elif request.method == 'POST':
		try:
			dict={}
			node = dim_node()
			node.id = request.POST.get('id')
			node.node_lon = request.POST.get('lon')
			node.node_lat = request.POST.get('lat')
			node.node_desc = request.POST.get('desc')
			node.node_status_id = request.POST.get('stat')
			node.is_base = request.POST.get('bs')
			node.save()
			dict['status'] = 1
			json["data"].append(dict)
		except IntegrityError:
			pass
	elif request.method == 'PUT':
		try:
			data = QueryDict(request.body)
			dict={}
			id = data.get('id')
			node = nodes.get(pk=id)
			if data.get('ops'):
				node.node_status_id = data.get('ops')
			else:
				node.node_lon = data.get('lon')
				node.node_lat = data.get('lat')
				node.node_desc = data.get('desc')
				node.node_status_id = data.get('stat')
				node.is_base = data.get('bs')
			node.save()
			dict['status'] = 1
			json["data"].append(dict)
		except ObjectDoesNotExist:
			dict = {}
			dict["status"] = -1
			json["data"].append(dict)
			pass
	
	return JsonResponse(json)