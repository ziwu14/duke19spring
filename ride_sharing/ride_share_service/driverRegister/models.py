from django.db import models
from django.contrib.auth import get_user_model
from django.contrib.auth.models import User
from django.core.validators import MinValueValidator, MaxValueValidator
User = get_user_model()
# Create your models here.

class Driver(models.Model):
    user = models.OneToOneField(User, on_delete=models.CASCADE,null=True)
    #user = models.ForeignKey(User, on_delete=models.CASCADE,)
    TYPE_CHOICES = (
    ('1', 'Type1'),
    ('2', 'Type2'),
    ('3', 'Type3'),
    )
    #name = models.CharField(max_length=264)
    type = models.CharField(max_length=1,choices=TYPE_CHOICES,default='1')
    license_plate_num = models.CharField(max_length=264, default='default value')
    max_num_passengers = models.IntegerField(validators=[MinValueValidator(1),
                                       MaxValueValidator(5)])

    def __str__(self):
        return self.type
