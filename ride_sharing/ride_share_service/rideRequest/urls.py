from django.conf.urls import url
from . import views

app_name = 'rideRequest'

urlpatterns = [
    url(r'^$',views.request_ride,name='requestride'),
    url(r'^personalrequest/$',views.personal_request,name='personalrequest'),
    url(r'^driversearch/$',views.driver_search,name='driversearch'),
    url(r'^delete/(?P<part_id>[0-9]+)/$', views.delete_view, name='delete_view'),
    url(r'^accept/(?P<part_id>[0-9]+)/$', views.accept_view, name='accept_view'),
    url(r'^complete/(?P<part_id>[0-9]+)/$', views.complete_view, name='complete_view'),
    url(r'^join/(?P<part_id>[0-9]+)/(?P<num_passengers>[0-9]+)$', views.join_view, name='join_view'),
    url(r'^rideshare/$',views.share_view,name='share_view'),
]
