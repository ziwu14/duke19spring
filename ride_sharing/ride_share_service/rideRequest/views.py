from django.shortcuts import render
from django.http import HttpResponse, HttpResponseRedirect
from django import forms
from django.forms import ModelForm
from rideRequest.forms import RequestForm
from django.contrib.auth.models import User
from django.contrib.auth.forms import UserChangeForm
from django.contrib.auth.decorators import login_required
from django.urls import reverse
from rideRequest.models import RequestRide
# Create your views here.
def request_ride(request):
    # if this is a POST request we need to process the form data
    if request.method == 'POST':
        # create a form instance and populate it with data from the request:
        form = RequestForm(request.POST)
        # check whether it's valid:
        if form.is_valid():
            instance = form.save(commit=False)
            instance.user = request.user
            instance.save()
            return HttpResponseRedirect(reverse('rideRequest:personalrequest'))
            #return render(request, 'driver_register_form.html', {'form': form,'registered': registered})

    # if a GET (or any other method) we'll create a blank form
    else:
        form = RequestForm()

    return render(request, 'ride_request_detail.html', {'form': form})


def personal_request(request):
    personal_ride_list = RequestRide.objects.all().filter(user=request.user)
    args = {'personal_ride_list': personal_ride_list}
    return render(request, 'user_ride_request_detail.html', args)


def driver_search(request):
    available_ride_list = RequestRide.objects.all()
    ride_dict = {'search_ride': available_ride_list}
    return render(request,'driver_search.html',ride_dict)

def delete_view(request,part_id =None):
    object = RequestRide.objects.get(id=part_id)
    #return HttpResponse('hhhhhh')
    object.delete()
    return HttpResponseRedirect(reverse('rideRequest:personalrequest'))

def accept_view(request,part_id =None):
    object = RequestRide.objects.get(id=part_id)
    #return HttpResponse('hhhhhh')
    #object.delete()
    object.status_confirm = True
    object.driver_name = request.user.username
    object.driver_vehicle_type = request.user.driver.type
    object.remaining_seat = request.user.driver.max_num_passengers - object.num_passengers
    object.save()
    #return HttpResponse(object.remaining_seat)
    return HttpResponseRedirect(reverse('rideRequest:driversearch'))

def complete_view(request,part_id =None):
    object = RequestRide.objects.get(id=part_id)
    #return HttpResponse('hhhhhh')
    object.delete()
    return HttpResponseRedirect(reverse('rideRequest:driversearch'))

def share_view(request):
    if request.method == 'POST':
        # create a form instance and populate it with data from the request:
        form = RequestForm(request.POST)
        # check whether it's valid:
        if form.is_valid():
            #instance = form.save(commit=False)
            #instance.user = request.user
            #instance.save()
            destination = form.cleaned_data['destination']
            date_time = form.cleaned_data['date_time']
            num_passengers = form.cleaned_data['num_passengers']
            type = form.cleaned_data['type']
            special_requests = form.cleaned_data['special_requests']
            share_list = RequestRide.objects.all().filter(destination=destination, date_time=date_time,
                                                        special_requests=special_requests,status_share=True)

            args = []

            if type != '4':
                share_list = share_list.filter(type=type)

            for object in share_list:
                if object.remaining_seat >= num_passengers:
                    args.append(object)
            context_dict = {'eligible_share': args,'num_passengers':num_passengers}

            return render(request,'share_ride.html',context_dict)
            #return render(request, 'driver_register_form.html', {'form': form,'registered': registered})

    # if a GET (or any other method) we'll create a blank form
    else:
        form = RequestForm()

    return render(request, 'ride_request_detail.html', {'form': form})

def join_view(request,part_id =None,num_passengers=None):
    object = RequestRide.objects.get(id=part_id)
    object.remaining_seat = object.remaining_seat - num_passengers
    object.save()

    HttpResponseRedirect(reverse('rideRequest:personalrequest'))
