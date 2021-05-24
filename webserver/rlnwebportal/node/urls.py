from django.urls import path

from . import views

urlpatterns = [
    path('', views.home, name='node-home'),
]