from django.urls import path
from django.contrib.auth import views as auth_views
from . import views
from driverRegister import views as driver_views

app_name = 'accounts'

urlpatterns = [
    path('login/',auth_views.LoginView.as_view(template_name='accounts/login.html'),
    name='login'),
    path('logout/',auth_views.LogoutView.as_view(),name='logout'),
    path('signup/',views.SignUp.as_view(),name='signup')
    #url(r'^profile/$',driver_views.views.profile, name='profile')
]
