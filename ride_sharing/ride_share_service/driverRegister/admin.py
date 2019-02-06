from django.contrib import admin
from driverRegister.models import Driver
from django.contrib.auth.models import User
from django.contrib.auth import get_user_model
#User = get_user_model()
# Register your models here.
class GroupMemberInline(admin.TabularInline):
    model = Driver
#admin.site.register(Driver)
class DriverInlineAdmin(admin.ModelAdmin):
    inlines = [GroupMemberInline,]

admin.site.unregister(get_user_model())
admin.site.register(get_user_model(), DriverInlineAdmin)
