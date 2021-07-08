from django.shortcuts import render
from django.http import JsonResponse, QueryDict
from django.db.models import Q
from main.models import entries, dim_entry_status, dim_node

def home(request):
	nodes = dim_node.objects.all()
	return render(request, 'main/home.html', {'title': 'Entries', "nodes": nodes})

def manageEntries(request):
	json = {"data":[]}
	nodes = dim_node.objects
	ents = entries.objects.order_by('-id')
	entry_stat = dim_entry_status.objects
	if request.method == 'GET':
		if 'latest_id' in request.GET: # Get latest Node ID
			dict={}
			id = ents.order_by('-id')[0].id
			dict["latest_id"]=id
			json['data'].append(dict)
		else:
			q = Q()
			if 'entry_id' in request.GET:
				q.add(Q(pk=request.GET.get('entry_id')), q.AND)
			if 'name' in request.GET:
				q.add(Q(name__icontains=request.GET.get('name')), q.AND)
			if 'address' in request.GET:
				q.add(Q(address__icontains=request.GET.get('address')), q.AND)
			if 'status' in request.GET:
				q.add(Q(status=request.GET.get('status')), q.AND)
			if 'node' in request.GET:
				q.add(Q(node_id=request.GET.get('node')), q.AND)	
			ents = ents.filter(q)	
			for entry in ents:
				reqNodeID = Q(pk=entry.node_id)
				node = nodes.filter(reqNodeID)[0]
				node_arr = {
					"id": node.id,
					"lat": node.node_lat,
					"lon": node.node_lon,
					"status_id": node.node_status_id,
					"desc": node.node_desc
				}
				dict = {
					"id": entry.id,
					"name": entry.name,
					"address": entry.address,
					"hpno": entry.hpno,
					"occupants": entry.occupants,
					"danger": entry.danger,
					"water": entry.water,
					"food": entry.food,
					"hug": entry.hug,
					"status": entry.status_id,
					"datetime": entry.datetime.timestamp(),
					"firstaid": entry.firstaid,
					"node": node_arr
				}
				
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

