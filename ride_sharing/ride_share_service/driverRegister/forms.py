from django import forms
from django.forms import ModelForm
from django.contrib.auth.models import User
from driverRegister.models import Driver
from django.contrib.auth.forms import UserChangeForm

class DriverRegisterForm(forms.ModelForm):
    #first_name = forms.CharField(max_length=None)
    #last_name = forms.CharField(max_length=None)
    class Meta:
        model = Driver
        fields = ['type','license_plate_num','max_num_passengers']

    def save(self, commit=True):
        user = super(DriverRegisterForm,self).save(commit=False)
        #user.first_name = self.cleaned_data['first_name']
        #user.last_name = self.cleaned_data['last_name']
        user.type = self.cleaned_data['type']
        user.license_plate_num = self.cleaned_data['license_plate_num']
        user.max_num_passengers = self.cleaned_data['max_num_passengers']

        if commit:
            user.save()

        return user


class EditProfileForm(forms.ModelForm):
    class Meta:
        model = User
        fields = ['email']

class EditDriverProfileForm(forms.ModelForm):
    class Meta:
        model = Driver
        fields = ['type','license_plate_num','max_num_passengers']
