# Register your models here.
from django.contrib import admin
from .models import Question

# admin.site.register(Question)

# aim1: define displaying order of fields in the form
# --QuestionAdmin extends admin.ModelAdmin
# --has fields of Question class: pub_data and question_text
# --add it to register() argument
#class QuestionAdmin(admin.ModelAdmin):
#    fields = ['pub_date', 'question_text']


# aim2: split the form up into fieldsets
# --list of fields -> list of tuples, each tuple that has set title name and dictionary with key 'fields'
class QuestionAdmin(admin.ModelAdmin):
    fieldsets = [
        (None,               {'fields': ['question_text']}),
        ('Date information', {'fields': ['pub_date']}),
    ]




admin.site.register(Question, QuestionAdmin)
