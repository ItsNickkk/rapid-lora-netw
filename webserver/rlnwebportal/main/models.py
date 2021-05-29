from django.db import models
from django.db.models.fields import CharField
from django.db.models.fields.related import ForeignKey

class dim_node_status(models.Model):
    status = models.CharField(max_length=50)

class dim_node(models.Model):
    node_lat = models.FloatField()
    node_lon = models.FloatField()
    node_desc = models.CharField(max_length=20) #Short Description of the Node
    node_status = models.ForeignKey(dim_node_status, on_delete=models.CASCADE)

class dim_entry_status(models.Model):
    status = models.CharField(max_length=20)

class entries(models.Model):
    name = models.CharField(max_length=30)
    address = models.CharField(max_length=100)
    hpno = models.CharField(max_length=12)
    occupants = models.IntegerField(2)
    danger = models.BooleanField()
    firstaid = models.BooleanField()
    water = models.BooleanField()
    food = models.BooleanField()
    hug = models.BooleanField()
    datetime = models.DateTimeField(auto_now_add=True)
    node = models.ForeignKey(dim_node, on_delete=models.CASCADE)
    status = models.ForeignKey(dim_entry_status, on_delete=models.CASCADE)