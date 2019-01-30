# Register your models here.
from django.contrib import admin
from .models import Question, Choice

# admin.site.register(Question)

# aim1.1: define displaying order of fields in the form
# --QuestionAdmin extends admin.ModelAdmin
# --has fields of Question class: pub_data and question_text
# --add it to register() argument
#class QuestionAdmin(admin.ModelAdmin):
#    fields = ['pub_date', 'question_text']

'''
# aim1.2: split the form up into fieldsets
# --list of fields -> list of tuples, each tuple that has set title name and dictionary with key 'fields'
class QuestionAdmin(admin.ModelAdmin):
    fieldsets = [
        (None,               {'fields': ['question_text']}),
        ('Date information', {'fields': ['pub_date']}),
    ]
'''
# aim2: adding related objects--here is to add choices to each question
# there are two ways to solve this

# way1: register Choice with the admin JUSK LIKE adding question
#--we will find the Question field is a <select> box since this fields is assgined by ForeignKey method
#--you could add another question from this, Django will save the question to the database and dynamically add it as the selected choice
#--problem: tedious to add Choice objects to the system.
# admin.site.register(Question, QuestionAdmin)
# admin.site.register(Choice)

# way2: add a bunch of Choices directly when you create the Question object
# --instead of registering Choices to admin, we edit the registration code of Question
# --add one field to the user-defined subclass of ModelAdmin
#   which is a list of classes: style of layout, etc.
#   NB:we could  admin.Stackedinline, admin.TabularInline, etc.
# --define "model" and "extra" fields of Stackedinline
# --we could add another choice in admin page, and it will save it to the database and     dynamically add it to the page
class ChoiceInline(admin.TabularInline):
    model = Choice # model inline
    extra = 3 # number of objects


class QuestionAdmin(admin.ModelAdmin):
    fieldsets = [
        (None,               {'fields': ['question_text']}),
        ('Date information', {'fields': ['pub_date'], 'classes': ['collapse']}),
    ]
    inlines = [ChoiceInline]
    
    # aim3.1: customize the change list
    # --By default, admin display __str()__ (we define for models) of each object我们之前都只返回了text，没有其它信息
    # --override the field 'list_display' in QuestionAdmin class
    #   with a tuple of information(tuple里的顺序不能改)
    #   NB: 还可以加method的名字到tuple
    # --we could look in detail of list_display about function attributes
    list_display = ('question_text', 'pub_date', 'was_published_recently')

    # aim3.2: improve was_published_recently column by adding function attributes
    #         to was_published _recently() function, which could be recognized by
    #         the "list_display" attribute of admin.ModelAdmin

    # aim3.3: to add a sidebar, called Filter that lets people filter
    #       the change list by the pub_date field
    list_filter = ['pub_date']

    # aim3.3: add search capability which adds a search box
    search_fields = ['question_text']

# 纵列(objects); change list 横列(attributes of object)
    
admin.site.register(Question, QuestionAdmin)



