from django.db import models
from django.core.validators import MinValueValidator, MaxValueValidator
from driverRegister.models import Driver
import datetime
# Create your models here.
from django.contrib.auth import get_user_model
User = get_user_model()

class RequestRide(models.Model):
    TYPE_CHOICES = (
    ('1', 'Type1'),
    ('2', 'Type2'),
    ('3', 'Type3'),
    ('4', 'Does not matter')
    )
    #users=[]
    user = models.ForeignKey(User,related_name='request',on_delete=models.CASCADE)
    driver_name = models.CharField(max_length=246, null=True)
    driver_vehicle_type = models.CharField(max_length=246,null=True)
    destination = models.CharField(max_length=246)
    date_time = models.DateField()
    num_passengers = models.IntegerField(validators=[MinValueValidator(1),
                                        MaxValueValidator(5)])
    type = models.CharField(max_length=1,choices=TYPE_CHOICES,default='4')
    special_requests = models.CharField(max_length=300)
    status_share = models.BooleanField(default=False)
    #if the request has been submitted, this status should be set to true
    status_confirm = models.BooleanField(default=False)
    status_conplete = models.BooleanField(default=False)

    remaining_seat = models.IntegerField(default=0,validators=[MinValueValidator(0),
                                        MaxValueValidator(5)])

    def __str__(self):
        return self.user.username
