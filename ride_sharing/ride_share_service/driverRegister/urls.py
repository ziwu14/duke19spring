from django.conf.urls import url
from django.contrib.auth import views as auth_views
from . import views
from django import forms
from django.forms import ModelForm

app_name = 'driverRegister'

urlpatterns = [
    url(r'^$',views.reg_driver,name='registerdriver'),
    url(r'^profile/$',views.view_profile, name='view_profile'),
    url(r'^profile/edit/$',views.edit_profile, name='edit_profile')
]
