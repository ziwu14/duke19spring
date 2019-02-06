from django import forms
from django.forms import ModelForm
from django.contrib.auth.models import User
from rideRequest.models import RequestRide
from django.contrib.auth.forms import UserChangeForm
from datetime import date

class RequestForm(forms.ModelForm):
    class Meta:
        model = RequestRide
        fields = ['destination','date_time','num_passengers','type', 'special_requests','status_share']

    def save(self, commit=True):
        user = super(RequestForm,self).save(commit=False)
        user.destination = self.cleaned_data['destination']
        user.date_time = self.cleaned_data['date_time']
        user.num_passengers = self.cleaned_data['num_passengers']
        user.type = self.cleaned_data['type']
        user.special_requests = self.cleaned_data['special_requests']
        user.status_share = self.cleaned_data['status_share']

        if commit:
            user.save()

        return user
