from django.shortcuts import render, get_object_or_404
#from django.urls import reverse_lazy
#from django.views.generic import TemplateView
from django.http import HttpResponse, HttpResponseRedirect
from django import forms
from django.forms import ModelForm
from driverRegister.forms import DriverRegisterForm, EditProfileForm, EditDriverProfileForm
from django.contrib.auth.models import User
from django.contrib.auth.forms import UserChangeForm
from django.urls import reverse
from .models import Driver

# Create your views here.
def reg_driver(request):
    #try:
    #    driverProfile = request.user.driver
    #except Driver.DoesNotExist:
    #    driverProfile = Driver(user=request.user)
    #registered = False
    # if this is a POST request we need to process the form data
    if request.method == 'POST':
        # create a form instance and populate it with data from the request:
        form = DriverRegisterForm(request.POST)
        # check whether it's valid:
        if form.is_valid():
            instance = form.save(commit=False)
            instance.user = request.user
            instance.save()
            #registered = True
            # process the data in form.cleaned_data as required
            # ...
            # redirect to a new URL:
            return HttpResponseRedirect('/test/')
            #return render(request, 'driver_register_form.html', {'form': form,'registered': registered})

    # if a GET (or any other method) we'll create a blank form
    else:
        form = DriverRegisterForm()

    return render(request, 'driver_register_form.html', {'form': form})
                                                         #'registered': registered})

def view_profile(request):
    args = {'user':request.user}
    return render(request, 'driver_detail.html',args)
'''
def edit_profile(request,pk):
    user = get_object_or_404(User,pk=pk)
    user_profile = get_object_or_404(Driver,user=user)
    if request.method == 'POST':
        form = EditProfileForm(request.POST)
        # driver_form = EditDriverProfileForm(request.POST, instance=request.user)
        #return HttpResponse('Oops!')
        
        if form.is_valid(): # and driver_form.is_valid():
            user.email = form.cleaned_data['email']
                                                        
            user_profile.type = form.cleaned_data['type']
            user_profile.license_plate_num = form.cleaned_data['license_plate_num']
            user_profile.max_num_passengers = form.cleaned_data['max_num_passengers']
            user.save()
            user_profile.save()
            return HttpResponseRedirect(reverse('driverRegister:view_profile'))
        
    else:
        form = EditProfileForm(instance=request.user)
        driver_form = EditDriverProfileForm(instance=request.user)
        args = {'form': form, 'driver_form': driver_form}
        return render(request, 'edit_profile.html',args)
'''
def edit_profile(request):
    user=request.user
    if request.method == 'POST':
        form = EditProfileForm(request.POST)
        driver_form = EditDriverProfileForm(request.POST, instance=request.user)
        #return HttpResponse('Oops!')
        
        if form.is_valid() and driver_form.is_valid():
            user.email = form.cleaned_data['email']                                    
            user.driver.type = driver_form.cleaned_data['type']
            user.driver.license_plate_num = driver_form.cleaned_data['license_plate_num']
            user.driver.max_num_passengers = driver_form.cleaned_data['max_num_passengers']
            
            user.save()
            user.driver.save()
            return HttpResponseRedirect(reverse('driverRegister:view_profile'))
        
    else:
        default_data = {'type': user.driver.type, 'license_plate_num': user.driver.license_plate_num,
                        'max_num_passengers': user.driver.max_num_passengers, }
        form = EditProfileForm(instance=request.user)
        driver_form = EditDriverProfileForm(default_data,instance=request.user)
        args = {'form': form, 'driver_form': driver_form}
        return render(request, 'edit_profile.html',args)
    
        form = EditProfileForm(instance=request.user)
        driver_form = EditDriverProfileForm(instance=request.user)
        args = {'form': form, 'driver_form': driver_form}
        return render(request, 'edit_profile.html',args)
