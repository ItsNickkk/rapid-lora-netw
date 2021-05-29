from django.urls import path

from . import views

urlpatterns = [
    path('', views.home, name='node-home'),
    path('/api/manageNode', views.manageNode, name='api-manage-node')
]