from django.urls import path

from . import views

urlpatterns = [
    path('', views.home, name='home'),
    path('api/manageEntries', views.manageEntries, name='mngEntries'),
]