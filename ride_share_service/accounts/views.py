from django.shortcuts import render
from django.urls import reverse_lazy
from django.views.generic import CreateView

from . import forms
# Create your views here.
class SignUp(CreateView):
    form_class = forms.UserCreateForm
    # on a successful sign up, I will reverse them to the login page
    # do not execute until user hit submit on the sign up button
    success_url = reverse_lazy('login')
    template_name = 'accounts/signup.html'

#def profile(request):
#    args = {'user':request.user}
#    return render(request, 'accounts/profile.html',args)
