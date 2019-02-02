from django.urls import path
from django.contrib.auth import views as auth_views
from . import views
from django import forms
from django.forms import ModelForm

app_name = 'driverRegister'

urlpatterns = [
    path('',views.reg_driver,name='registerdriver'),
    path('profile/',views.view_profile, name='view_profile'),
    path('profile/edit/',views.edit_profile, name='edit_profile')
]
