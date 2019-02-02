from django.urls import path
from . import views

app_name = 'rideRequest'

urlpatterns = [
    path('request/',views.reg_driver,name='request'),
    path('wait/',views.view_profile, name='wait'),
]
