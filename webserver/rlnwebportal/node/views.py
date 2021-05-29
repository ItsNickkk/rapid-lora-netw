from django.shortcuts import render
from django.http import JsonResponse, QueryDict
from django.db.models import Q
from django.db import IntegrityError
from main.models import dim_node, dim_node_status

def home(request):
	node_status = dim_node_status.objects.all()
	return render(request, 'node/node.html', {'node_status':node_status})

def manageNode(request):
	nodes = dim_node.objects.order_by('id')
	node_status = dim_node_status.objects
	json = {"data":[]}
	if request.method == 'GET':
		if 'node_id' in request.GET:
			try:
				dict = {}
				reqNodeID = Q(id=request.GET.get('node_id'))
				node = nodes.filter(reqNodeID)[0]
				dict["id"] = node.id
				dict["lat"] = node.node_lat
				dict["lon"] = node.node_lon
				dict["dsc"] = node.node_desc
				dict["stat"] = node.node_status_id
				dict["statdesc"] = node_status.select_related().filter(id = node.id)[0].status
				json["data"].append(dict)
			except IndexError:
				dict = {}
				dict["status"] = -1
				json["data"].append(dict)
		if 'latest_id' in request.GET:
			dict={}
			id = nodes.order_by('-id')[0].id
			dict["latest_id"]=id
			json['data'].append(dict)
		else:
			for node in nodes:
				dict = {}
				dict["id"] = node.id
				dict["lat"] = node.node_lat
				dict["lon"] = node.node_lon
				dict["dsc"] = node.node_desc
				dict["stat"] = node.node_status_id
				dict["statdesc"] = node_status.select_related().filter(id = node.id)[0].status
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
			node.save()
			dict['status'] = 1
			json["data"].append(dict)
		except IntegrityError:
			pass
	elif request.method == 'PATCH':
		try:
			dict={}
			data = QueryDict(request.body)
			id = data['id']
			node = nodes.get(pk=id)
			node.node_lon = data['lon']
			node.node_lat = data['lat']
			node.node_desc = data['desc']
			node.node_status_id = data['stat']
			node.save()
			dict['status'] = 1
			json["data"].append(dict)
		except Exception:
			pass
	return JsonResponse(json)